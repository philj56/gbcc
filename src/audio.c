#include "gbcc.h"
#include "audio.h"
#include "bit_utils.h"
#include "debug.h"
#include "memory.h"
#include <SDL2/SDL.h>
#include <stdint.h>
#include <time.h>

#define BASE_AMPLITUDE (UINT16_MAX / 4 / 0x0F / 0x10u)
#define SAMPLE_RATE 44150
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)

static void ch1_update(struct gbcc_audio *audio);
static void ch2_update(struct gbcc_audio *audio);
static void ch3_update(struct gbcc_audio *audio);
static void ch4_update(struct gbcc_audio *audio);

void gbcc_audio_initialise(struct gbcc_audio *audio, struct gbc *gbc)
{
	audio->gbc = gbc;

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		gbcc_log_error("Failed to initialize SDL: %s\n", SDL_GetError());
	}

	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 0;
	want.callback = NULL;
	want.userdata = NULL;

	audio->device = SDL_OpenAudioDevice(NULL, 0, &want, &audio->audiospec, 0);
	if (audio->device == 0) {
		gbcc_log_error("Failed to open audio: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	if (want.format != audio->audiospec.format) {
		gbcc_log_error("Failed to get the desired AudioSpec.\n");
		exit(EXIT_FAILURE);
	}
	
	clock_gettime(CLOCK_REALTIME, &audio->start_time);
	SDL_PauseAudioDevice(audio->device, 0);
}

void gbcc_audio_update(struct gbcc_audio *audio)
{
	double mult = 1;
	if (audio->gbc->keys.turbo) {
		if (audio->gbc->turbo_speed > 0) {
			mult = audio->gbc->turbo_speed;
		} else {
			return;
		}
	}
	audio->clock++;
	if (audio->clock - audio->sample_clock > CLOCKS_PER_SAMPLE * mult) {
		audio->sample_clock = audio->clock;
		audio->mix_buffer[audio->index] = 0;
		audio->mix_buffer[audio->index + 1] = 0;
		ch1_update(audio);
		ch2_update(audio);
		ch3_update(audio);
		ch4_update(audio);
		audio->mix_buffer[audio->index] *= (1 + audio->gbc->apu.left_vol);
		audio->mix_buffer[audio->index + 1] *= (1 + audio->gbc->apu.right_vol);
		audio->index += 2;
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
