#include "gbcc.h"
#include "gbcc_audio.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

#define BASE_AMPLITUDE (UINT16_MAX / 8 / 0x0F)
#define SAMPLE_RATE 48000
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)

static void ch1_update(struct gbcc_audio *audio);
static void ch2_update(struct gbcc_audio *audio);
static void ch3_update(struct gbcc_audio *audio);
static void ch4_update(struct gbcc_audio *audio);

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
	want.channels = 2;
	want.samples = 0;
	want.callback = NULL;
	want.userdata = NULL;

	audio->sample_clock = 0;
	audio->device = SDL_OpenAudioDevice(NULL, 0, &want, &audio->audiospec, 0);
	if (audio->device == 0) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to open audio: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (want.format != audio->audiospec.format) {
		gbcc_log(GBCC_LOG_ERROR, "Failed to get the desired AudioSpec.\n");
		exit(EXIT_FAILURE);
	}
	
	clock_gettime(CLOCK_REALTIME, &audio->start_time);
	SDL_PauseAudioDevice(audio->device, 0);
	return audio;
}

void gbcc_audio_update(struct gbcc_audio *audio)
{
	if (audio->gbc->keys.turbo) {
		return;
	}
	if (audio->gbc->apu_clock - audio->sample_clock > CLOCKS_PER_SAMPLE) {
		audio->sample_clock = audio->gbc->apu_clock;
		audio->mix_buffer[audio->index] = 0;
		audio->mix_buffer[audio->index + 1] = 0;
		ch1_update(audio);
		ch2_update(audio);
		ch3_update(audio);
		ch4_update(audio);
		audio->index += 2;
		audio->sample++;
		if (audio->index == GBCC_AUDIO_BUFSIZE) {
			audio->index = 0;
			SDL_QueueAudio(audio->device, audio->mix_buffer, GBCC_AUDIO_BUFSIZE * sizeof(GBCC_AUDIO_FMT));
		}
	}
}

void ch1_update(struct gbcc_audio *audio)
{
	struct channel *ch1 = &audio->gbc->apu.ch1;
	if (!ch1->enabled) {
		return;
	}
	uint8_t volume = ch1->envelope.volume;
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(ch1->left * ch1->state * volume * BASE_AMPLITUDE);
	audio->mix_buffer[audio->index + 1] += (GBCC_AUDIO_FMT)(ch1->right * ch1->state * volume * BASE_AMPLITUDE);
}

void ch2_update(struct gbcc_audio *audio)
{
	struct channel *ch2 = &audio->gbc->apu.ch2;
	if (!ch2->enabled) {
		return;
	}
	uint8_t volume = ch2->envelope.volume;
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(ch2->left * ch2->state * volume * BASE_AMPLITUDE);
	audio->mix_buffer[audio->index + 1] += (GBCC_AUDIO_FMT)(ch2->right * ch2->state * volume * BASE_AMPLITUDE);
}

void ch3_update(struct gbcc_audio *audio)
{
	struct channel *ch3 = &audio->gbc->apu.ch3;
	if (!audio->gbc->apu.ch3.enabled || audio->gbc->apu.wave.shift == 0) {
		return;
	}
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(ch3->left * (audio->gbc->apu.wave.buffer >> (audio->gbc->apu.wave.shift - 1u)) * BASE_AMPLITUDE);
	audio->mix_buffer[audio->index + 1] += (GBCC_AUDIO_FMT)(ch3->right * (audio->gbc->apu.wave.buffer >> (audio->gbc->apu.wave.shift - 1u)) * BASE_AMPLITUDE);
}

void ch4_update(struct gbcc_audio *audio)
{
	struct channel *ch4 = &audio->gbc->apu.ch4;
	if (!ch4->enabled) {
		return;
	}
	uint8_t volume = ch4->envelope.volume;
	audio->mix_buffer[audio->index] += (GBCC_AUDIO_FMT)(ch4->left * ch4->state * volume * BASE_AMPLITUDE);
	audio->mix_buffer[audio->index + 1] += (GBCC_AUDIO_FMT)(ch4->right * ch4->state * volume * BASE_AMPLITUDE);
}
