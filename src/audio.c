#include "gbcc.h"
#include "audio.h"
#include "bit_utils.h"
#include "debug.h"
#include "memory.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <stdint.h>
#include <time.h>

#define BASE_AMPLITUDE (UINT16_MAX / 4 / 0x0F / 0x10u)
#define SAMPLE_RATE 65536
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)

static void ch1_update(struct gbcc_audio *audio);
static void ch2_update(struct gbcc_audio *audio);
static void ch3_update(struct gbcc_audio *audio);
static void ch4_update(struct gbcc_audio *audio);

void gbcc_audio_initialise(struct gbcc_audio *audio, struct gbc *gbc)
{
	audio->gbc = gbc;
	audio->al.device = alcOpenDevice(NULL);
	if (!audio->al.device) {
		gbcc_log_error("Failed to open audio device.\n");
	}
	audio->al.context = alcCreateContext(audio->al.device, NULL);
	if (!audio->al.context) {
		gbcc_log_error("Failed to create OpenAL context.\n");
		exit(EXIT_FAILURE);
	}
	if (!alcMakeContextCurrent(audio->al.context)) {
		gbcc_log_error("Failed to set OpenAL context.\n");
		exit(EXIT_FAILURE);
	}

	alGenSources(1, &audio->al.source);
	if (gbcc_check_openal_error("Failed to create source.\n")) {
		exit(EXIT_FAILURE);
	}

	alSourcef(audio->al.source, AL_PITCH, 1);
	if (gbcc_check_openal_error("Failed to set pitch.\n")) {
		exit(EXIT_FAILURE);
	}
	alSourcef(audio->al.source, AL_GAIN, 1);
	if (gbcc_check_openal_error("Failed to set gain.\n")) {
		exit(EXIT_FAILURE);
	}
	alSource3f(audio->al.source, AL_POSITION, 0, 0, 0);
	if (gbcc_check_openal_error("Failed to set position.\n")) {
		exit(EXIT_FAILURE);
	}
	alSource3f(audio->al.source, AL_VELOCITY, 0, 0, 0);
	if (gbcc_check_openal_error("Failed to set velocity.\n")) {
		exit(EXIT_FAILURE);
	}
	alSourcei(audio->al.source, AL_LOOPING, AL_FALSE);
	if (gbcc_check_openal_error("Failed to set loop.\n")) {
		exit(EXIT_FAILURE);
	}

	alGenBuffers(sizeof(audio->al.buffers) / sizeof(audio->al.buffers[0]), audio->al.buffers);
	if (gbcc_check_openal_error("Failed to create buffers.\n")) {
		exit(EXIT_FAILURE);
	}

	clock_gettime(CLOCK_REALTIME, &audio->start_time);
	for (int i = 0; i < sizeof(audio->al.buffers) / sizeof(audio->al.buffers[0]); i++) {
		alBufferData(audio->al.buffers[i], AL_FORMAT_STEREO16, audio->mix_buffer, sizeof(audio->mix_buffer), SAMPLE_RATE);
	}
	alSourceQueueBuffers(audio->al.source, sizeof(audio->al.buffers) / sizeof(audio->al.buffers[0]), audio->al.buffers);
	gbcc_check_openal_error("Failed to queue buffers.\n");
	alSourcePlay(audio->al.source);
	gbcc_check_openal_error("Failed to play audio.\n");
}

void gbcc_audio_destroy(struct gbcc_audio *audio) {
	alcCloseDevice(audio->al.device);
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
	if (audio->clock - audio->sample_clock >= CLOCKS_PER_SAMPLE * mult) {
		if (audio->index == GBCC_AUDIO_BUFSIZE) {
			ALint processed = 0;
			while (!processed) {
				alGetSourcei(audio->al.source, AL_BUFFERS_PROCESSED, &processed);
			}
			ALuint buffer;
			alSourceUnqueueBuffers(audio->al.source, 1, &buffer);
			gbcc_check_openal_error("Failed to unqueue buffer.\n");
			alBufferData(buffer, AL_FORMAT_STEREO16, audio->mix_buffer, audio->index * sizeof(audio->mix_buffer[0]), SAMPLE_RATE);
			gbcc_check_openal_error("Failed to fill buffer.\n");
			alSourceQueueBuffers(audio->al.source, 1, &buffer);
			gbcc_check_openal_error("Failed to queue buffer.\n");
			audio->index = 0;
			ALint state;
			alGetSourcei(audio->al.source, AL_SOURCE_STATE, &state);
			gbcc_check_openal_error("Failed to get source state.\n");
			if (state == AL_STOPPED) {
				clock_gettime(CLOCK_REALTIME, &audio->gbc->apu.start_time);
				audio->gbc->apu.sample = 0;
				alSourcePlay(audio->al.source);
				gbcc_check_openal_error("Failed to resume audio playback.\n");
			}
		}
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


int gbcc_check_openal_error(const char *msg)
{
	ALenum error = alGetError();
	switch (error) {
		case AL_NO_ERROR:
			return 0;
		case AL_INVALID_NAME:
			gbcc_log_error("Invalid name: %s", msg);
			break;
		case AL_INVALID_ENUM:
			gbcc_log_error("Invalid enum: %s", msg);
			break;
		case AL_INVALID_VALUE:
			gbcc_log_error("Invalid value: %s", msg);
			break;
		case AL_INVALID_OPERATION:
			gbcc_log_error("Invalid operation: %s", msg);
			break;
		case AL_OUT_OF_MEMORY:
			gbcc_log_error("Out of memory: %s", msg);
			break;
	}
	gbcc_log_error("%s", msg);
	return 1;
}
