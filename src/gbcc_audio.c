#include "gbcc.h"
#include "gbcc_audio.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

/* 4 channels, with volume 0x00-0x0F each */
#define NANOSECOND 1000000000lu
#define BASE_AMPLITUDE (UINT16_MAX / 4 / 0x0F)
#define SAMPLE_RATE 48000
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)
#define BUFSIZE 512 /* samples */
#define AUDIO_FMT Uint16
#define SLEEP_TIME (NANOSECOND / SAMPLE_RATE)
#define SEQUENCER_CLOCKS 8192

struct channel {
	AUDIO_FMT buffer[BUFSIZE];
	uint64_t freq;
	uint8_t counter;
	uint8_t volume;
	struct {
		uint8_t period;
		int8_t dir;
	} envelope;
};

struct apu {
	SDL_AudioSpec audiospec;
	SDL_AudioDeviceID device;
	uint64_t sample_clock;
	uint64_t sample;
	size_t index;
	struct timespec cur_time;
	struct timespec start_time;
	AUDIO_FMT mix_buffer[BUFSIZE];
	struct channel ch1; 	/* Tone & Sweep */
	struct channel ch2; 	/* Tone */
	struct channel ch3; 	/* Wave Output */
	struct channel ch4; 	/* Noise */
	uint64_t clock;
	struct {
		uint64_t freq;
		uint8_t period;
		uint8_t shift;
		int8_t dir;
		int8_t counter;
	} sweep;
};

static struct apu apu;

static void sequencer_clock(struct gbc *gbc);
static void ch1_update(struct gbc *gbc);
static void ch2_update(struct gbc *gbc);
static void ch3_update(struct gbc *gbc);
static void ch4_update(struct gbc *gbc);
static uint64_t time_diff(const struct timespec *cur,
		const struct timespec *old);

void gbcc_audio_initialise(void)
{
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to initialize SDL: %s\n", SDL_GetError());
	}
	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 1;
	want.samples = 0;
	want.callback = NULL;
	want.userdata = NULL;
	apu.sample_clock = 0;
	apu.sample = 0;
	apu.index = 0;
	apu.start_time.tv_sec = 0;
	apu.start_time.tv_nsec = 0;

	apu.device = SDL_OpenAudioDevice(NULL, 0, &want, &apu.audiospec, 0);
	if (apu.device == 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to open audio: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (want.format != apu.audiospec.format) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to get the desired AudioSpec.\n");
		exit(EXIT_FAILURE);
	} 
	SDL_PauseAudioDevice(apu.device, 0);
}

void gbcc_audio_update(struct gbc *gbc)
{
	if (gbc->keys.turbo) {
		apu.start_time.tv_sec = 0;
		apu.sample = 0;
		return;
	}
	if (apu.start_time.tv_sec == 0) {
		clock_gettime(CLOCK_REALTIME, &apu.start_time);
	}
	uint64_t clocks = gbc->clock - apu.sample_clock;
	if (!(gbc->clock % SEQUENCER_CLOCKS)) {
		sequencer_clock(gbc);
	}
	if (clocks > CLOCKS_PER_SAMPLE) {
		clock_gettime(CLOCK_REALTIME, &apu.cur_time);
		uint64_t diff = time_diff(&apu.cur_time, &apu.start_time);
		while (diff < (NANOSECOND * apu.sample) / SAMPLE_RATE) {
			//printf("%lu, %lu, %lu, %lu\n", diff, (NANOSECOND * apu.sample) / SAMPLE_RATE, apu.start_time.tv_sec, apu.start_time.tv_nsec);
			const struct timespec time = {.tv_sec = 0, .tv_nsec = SLEEP_TIME};
			nanosleep(&time, NULL);
			clock_gettime(CLOCK_REALTIME, &apu.cur_time);
			diff = time_diff(&apu.cur_time, &apu.start_time);
		}
		//printf("\n");
		apu.sample_clock = gbc->clock;
		ch1_update(gbc);
		ch2_update(gbc);
		ch3_update(gbc);
		ch4_update(gbc);
		apu.sample++;
		apu.index++;
		if (apu.index == BUFSIZE) {
			apu.index = 0;
			for (size_t i = 0; i < BUFSIZE; i++) {
				apu.mix_buffer[i] =
					apu.ch1.buffer[i]
					+ apu.ch2.buffer[i]
					+ apu.ch3.buffer[i]
					+ apu.ch4.buffer[i];
			}
			SDL_QueueAudio(apu.device, apu.mix_buffer, BUFSIZE * sizeof(AUDIO_FMT));
		}
	}
}

