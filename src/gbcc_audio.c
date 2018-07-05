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
#define BASE_AMPLITUDE (UINT16_MAX / 8 / 0x0F)
#define SAMPLE_RATE 48000
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)
#define BUFSIZE 512 /* samples */
#define AUDIO_FMT Uint16
#define SLEEP_TIME (NANOSECOND / SAMPLE_RATE)
#define SEQUENCER_CLOCKS 8192

struct timer {
	uint16_t period;
	uint16_t counter;
};
struct duty {
	struct timer freq_timer;
	struct timer duty_timer;
	uint8_t cycle;
	uint16_t freq;
};

struct sweep {
	uint16_t freq;
	struct timer timer;
	uint8_t shift;
	int8_t dir;
};

struct noise {
	struct timer timer;
	uint16_t lfsr;
};

struct wave {
	struct timer timer;
	uint16_t addr;
	uint8_t buffer;
	uint8_t nibble;
	uint8_t shift;
};

struct envelope {
	struct timer timer;
	uint8_t volume;
	bool enabled;
};

struct channel {
	AUDIO_FMT buffer[BUFSIZE];
	uint8_t counter;
	bool state;
	bool enabled;
	struct envelope envelope;
	struct duty duty;
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
	struct sweep sweep;
	struct noise noise;
	struct wave wave;
	struct timer sequencer_timer;
};

static struct apu apu;
static const uint8_t duty_table[4] = {
	0x01, 	/* 00000001b */
	0x81, 	/* 10000001b */
	0x87, 	/* 10000111b */
	0x7E 	/* 01111110b */
};

