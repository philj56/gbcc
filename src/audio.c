/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#include "gbcc.h"
#include "audio.h"
#include "bit_utils.h"
#include "debug.h"
#include "memory.h"
#include "nelem.h"
#include "time_diff.h"
#include "wav.h"

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Max amplitude / no. channels / max global volume multiplier */
#define MAX_CHANNEL_AMPLITUDE (INT16_MAX / 4 / 0x10u)
/* Max channel amplitude / max envelope volume multiplier */
#define BASE_AMPLITUDE (MAX_CHANNEL_AMPLITUDE / 0x10u)
#define CLOCKS_PER_SAMPLE ((float)GBC_CLOCK_FREQ / (float)GBCC_SAMPLE_RATE)

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void ch1_update(struct gbcc *gbc);
static void ch2_update(struct gbcc *gbc);
static void ch3_update(struct gbcc *gbc);
static void ch4_update(struct gbcc *gbc);
static int check_openal_error(const char *msg);
static void *wav_thread(void *filename);

void gbcc_audio_initialise(struct gbcc *gbc)
{
	struct gbcc_audio *audio = &gbc->audio;
	audio->scale = 0.9955;
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
	if (check_openal_error("Failed to create source.\n")) {
		exit(EXIT_FAILURE);
	}

	alSourcef(audio->al.source, AL_PITCH, 1);
	if (check_openal_error("Failed to set pitch.\n")) {
		exit(EXIT_FAILURE);
	}
	alSourcef(audio->al.source, AL_GAIN, 1);
	if (check_openal_error("Failed to set gain.\n")) {
		exit(EXIT_FAILURE);
	}
	alSource3f(audio->al.source, AL_POSITION, 0, 0, 0);
	if (check_openal_error("Failed to set position.\n")) {
		exit(EXIT_FAILURE);
	}
	alSource3f(audio->al.source, AL_VELOCITY, 0, 0, 0);
	if (check_openal_error("Failed to set velocity.\n")) {
		exit(EXIT_FAILURE);
	}
	alSourcei(audio->al.source, AL_LOOPING, AL_FALSE);
	if (check_openal_error("Failed to set loop.\n")) {
		exit(EXIT_FAILURE);
	}

	alGenBuffers(N_ELEM(audio->al.buffers), audio->al.buffers);
	if (check_openal_error("Failed to create buffers.\n")) {
		exit(EXIT_FAILURE);
	}

	clock_gettime(CLOCK_REALTIME, &audio->start_time);
	memset(audio->mix_buffer, 0, sizeof(audio->mix_buffer));
	for (size_t i = 0; i < N_ELEM(audio->al.buffers); i++) {
		alBufferData(audio->al.buffers[i], AL_FORMAT_STEREO16, audio->mix_buffer, sizeof(audio->mix_buffer), GBCC_SAMPLE_RATE);
	}
	alSourceQueueBuffers(audio->al.source, N_ELEM(audio->al.buffers), audio->al.buffers);
	check_openal_error("Failed to queue buffers.\n");
	alSourcePlay(audio->al.source);
	check_openal_error("Failed to play audio.\n");
}

void gbcc_audio_destroy(struct gbcc *gbc) {
	alDeleteSources(1, &gbc->audio.al.source);
	alDeleteBuffers(N_ELEM(gbc->audio.al.buffers), gbc->audio.al.buffers);
	alcDestroyContext(gbc->audio.al.context);
	alcCloseDevice(gbc->audio.al.device);
}

