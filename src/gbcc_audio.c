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
#define BUFSIZE 1024 /* samples */
#define AUDIO_FMT Uint16

struct channel {
	AUDIO_FMT buffer[BUFSIZE];
	uint8_t counter;
};

struct gbcc_audio {
	SDL_AudioSpec audiospec;
	SDL_AudioDeviceID device;
	uint64_t clock;
	uint64_t sample;
	size_t index;
	struct timespec cur_time;
	struct timespec start_time;
	AUDIO_FMT mix_buffer[BUFSIZE];
	struct channel ch1; 	/* Tone & Sweep */
	struct channel ch2; 	/* Tone */
	struct channel ch3; 	/* Wave Output */
	struct channel ch4; 	/* Noise */
};

static struct gbcc_audio gbcc_audio;

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
	gbcc_audio.clock = 0;
	gbcc_audio.sample = 0;
	gbcc_audio.index = 0;
	gbcc_audio.start_time.tv_sec = 0;
	gbcc_audio.start_time.tv_nsec = 0;

	gbcc_audio.device = SDL_OpenAudioDevice(NULL, 0, &want, &gbcc_audio.audiospec, 0);
	if (gbcc_audio.device == 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to open audio: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (want.format != gbcc_audio.audiospec.format) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to get the desired AudioSpec.\n");
		exit(EXIT_FAILURE);
	} 
	SDL_PauseAudioDevice(gbcc_audio.device, 0);
}

void gbcc_audio_update(struct gbc *gbc)
{
	if (gbc->keys.turbo) {
		gbcc_audio.start_time.tv_sec = 0;
		gbcc_audio.sample = 0;
		return;
	}
	if (gbcc_audio.start_time.tv_sec == 0) {
		clock_gettime(CLOCK_REALTIME, &gbcc_audio.start_time);
	}
	uint64_t clocks = gbc->clock - gbcc_audio.clock;
	if (clocks > CLOCKS_PER_SAMPLE) {
		clock_gettime(CLOCK_REALTIME, &gbcc_audio.cur_time);
		uint64_t diff = time_diff(&gbcc_audio.cur_time, &gbcc_audio.start_time);
		while (diff < (NANOSECOND * gbcc_audio.sample) / SAMPLE_RATE) {
			//printf("%lu, %lu, %lu, %lu\n", diff, (NANOSECOND * gbcc_audio.sample) / SAMPLE_RATE, gbcc_audio.start_time.tv_sec, gbcc_audio.start_time.tv_nsec);
			clock_gettime(CLOCK_REALTIME, &gbcc_audio.cur_time);
			diff = time_diff(&gbcc_audio.cur_time, &gbcc_audio.start_time);
		}
		//printf("\n");
		gbcc_audio.clock = gbc->clock;
		ch1_update(gbc);
		ch2_update(gbc);
		ch3_update(gbc);
		ch4_update(gbc);
		gbcc_audio.sample++;
		gbcc_audio.index++;
		if (gbcc_audio.index == BUFSIZE) {
			gbcc_audio.index = 0;
			for (size_t i = 0; i < BUFSIZE; i++) {
				gbcc_audio.mix_buffer[i] =
					gbcc_audio.ch1.buffer[i]
					+ gbcc_audio.ch2.buffer[i]
					+ gbcc_audio.ch3.buffer[i]
					+ gbcc_audio.ch4.buffer[i];
			}
			SDL_QueueAudio(gbcc_audio.device, gbcc_audio.mix_buffer, BUFSIZE * sizeof(AUDIO_FMT));
		}
	}
}

void ch1_update(struct gbc *gbc)
{
	uint8_t volume = (gbcc_memory_read(gbc, NR12, true) & 0xF0u) >> 4u;
	uint64_t freq = gbcc_memory_read(gbc, NR13, true);
	uint8_t nr14 = gbcc_memory_read(gbc, NR14, true);
	freq |= (nr14 & 0x07u) << 8u;
	freq = 131072/(2048-freq);
	if (nr14 & bit(7)) {
		gbcc_audio.ch1.counter = gbcc_memory_read(gbc, NR11, true) & 0x1Fu;
		gbcc_memory_clear_bit(gbc, NR14, 7, true);
	}
	if (nr14 & bit(6)) {
		if (gbcc_audio.ch1.counter == 0) {
			gbcc_audio.ch1.buffer[gbcc_audio.index] = 0;
			return;
		}
		gbcc_audio.ch1.counter--;
	}
	unsigned long time = (unsigned long)lround(2 * gbcc_audio.sample * (double)freq / SAMPLE_RATE);
	gbcc_audio.ch1.buffer[gbcc_audio.index] = (AUDIO_FMT)(volume * BASE_AMPLITUDE * (time & 1u));
}

void ch2_update(struct gbc *gbc)
{
	uint8_t volume = (gbcc_memory_read(gbc, NR22, true) & 0xF0u) >> 4u;
	uint64_t freq = gbcc_memory_read(gbc, NR23, true);
	uint8_t nr24 = gbcc_memory_read(gbc, NR24, true);
	freq |= (nr24 & 0x07u) << 8u;
	freq = 131072/(2048-freq);
	if (nr24 & bit(7)) {
		gbcc_audio.ch2.counter = gbcc_memory_read(gbc, NR21, true) & 0x1Fu;
		gbcc_memory_clear_bit(gbc, NR24, 7, true);
	}
	if (nr24 & bit(6)) {
		if (gbcc_audio.ch2.counter == 0) {
			gbcc_audio.ch2.buffer[gbcc_audio.index] = 0;
			return;
		}
		gbcc_audio.ch2.counter--;
	}
	unsigned long time = (unsigned long)lround(2 * gbcc_audio.sample * (double)freq / SAMPLE_RATE);
	gbcc_audio.ch2.buffer[gbcc_audio.index] = (AUDIO_FMT)(volume * BASE_AMPLITUDE * (time & 1u));
}

void ch3_update(struct gbc *gbc)
{
	gbcc_audio.ch3.buffer[gbcc_audio.index] = 0;
}

void ch4_update(struct gbc *gbc)
{
	gbcc_audio.ch4.buffer[gbcc_audio.index] = 0;
}

uint64_t time_diff(const struct timespec * const cur,
		const struct timespec * const old)
{
	uint64_t sec = cur->tv_sec - old->tv_sec;
	uint64_t nsec;
	if (sec == 0) {
		nsec = cur->tv_nsec - old->tv_nsec;
		return nsec;
	}
	nsec = NANOSECOND * sec;
	if (old->tv_nsec > cur->tv_nsec) {
		nsec -= old->tv_nsec - cur->tv_nsec;
	} else {
		nsec += cur->tv_nsec - old->tv_nsec;
	}
	return nsec;
}