static bool timer_clock(struct timer *timer);
static void timer_reset(struct timer *timer);
static bool duty_clock(struct duty *duty);
static void envelope_clock(struct envelope *envelope, uint8_t nrx2);
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
	apu.sequencer_timer.period = 8;
	timer_reset(&apu.sequencer_timer);
	apu.ch1.duty.duty_timer.period = 8;
	apu.ch2.duty.duty_timer.period = 8;
	timer_reset(&apu.ch1.duty.duty_timer);
	timer_reset(&apu.ch2.duty.duty_timer);
	apu.ch1.envelope.timer.period = 8;
	apu.ch2.envelope.timer.period = 8;
	apu.ch4.envelope.timer.period = 8;
	timer_reset(&apu.ch1.envelope.timer);
	timer_reset(&apu.ch2.envelope.timer);
	timer_reset(&apu.ch4.envelope.timer);
	apu.sweep.timer.period = 8;
	timer_reset(&apu.sweep.timer);
	apu.noise.timer.period = 8;
	timer_reset(&apu.noise.timer);
	apu.wave.timer.period = 8;
	timer_reset(&apu.wave.timer);
	apu.wave.addr = WAVE_START;
	apu.ch1.enabled = false;
	apu.ch2.enabled = false;
	apu.ch3.enabled = false;
	apu.ch4.enabled = false;

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

	/* Duty */
	apu.ch1.duty.cycle = (gbcc_memory_read(gbc, NR11, true) & 0xC0u) >> 6u;
	apu.ch1.duty.freq = gbcc_memory_read(gbc, NR13, true);
	apu.ch1.duty.freq |= (gbcc_memory_read(gbc, NR14, true) & 0x07u) << 8u;
	apu.ch1.state = duty_clock(&apu.ch1.duty);

	apu.ch2.duty.cycle = (gbcc_memory_read(gbc, NR21, true) & 0xC0u) >> 6u;
	apu.ch2.duty.freq = gbcc_memory_read(gbc, NR23, true);
	apu.ch2.duty.freq |= (gbcc_memory_read(gbc, NR24, true) & 0x07u) << 8u;
	apu.ch2.state = duty_clock(&apu.ch2.duty);

	/* Noise */
	apu.noise.timer.period = 0x10u * (gbcc_memory_read(gbc, NR43, true) & 0x07u);
	if (apu.noise.timer.period == 0) {
		apu.noise.timer.period = 0x08u;
	}
	apu.noise.timer.period <<= (gbcc_memory_read(gbc, NR43, true) & 0xF0u) >> 4u;
	if (timer_clock(&apu.noise.timer)) {
		uint8_t tmp = check_bit(apu.noise.lfsr, 0) ^ check_bit(apu.noise.lfsr, 1);
		apu.noise.lfsr >>= 1;
		apu.noise.lfsr |= tmp * (1u << 14u);
		if (check_bit(gbcc_memory_read(gbc, NR43, true), 3)) {
			apu.noise.lfsr |= tmp * bit(6);
		}
		apu.ch4.state = !(apu.noise.lfsr & bit(0));
	}

	/* Wave */
	apu.wave.timer.period = gbcc_memory_read(gbc, NR33, true);
	apu.wave.timer.period |= (gbcc_memory_read(gbc, NR34, true) & 0x07u) << 8u;
	apu.wave.timer.period = (2048u - apu.wave.timer.period) / 2;
	if (timer_clock(&apu.wave.timer)) {
		apu.wave.shift = (gbcc_memory_read(gbc, NR32, true) & 0x60u) >> 5u;
		apu.wave.addr += apu.wave.nibble;
		apu.wave.nibble = !apu.wave.nibble;
		if (apu.wave.addr > WAVE_END) {
			apu.wave.addr = WAVE_START;
		}
		apu.wave.buffer = gbcc_memory_read(gbc, apu.wave.addr, true);
		apu.wave.buffer &= 0x0Fu << (4u * apu.wave.nibble);
		apu.wave.buffer >>= 4u * apu.wave.nibble;
	}

	uint64_t clocks = gbc->clock - apu.sample_clock;
	if (!(gbc->clock % SEQUENCER_CLOCKS)) {
		sequencer_clock(gbc);
	}
	if (clocks >= CLOCKS_PER_SAMPLE) {
		clock_gettime(CLOCK_REALTIME, &apu.cur_time);
		uint64_t diff = time_diff(&apu.cur_time, &apu.start_time);
		while (diff < (NANOSECOND * apu.sample) / SAMPLE_RATE) {
			const struct timespec time = {.tv_sec = 0, .tv_nsec = SLEEP_TIME};
			nanosleep(&time, NULL);
			clock_gettime(CLOCK_REALTIME, &apu.cur_time);
			diff = time_diff(&apu.cur_time, &apu.start_time);
		}
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

bool timer_clock(struct timer *timer)
{
	if (--timer->counter == 0) {
		timer_reset(timer);
		return true;
	}
	return false;
}

void timer_reset(struct timer *timer)
{
	timer->counter = timer->period;
}

bool duty_clock(struct duty *duty)
{
	duty->freq_timer.period = 2048u - duty->freq;
	if (timer_clock(&duty->freq_timer)) {
		timer_clock(&duty->duty_timer);
	}
	return duty_table[duty->cycle] & bit(8-duty->duty_timer.counter);
}

void envelope_clock(struct envelope *envelope, uint8_t nrx2)
{
	if (!envelope->enabled) {
		return;
	}
	envelope->timer.period = nrx2 & 0x07u;
	int dir = nrx2 & 0x08u ? 1 : -1;
	if (envelope->timer.period == 0) {
		envelope->timer.period = 8;
	}
	if (timer_clock(&envelope->timer)) {
		envelope->volume += dir;
		if (envelope->volume == 0x10u || envelope->volume == 0xFFu) {
			envelope->volume -= dir;
			envelope->enabled = false;
		}
	}
}

void sequencer_clock(struct gbc *gbc)
{
	/* DAC */
	if (!(gbcc_memory_read(gbc, NR12, true) & 0xF7u)) {
		apu.ch1.enabled = false;
	}
	if (!(gbcc_memory_read(gbc, NR22, true) & 0xF7u)) {
		apu.ch2.enabled = false;
	}
	if (!(gbcc_memory_read(gbc, NR42, true) & 0xF7u)) {
		apu.ch4.enabled = false;
	}

	/* Length counters every other clock */
	if (!(apu.sequencer_timer.counter & 0x01u)) {
		if (gbcc_memory_read(gbc, NR14, true) & bit(6)) {
			apu.ch1.counter--;
			if (apu.ch1.counter == 0) {
				apu.ch1.enabled = false;
			}
		}
		if (gbcc_memory_read(gbc, NR24, true) & bit(6)) {
			apu.ch2.counter--;
			if (apu.ch2.counter == 0) {
				apu.ch2.enabled = false;
			}
		}
		if (gbcc_memory_read(gbc, NR34, true) & bit(6)) {
			apu.ch3.counter--;
			if (apu.ch3.counter == 0) {
				apu.ch3.enabled = false;
			}
		}
		if (gbcc_memory_read(gbc, NR44, true) & bit(6)) {
			apu.ch4.counter--;
			if (apu.ch4.counter == 0) {
				apu.ch4.enabled = false;
			}
		}
	}

	/* Sweep on clocks 2 & 6 */
	if (apu.sequencer_timer.counter == 2u || apu.sequencer_timer.counter == 6u) {
		apu.sweep.timer.period = (gbcc_memory_read(gbc, NR10, true) & 0x70u) >> 4u;
		apu.sweep.dir = gbcc_memory_read(gbc, NR10, true) & 0x07u ? 1 : -1;
		if (apu.sweep.timer.period == 0) {
			apu.sweep.timer.period = 8;
		}
		if (timer_clock(&apu.sweep.timer)) {
			uint64_t freq = apu.sweep.freq >> apu.sweep.shift;
			freq = apu.sweep.freq + apu.sweep.dir * freq;
			if (apu.sweep.shift != 0 && freq < 2047) {
				apu.sweep.freq = freq;
				apu.ch1.duty.freq = freq;
			}
		}
	}

	/* Envelope */
	if (apu.sequencer_timer.counter == 7u) {
		envelope_clock(&apu.ch1.envelope, gbcc_memory_read(gbc, NR12, true));
		envelope_clock(&apu.ch2.envelope, gbcc_memory_read(gbc, NR22, true));
		envelope_clock(&apu.ch4.envelope, gbcc_memory_read(gbc, NR42, true));
	}

	/* Restart sounds */
	if (gbcc_memory_read(gbc, NR14, true) & bit(7)) {
		apu.ch1.enabled = true;
		if (apu.ch1.counter == 0) {
			apu.ch1.counter = 64;
		}
		timer_reset(&apu.ch1.duty.freq_timer);
		timer_reset(&apu.ch1.envelope.timer);
		apu.ch1.envelope.enabled = true;
		apu.ch1.envelope.volume = (gbcc_memory_read(gbc, NR12, true) & 0xF0u) >> 4u;
		gbcc_memory_clear_bit(gbc, NR14, 7, true);
	}
	if (gbcc_memory_read(gbc, NR24, true) & bit(7)) {
		apu.ch2.enabled = true;
		if (apu.ch2.counter == 0) {
			apu.ch2.counter = 64;
		}
		timer_reset(&apu.ch2.duty.freq_timer);
		timer_reset(&apu.ch2.envelope.timer);
		apu.ch2.envelope.enabled = true;
		apu.ch2.envelope.volume = (gbcc_memory_read(gbc, NR22, true) & 0xF0u) >> 4u;
		gbcc_memory_clear_bit(gbc, NR24, 7, true);
	}
	if (gbcc_memory_read(gbc, NR34, true) & bit(7)) {
		apu.ch3.enabled = true;
		if (apu.ch3.counter == 0) {
			apu.ch3.counter = 64;
		}
		timer_reset(&apu.wave.timer);
		apu.wave.addr = WAVE_START;
		gbcc_memory_clear_bit(gbc, NR34, 7, true);
	}
	if (gbcc_memory_read(gbc, NR44, true) & bit(7)) {
		apu.ch4.enabled = true;
		if (apu.ch4.counter == 0) {
			apu.ch4.counter = 64;
		}
		timer_reset(&apu.noise.timer);
		timer_reset(&apu.ch4.envelope.timer);
		apu.ch4.envelope.enabled = true;
		apu.ch4.envelope.volume = (gbcc_memory_read(gbc, NR42, true) & 0xF0u) >> 4u;
		apu.noise.lfsr = 0xFFu;
		gbcc_memory_clear_bit(gbc, NR44, 7, true);
	}

	timer_clock(&apu.sequencer_timer);
}

void ch1_update(struct gbc *gbc)
{
	if (!apu.ch1.enabled) {
		apu.ch1.buffer[apu.index] = 0;
		return;
	}
	uint8_t volume = apu.ch1.envelope.volume;
	apu.ch1.buffer[apu.index] = (AUDIO_FMT)(apu.ch1.state * volume * BASE_AMPLITUDE);
}

void ch2_update(struct gbc *gbc)
{
	if (!apu.ch2.enabled) {
		apu.ch2.buffer[apu.index] = 0;
		return;
	}
	uint8_t volume = apu.ch2.envelope.volume;
	apu.ch2.buffer[apu.index] = (AUDIO_FMT)(apu.ch2.state * volume * BASE_AMPLITUDE);
}

void ch3_update(struct gbc *gbc)
{
	if (!apu.ch3.enabled || apu.wave.shift == 0) {
		apu.ch3.buffer[apu.index] = 0;
		return;
	}
	apu.ch3.buffer[apu.index] = (AUDIO_FMT)((apu.wave.buffer >> (apu.wave.shift - 1)) * BASE_AMPLITUDE);
}

void ch4_update(struct gbc *gbc)
{
	if (!apu.ch4.enabled) {
		apu.ch4.buffer[apu.index] = 0;
		return;
	}
	uint8_t volume = apu.ch4.envelope.volume;
	apu.ch4.buffer[apu.index] = (AUDIO_FMT)(apu.ch4.state * volume * BASE_AMPLITUDE);
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
