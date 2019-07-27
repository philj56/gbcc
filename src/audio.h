#ifndef GBCC_AUDIO_H
#define GBCC_AUDIO_H

#include "gbcc.h"
#include <AL/al.h>
#include <AL/alc.h>

#define GBCC_AUDIO_BUFSIZE (1024*2) /* samples * channels */
#define GBCC_AUDIO_FMT uint16_t

struct gbcc_audio {
	struct gbc *gbc;
	struct {
		ALCdevice *device;
		ALCcontext *context;
		ALuint source;
		ALuint buffers[5];
	} al;
	uint64_t sample_clock;
	uint64_t sync_clock;
	uint64_t clock;
	size_t index;
	struct timespec cur_time;
	struct timespec start_time;
	GBCC_AUDIO_FMT mix_buffer[GBCC_AUDIO_BUFSIZE];
};

void gbcc_audio_initialise(struct gbcc_audio *audio, struct gbc *gbc);
void gbcc_audio_destroy(struct gbcc_audio *audio);
void gbcc_audio_update(struct gbcc_audio *audio);
int gbcc_check_openal_error(const char *msg);

#endif /* GBCC_AUDIO_H */
