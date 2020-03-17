/*
 * Copyright (C) 2017-2020 Philip Jones
 *
 * Licensed under the MIT License.
 * See either the LICENSE file, or:
 *
 * https://opensource.org/licenses/MIT
 *
 */

#ifndef GBCC_AUDIO_H
#define GBCC_AUDIO_H

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <stdint.h>
#include <time.h>

#ifdef __ANDROID__
#define GBCC_SAMPLE_RATE 44100
#define GBCC_AUDIO_BUFSIZE_SAMPLES 1024
#else
#define GBCC_SAMPLE_RATE 96000
#define GBCC_AUDIO_BUFSIZE_SAMPLES 2048
#endif
#define GBCC_AUDIO_BUFSIZE (GBCC_AUDIO_BUFSIZE_SAMPLES*2) /* samples * channels */
#define GBCC_AUDIO_FMT uint16_t

struct gbcc;

struct gbcc_audio {
	struct {
		ALCdevice *device;
		ALCcontext *context;
		ALuint source;
		ALuint buffers[8];
	} al;
	float clock;
	int sample;
	size_t index;
	struct timespec cur_time;
	struct timespec start_time;
	GBCC_AUDIO_FMT mix_buffer[GBCC_AUDIO_BUFSIZE];
	float scale;
};

void gbcc_audio_initialise(struct gbcc *gbc);
void gbcc_audio_destroy(struct gbcc *gbc);
void gbcc_audio_update(struct gbcc *gbc);
int gbcc_check_openal_error(const char *msg);

#endif /* GBCC_AUDIO_H */
