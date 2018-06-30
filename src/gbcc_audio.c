#include "gbcc.h"
#include "gbcc_audio.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

/* 4 channels, with volume 0x00-0x0F each */
#define BASE_AMPLITUDE (INT16_MAX / 4 / 0x0F)
#define SAMPLE_RATE 48000
#define BUFSIZE SAMPLE_RATE
#define SYNC_PERIOD 1e6 /* nanoseconds */
#define SYNC_CLOCKS (SYNC_PERIOD / GBC_CLOCK_PERIOD)

struct gbcc_audio {
	SDL_AudioSpec audiospec;
	SDL_AudioDeviceID device;
	struct timespec cur_time;
	struct timespec old_time;
	uint64_t cur_clock;
	uint64_t old_clock;
	Sint16 mix_buffer[BUFSIZE];
	Sint16 ch1_buffer[BUFSIZE]; 	/* Tone & Sweep */
	Sint16 ch2_buffer[BUFSIZE]; 	/* Tone */
	Sint16 ch3_buffer[BUFSIZE]; 	/* Wave Output */
	Sint16 ch4_buffer[BUFSIZE]; 	/* Noise */
};

static struct gbcc_audio gbcc_audio;

static void ch1_update(struct gbc *gbc, uint64_t nsamples);
static void ch2_update(struct gbc *gbc, uint64_t nsamples);
static void ch3_update(struct gbc *gbc, uint64_t nsamples);
static void ch4_update(struct gbc *gbc, uint64_t nsamples);
static void time_diff(const struct timespec *cur,
		const struct timespec *old,
		struct timespec *diff);

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
	timespec_get(&gbcc_audio.old_time, TIME_UTC);
	timespec_get(&gbcc_audio.cur_time, TIME_UTC);
	gbcc_audio.old_clock = 0;
	gbcc_audio.cur_clock = 0;

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
		return;
	}
	gbcc_audio.cur_clock = gbc->clock;
	uint64_t clocks = gbcc_audio.cur_clock - gbcc_audio.old_clock;
	if (clocks > SYNC_CLOCKS) {
		gbcc_audio.old_clock = gbcc_audio.cur_clock;
		uint64_t nsamples = 2*((double)clocks * GBC_CLOCK_PERIOD / 1e9) * SAMPLE_RATE;
		if (nsamples > BUFSIZE / 2) {
			return;
		}
		ch1_update(gbc, nsamples);
		ch2_update(gbc, nsamples);
		ch3_update(gbc, nsamples);
		ch4_update(gbc, nsamples);
		for (size_t i = 0; i < nsamples; i++) {
			gbcc_audio.mix_buffer[i] = gbcc_audio.ch1_buffer[i]
				+ gbcc_audio.ch2_buffer[i]
				+ gbcc_audio.ch3_buffer[i]
				+ gbcc_audio.ch4_buffer[i];
		}
		SDL_QueueAudio(gbcc_audio.device, gbcc_audio.mix_buffer, nsamples);
		timespec_get(&gbcc_audio.cur_time, TIME_UTC);
		struct timespec diff;
		time_diff(&gbcc_audio.cur_time, &gbcc_audio.old_time, &diff);
		if (diff.tv_nsec < SYNC_PERIOD) {
			diff.tv_nsec = SYNC_PERIOD - diff.tv_nsec;
			nanosleep(&diff, NULL);
		}
		gbcc_audio.old_time = gbcc_audio.cur_time;
	}
}

void ch1_update(struct gbc *gbc, uint64_t nsamples)
{
	static uint64_t sample = 0;
	static uint16_t old_freq = 0;
	static FILE *file = NULL;
	if (file == NULL) {
		file = fopen("sound.wav", "wbe");
	}
	uint8_t volume = (gbcc_memory_read(gbc, NR12, true) & 0xF0u) >> 4u;
	uint16_t freq = gbcc_memory_read(gbc, NR13, true);
	freq |= (gbcc_memory_read(gbc, NR14, true) & 0x03u) << 8u;
	freq &= ~3;
	if (freq != old_freq) {
		sample = 0;
		old_freq = freq;
	}
	printf("%u:\t%lu\n", freq, sample);
	for (size_t i = 0; i < nsamples; i++, sample++) {
		unsigned int time = lround(2 * sample * (double)freq / SAMPLE_RATE);
		gbcc_audio.ch1_buffer[i] = (Sint16)(volume * BASE_AMPLITUDE * (time & 1u));
	}
	//fwrite(gbcc_audio.ch2_buffer, 2, nsamples, file);
}

void ch2_update(struct gbc *gbc, uint64_t nsamples)
{
	static uint64_t sample = 0;
	static uint16_t old_freq = 0;
	static FILE *file = NULL;
	if (file == NULL) {
		file = fopen("sound.wav", "wbe");
	}
	uint8_t volume = (gbcc_memory_read(gbc, NR22, true) & 0xF0u) >> 4u;
	uint16_t freq = gbcc_memory_read(gbc, NR23, true);
	freq |= (gbcc_memory_read(gbc, NR24, true) & 0x03u) << 8u;
	freq &= ~3;
	if (freq != old_freq) {
		sample = 0;
		old_freq = freq;
	}
	printf("%u:\t%lu\n", freq, sample);
	for (size_t i = 0; i < nsamples; i++, sample++) {
		unsigned int time = lround(2 * sample * (double)freq / SAMPLE_RATE);
		gbcc_audio.ch2_buffer[i] = (Sint16)(volume * BASE_AMPLITUDE * (time & 1u));
	}
	//fwrite(gbcc_audio.ch2_buffer, 2, nsamples, file);
}

void ch3_update(struct gbc *gbc, uint64_t nsamples)
{
	static uint64_t sample = 0;
	for (size_t i = 0; i < nsamples; i++, sample++) {
		gbcc_audio.ch3_buffer[i] = 0;
	}
}

void ch4_update(struct gbc *gbc, uint64_t nsamples)
{
	static uint64_t sample = 0;
	for (size_t i = 0; i < nsamples; i++, sample++) {
		gbcc_audio.ch4_buffer[i] = 0;
	}
}

void time_diff(const struct timespec * const cur,
		const struct timespec * const old,
		struct timespec *diff)
{
	diff->tv_sec = cur->tv_sec - old->tv_sec;
	if (diff->tv_sec == 0) {
		diff->tv_nsec = cur->tv_nsec - old->tv_nsec;
		return;
	}
	diff->tv_nsec = 1e9;// * diff->tv_sec;
	diff->tv_sec = 0;
	if (old->tv_nsec > cur->tv_nsec) {
		diff->tv_nsec -= old->tv_nsec - cur->tv_nsec;
	} else {
		diff->tv_nsec = cur->tv_nsec - old->tv_nsec;
	}
}
