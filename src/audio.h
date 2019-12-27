#ifndef GBCC_AUDIO_H
#define GBCC_AUDIO_H

#include <AL/al.h>
#include <AL/alc.h>
#include <stdint.h>
#include <time.h>

#define GBCC_AUDIO_BUFSIZE (1024*2) /* samples * channels */
#define GBCC_AUDIO_FMT uint16_t

struct gbcc;

struct gbcc_audio {
	struct {
		ALCdevice *device;
		ALCcontext *context;
		ALuint source;
		ALuint buffers[5];
	} al;
	uint64_t sample_clock;
	uint64_t clock;
	size_t index;
	struct timespec cur_time;
	struct timespec start_time;
	GBCC_AUDIO_FMT mix_buffer[GBCC_AUDIO_BUFSIZE];
};

void gbcc_audio_initialise(struct gbcc *gbc);
void gbcc_audio_destroy(struct gbcc *gbc);
void gbcc_audio_update(struct gbcc *gbc);
int gbcc_check_openal_error(const char *msg);

#endif /* GBCC_AUDIO_H */
