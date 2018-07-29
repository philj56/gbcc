#include "gbcc.h"
#include "gbcc_audio.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

#define NANOSECOND 1000000000lu
#define BASE_AMPLITUDE (UINT16_MAX / 8 / 0x0F)
#define SAMPLE_RATE 48000
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)
#define SLEEP_TIME (NANOSECOND / SAMPLE_RATE)
#define SLEEP_DETECT NANOSECOND

static int audio_thread_function(void *_audio);
static void ch1_update(struct gbcc_audio *audio);
static void ch2_update(struct gbcc_audio *audio);
static void ch3_update(struct gbcc_audio *audio);
static void ch4_update(struct gbcc_audio *audio);
static void time_sync(struct gbcc_audio *audio);
static uint64_t time_diff(const struct timespec *cur,
		const struct timespec *old);

struct gbcc_audio *gbcc_audio_initialise(struct gbc *gbc)
{
	SDL_Init(0);

	struct gbcc_audio *audio = malloc(sizeof(struct gbcc_audio));
	audio->gbc = gbc;
	audio->index = 0;
	audio->quit = false;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to initialize SDL: %s\n", SDL_GetError());
	}

	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 1;
	want.samples = 0;
	want.callback = NULL;
	want.userdata = NULL;

	audio->device = SDL_OpenAudioDevice(NULL, 0, &want, &audio->audiospec, 0);
	if (audio->device == 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to open audio: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (want.format != audio->audiospec.format) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to get the desired AudioSpec.\n");
		exit(EXIT_FAILURE);
	}
	
	audio->thread = SDL_CreateThread(
			audio_thread_function, 
			"AudioThread", 
			(void *)(audio));

	if (audio->thread == NULL) {
		gbcc_log(GBCC_LOG_ERROR, "Error creating audio thread: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	SDL_PauseAudioDevice(audio->device, 0);
	return audio;
}

int audio_thread_function(void *_audio)
{
	struct gbcc_audio *audio = (struct gbcc_audio *)_audio;

	clock_gettime(CLOCK_REALTIME, &audio->start_time);
	while (!audio->quit) {
		time_sync(audio);
		audio->mix_buffer[audio->index] = 0;
		ch1_update(audio);
		ch2_update(audio);
		ch3_update(audio);
		ch4_update(audio);
		audio->index++;
		audio->sample++;
		if (audio->index == GBCC_AUDIO_BUFSIZE) {
			audio->index = 0;
			SDL_QueueAudio(audio->device, audio->mix_buffer, GBCC_AUDIO_BUFSIZE * sizeof(GBCC_AUDIO_FMT));
		}
	}

	return 0;
}

void ch1_update(struct gbcc_audio *audio)
{
	if (!audio->gbc->apu.ch1.enabled) {
		return;
	}
	uint8_t volume = audio->gbc->apu.ch1.envelope.volume;
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(audio->gbc->apu.ch1.state * volume * BASE_AMPLITUDE);
}

void ch2_update(struct gbcc_audio *audio)
{
	if (!audio->gbc->apu.ch2.enabled) {
		return;
	}
	uint8_t volume = audio->gbc->apu.ch2.envelope.volume;
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(audio->gbc->apu.ch2.state * volume * BASE_AMPLITUDE);
}

void ch3_update(struct gbcc_audio *audio)
{
	if (!audio->gbc->apu.ch3.enabled || audio->gbc->apu.wave.shift == 0) {
		return;
	}
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)((audio->gbc->apu.wave.buffer >> (audio->gbc->apu.wave.shift - 1u)) * BASE_AMPLITUDE);
}

void ch4_update(struct gbcc_audio *audio)
{
	if (!audio->gbc->apu.ch4.enabled) {
		return;
	}
	uint8_t volume = audio->gbc->apu.ch4.envelope.volume;
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(audio->gbc->apu.ch4.state * volume * BASE_AMPLITUDE);
}

void time_sync(struct gbcc_audio *audio)
{
	clock_gettime(CLOCK_REALTIME, &audio->cur_time);
	uint64_t diff = time_diff(&audio->cur_time, &audio->start_time);
	if (diff > SLEEP_DETECT + (NANOSECOND * audio->sample) / SAMPLE_RATE) {
		const struct timespec time = {.tv_sec = 2, .tv_nsec = 0};
		nanosleep(&time, NULL);
		audio->start_time.tv_sec = 0;
		audio->sample = 0;
		return;
	}
	while (diff < (NANOSECOND * audio->sample) / SAMPLE_RATE) {
		const struct timespec time = {.tv_sec = 0, .tv_nsec = SLEEP_TIME};
		nanosleep(&time, NULL);
		clock_gettime(CLOCK_REALTIME, &audio->cur_time);
		diff = time_diff(&audio->cur_time, &audio->start_time);
	}
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