ANDROID_INLINE
void gbcc_audio_update(struct gbcc *gbc)
{
	struct gbcc_audio *audio = &gbc->audio;
	float mult = 1;
	if (gbc->core.keys.turbo) {
		if (gbc->turbo_speed > 0) {
			mult = gbc->turbo_speed;
		} else {
			return;
		}
	}
	audio->clock++;
	if (gbc->core.sync_to_video) {
		mult /= audio->scale;
	}
	/* When err > 0, it tells us how much we overshot the last sample by */
	float err = audio->clock - CLOCKS_PER_SAMPLE * mult * audio->sample;
	if (err >= 0) {
		ALint processed = 0;
		alGetSourcei(audio->al.source, AL_BUFFERS_PROCESSED, &processed);
		if (gbc->core.sync_to_video) {
			/*
			 * Some minor black magic to keep our audio buffers
			 * being filled at the correct rate. If we overshot
			 * this sample, we take the next sample slightly early.
			 *
			 * (processed * 0.1) is a factor controlling how strong
			 * this effect is - really it should be 1, but then we
			 * get horrible wobble in the audio. Instead I fudge
			 * it a little here, being more aggressive the more
			 * we're lagging behind to avoid filling all our
			 * buffers and producing a stutter.
			 *
			 * A better way to achieve this would be very much
			 * appreciated :)
			 */
			if (err < 1) audio->clock += err * processed * 0.1f;
		}
		if (audio->index >= GBCC_AUDIO_BUFSIZE) {
			while (!processed) {
				const struct timespec time = {.tv_sec = 0, .tv_nsec = 100000};
				nanosleep(&time, NULL);
				alGetSourcei(audio->al.source, AL_BUFFERS_PROCESSED, &processed);
			}
			ALuint buffer;
			alSourceUnqueueBuffers(audio->al.source, 1, &buffer);
			check_openal_error("Failed to unqueue buffer.\n");
			alBufferData(buffer, AL_FORMAT_STEREO16, audio->mix_buffer, audio->index * sizeof(audio->mix_buffer[0]), GBCC_SAMPLE_RATE);
			check_openal_error("Failed to fill buffer.\n");
			alSourceQueueBuffers(audio->al.source, 1, &buffer);
			check_openal_error("Failed to queue buffer.\n");
			audio->index = 0;
			ALint state;
			alGetSourcei(audio->al.source, AL_SOURCE_STATE, &state);
			check_openal_error("Failed to get source state.\n");
			if (state == AL_STOPPED) {
				clock_gettime(CLOCK_REALTIME, &gbc->core.apu.start_time);
				gbc->core.apu.sample = 0;
				alSourcePlay(audio->al.source);
				check_openal_error("Failed to resume audio playback.\n");
			}
			audio->clock = 0;
			audio->sample = 0;
		}
		audio->sample++;
		audio->mix_buffer[audio->index] = 0;
		audio->mix_buffer[audio->index + 1] = 0;
		ch1_update(gbc);
		ch2_update(gbc);
		ch3_update(gbc);
		ch4_update(gbc);
		audio->mix_buffer[audio->index] *= (1 + gbc->core.apu.left_vol);
		audio->mix_buffer[audio->index + 1] *= (1 + gbc->core.apu.right_vol);
		audio->index += 2;
	}
}

void ch1_update(struct gbcc *gbc)
{
	struct channel *ch1 = &gbc->core.apu.ch1;
	if (ch1->dac) {
		/* The DAC produces a signal in the range [-1, 1] */
		gbc->audio.mix_buffer[gbc->audio.index] -= MAX_CHANNEL_AMPLITUDE / 2;
		gbc->audio.mix_buffer[gbc->audio.index + 1] -= MAX_CHANNEL_AMPLITUDE / 2;
	}
	if (!ch1->enabled) {
		return;
	}
	uint8_t volume = ch1->envelope.volume;
	gbc->audio.mix_buffer[gbc->audio.index] += (GBCC_AUDIO_FMT)(ch1->left * ch1->state * volume * BASE_AMPLITUDE);
	gbc->audio.mix_buffer[gbc->audio.index + 1] += (GBCC_AUDIO_FMT)(ch1->right * ch1->state * volume * BASE_AMPLITUDE);
}

void ch2_update(struct gbcc *gbc)
{
	struct channel *ch2 = &gbc->core.apu.ch2;
	if (ch2->dac) {
		/* The DAC produces a signal in the range [-1, 1] */
		gbc->audio.mix_buffer[gbc->audio.index] -= MAX_CHANNEL_AMPLITUDE / 2;
		gbc->audio.mix_buffer[gbc->audio.index + 1] -= MAX_CHANNEL_AMPLITUDE / 2;
	}
	if (!ch2->enabled) {
		return;
	}
	uint8_t volume = ch2->envelope.volume;
	gbc->audio.mix_buffer[gbc->audio.index] += (GBCC_AUDIO_FMT)(ch2->left * ch2->state * volume * BASE_AMPLITUDE);
	gbc->audio.mix_buffer[gbc->audio.index + 1] += (GBCC_AUDIO_FMT)(ch2->right * ch2->state * volume * BASE_AMPLITUDE);
}

void ch3_update(struct gbcc *gbc)
{
	struct channel *ch3 = &gbc->core.apu.ch3;
	if (ch3->dac) {
		/* The DAC produces a signal in the range [-1, 1] */
		gbc->audio.mix_buffer[gbc->audio.index] -= MAX_CHANNEL_AMPLITUDE / 2;
		gbc->audio.mix_buffer[gbc->audio.index + 1] -= MAX_CHANNEL_AMPLITUDE / 2;
	}
	if (!ch3->enabled || gbc->core.apu.wave.shift == 0) {
		return;
	}
	gbc->audio.mix_buffer[gbc->audio.index] += (GBCC_AUDIO_FMT)(ch3->left * (gbc->core.apu.wave.buffer >> (gbc->core.apu.wave.shift - 1u)) * BASE_AMPLITUDE);
	gbc->audio.mix_buffer[gbc->audio.index + 1] += (GBCC_AUDIO_FMT)(ch3->right * (gbc->core.apu.wave.buffer >> (gbc->core.apu.wave.shift - 1u)) * BASE_AMPLITUDE);
}