void sequencer_clock(struct gbc *gbc)
{
	apu.ch1.freq = gbcc_memory_read(gbc, NR13, true);
	apu.ch1.freq |= (gbcc_memory_read(gbc, NR14, true) & 0x07u) << 8u;
	apu.ch1.freq = 131072/(2048-apu.ch1.freq);
	apu.ch2.freq = gbcc_memory_read(gbc, NR23, true);
	apu.ch2.freq |= (gbcc_memory_read(gbc, NR24, true) & 0x07u) << 8u;
	apu.ch2.freq = 131072/(2048-apu.ch2.freq);

	/* Length counters */
	if (apu.clock & 0x01u) {
		if (gbcc_memory_read(gbc, NR14, true) & bit(6)) {
			apu.ch1.counter--;
			if (apu.ch1.counter == 0) {
				apu.ch1.volume = 0;
			}
		}
		if (gbcc_memory_read(gbc, NR24, true) & bit(6)) {
			apu.ch2.counter--;
			if (apu.ch2.counter == 0) {
				apu.ch2.volume = 0;
			}
		}
		if (gbcc_memory_read(gbc, NR34, true) & bit(6)) {
			apu.ch3.counter--;
			if (apu.ch3.counter == 0) {
				apu.ch3.volume = 0;
			}
		}
		if (gbcc_memory_read(gbc, NR44, true) & bit(6)) {
			apu.ch4.counter--;
			if (apu.ch4.counter == 0) {
				apu.ch4.volume = 0;
			}
		}
	}
	
	/* Sweep */
	if (apu.clock % 0x08u == 2u || apu.clock % 0x08u == 6u) {
		apu.sweep.period = (gbcc_memory_read(gbc, NR10, true) & 0x70u) >> 4u;
		apu.sweep.dir = gbcc_memory_read(gbc, NR10, true) & 0x07u ? 1 : -1;
		apu.sweep.counter++;
		if (apu.sweep.counter == apu.sweep.period) {
			apu.sweep.counter = 0;
			uint64_t freq = apu.sweep.freq >> apu.sweep.shift;
			freq = apu.sweep.freq + apu.sweep.dir * freq;
			if (apu.sweep.shift != 0 && freq < 2047) {
				apu.sweep.freq = freq;
				apu.ch1.freq = freq;
			}
		}
	}
	
	/* Envelope */
	if (apu.clock & 0x07u) {
		apu.ch1.envelope.period = gbcc_memory_read(gbc, NR12, true) & 0x07u;
		apu.ch2.envelope.period = gbcc_memory_read(gbc, NR22, true) & 0x07u;
		apu.ch1.envelope.dir = gbcc_memory_read(gbc, NR12, true) & 0x08u ? 1 : -1;
		apu.ch2.envelope.dir = gbcc_memory_read(gbc, NR22, true) & 0x08u ? 1 : -1;
		if (apu.ch1.envelope.period == 0) {
			apu.ch1.volume = (gbcc_memory_read(gbc, NR12, true) & 0xF0u) >> 4u;
		} else if (!(apu.clock % (apu.ch1.envelope.period * 0x07u))) {
			apu.ch1.volume += apu.ch1.envelope.dir;
			if (apu.ch1.volume == 0x10u || apu.ch1.volume == 0xFFu) {
				apu.ch1.volume -= apu.ch1.envelope.dir;
			}
		}
		if (apu.ch2.envelope.period == 0) {
			apu.ch2.volume = (gbcc_memory_read(gbc, NR22, true) & 0xF0u) >> 4u;
		} else if (!(apu.clock % (apu.ch2.envelope.period * 0x07u))) {
			apu.ch2.volume += apu.ch2.envelope.dir;
			if (apu.ch2.volume == 0x10u || apu.ch2.volume == 0xFFu) {
				apu.ch2.volume -= apu.ch2.envelope.dir;
			}
		}
	}

	/* Restart sounds */
	if (gbcc_memory_read(gbc, NR14, true) & bit(7)) {
		apu.ch1.volume = (gbcc_memory_read(gbc, NR12, true) & 0xF0u) >> 4u;
		gbcc_memory_clear_bit(gbc, NR14, 7, true);
	}
	if (gbcc_memory_read(gbc, NR24, true) & bit(7)) {
		apu.ch2.volume = (gbcc_memory_read(gbc, NR22, true) & 0xF0u) >> 4u;
		gbcc_memory_clear_bit(gbc, NR24, 7, true);
	}

	apu.clock++;
}

void ch1_update(struct gbc *gbc)
{
	uint8_t volume = apu.ch1.volume;
	uint64_t freq = apu.ch1.freq;
	unsigned long time = (unsigned long)lround(2 * apu.sample * (double)freq / SAMPLE_RATE);
	apu.ch1.buffer[apu.index] = (AUDIO_FMT)(volume * BASE_AMPLITUDE * (time & 1u));
}

void ch2_update(struct gbc *gbc)
{
	uint8_t volume = apu.ch2.volume;
	uint64_t freq = apu.ch2.freq;
	unsigned long time = (unsigned long)lround(2 * apu.sample * (double)freq / SAMPLE_RATE);
	apu.ch2.buffer[apu.index] = (AUDIO_FMT)(volume * BASE_AMPLITUDE * (time & 1u));
}

void ch3_update(struct gbc *gbc)
{
	apu.ch3.buffer[apu.index] = 0;
}

void ch4_update(struct gbc *gbc)
{
	apu.ch4.buffer[apu.index] = 0;
}

uint64_t time_diff(const struct timespec * const cur,
		const struct timespec * const old)
{
	uint64_t sec = (uint64_t)(cur->tv_sec - old->tv_sec);
	uint64_t nsec;
	if (sec == 0) {
		nsec = (uint64_t)(cur->tv_nsec - old->tv_nsec);
		return nsec;
	}
	nsec = NANOSECOND * sec;
	if (old->tv_nsec > cur->tv_nsec) {
		nsec -= (uint64_t)(old->tv_nsec - cur->tv_nsec);
	} else {
		nsec += (uint64_t)(cur->tv_nsec - old->tv_nsec);
	}
	return nsec;
}