void ch4_update(struct gbcc *gbc)
{
	struct channel *ch4 = &gbc->core.apu.ch4;
	if (ch4->dac) {
		/* The DAC produces a signal in the range [-1, 1] */
		gbc->audio.mix_buffer[gbc->audio.index] -= MAX_CHANNEL_AMPLITUDE / 2;
		gbc->audio.mix_buffer[gbc->audio.index + 1] -= MAX_CHANNEL_AMPLITUDE / 2;
	}
	if (!ch4->enabled) {
		return;
	}
	uint8_t volume = ch4->envelope.volume;
	gbc->audio.mix_buffer[gbc->audio.index] += (GBCC_AUDIO_FMT)(ch4->left * ch4->state * volume * BASE_AMPLITUDE);
	gbc->audio.mix_buffer[gbc->audio.index + 1] += (GBCC_AUDIO_FMT)(ch4->right * ch4->state * volume * BASE_AMPLITUDE);
}


int check_openal_error(const char *msg)
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


void gbcc_audio_play_wav(const char *filename)
{
	pthread_t thread;
	pthread_create(&thread, NULL, wav_thread, (void *)filename);
	pthread_detach(thread);
}

void *wav_thread(void *filename) {
	ALuint source;
	alGenSources(1, &source);
	if (check_openal_error("Failed to create source.\n")) {
		return NULL;
	}

	alSourcef(source, AL_PITCH, 1);
	if (check_openal_error("Failed to set pitch.\n")) {
		goto CLEANUP_SOURCE;
	}
	alSourcef(source, AL_GAIN, 1);
	if (check_openal_error("Failed to set gain.\n")) {
		goto CLEANUP_SOURCE;
	}
	alSource3f(source, AL_POSITION, 0, 0, 0);
	if (check_openal_error("Failed to set position.\n")) {
		goto CLEANUP_SOURCE;
	}
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	if (check_openal_error("Failed to set velocity.\n")) {
		goto CLEANUP_SOURCE;
	}
	alSourcei(source, AL_LOOPING, AL_FALSE);
	if (check_openal_error("Failed to set loop.\n")) {
		goto CLEANUP_SOURCE;
	}

	ALuint buffer;
	alGenBuffers(1, &buffer);
	if (check_openal_error("Failed to create buffer.\n")) {
		goto CLEANUP_SOURCE;
	}

	FILE *wav = fopen(filename, "rb");
	if (!wav) {
		gbcc_log_error("Failed to open print sound file.\n");
		goto CLEANUP_SOURCE;
	}
	struct wav_header header;
	wav_parse_header(&header, wav);
	if (header.AudioFormat != 1) {
		gbcc_log_error("Only PCM files are supported.\n");
		fclose(wav);
		goto CLEANUP_SOURCE;
	}
	uint8_t *data = malloc(header.Subchunk2Size);
	if (!data) {
		gbcc_log_error("Failed to allocate audio data buffer.\n");
		fclose(wav);
		goto CLEANUP_SOURCE;
	}
	if (fread(data, 1, header.Subchunk2Size, wav) == 0) {
		gbcc_log_error("Failed to read print audio data.\n");
		fclose(wav);
		goto CLEANUP_ALL;
	}
	fclose(wav);

	alBufferData(buffer, AL_FORMAT_MONO8, data, header.Subchunk2Size, header.SampleRate);
	if (check_openal_error("Failed to set buffer data.\n")) {
		goto CLEANUP_ALL;
	}
	alSourcei(source, AL_BUFFER, buffer);
	if (check_openal_error("Failed to set source buffer.\n")) {
		goto CLEANUP_ALL;
	}


	unsigned long long ns = SECOND * header.Subchunk2Size / header.ByteRate;

	struct timespec tts = {
		.tv_sec = ns / SECOND,
		.tv_nsec = ns % SECOND
	};
	alSourcePlay(source);
	nanosleep(&tts, NULL);

CLEANUP_ALL:
	free(data);
CLEANUP_SOURCE:
	alDeleteSources(1, &source);
	check_openal_error("Failed to delete source.\n");
	return NULL;
}
