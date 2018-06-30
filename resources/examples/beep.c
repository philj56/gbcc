#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <math.h>
#include <stdio.h>

const int AMPLITUDE = 32767 / 4;
const int SAMPLE_RATE = 48000;

void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes)
{
	Sint16 *buffer = (Sint16*)raw_buffer;
	int length = bytes / sizeof(Sint16);
	unsigned int time;
	int sample = *(int *)user_data;

	for(size_t i = 0; i < length; i++, sample++)
	{
		time = lround(2 * sample * 441.0 / SAMPLE_RATE);
		buffer[i] = (Sint16)(AMPLITUDE*(time & 1u)); // render 441 HZ square wave
		printf("%d", buffer[i] / AMPLITUDE);
	}
	printf("\n");
}

int main()
{
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		printf("Failed to initialize SDL: %s", SDL_GetError());
	}

	int sample = 0;
	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE; // number of samples per second
	want.format = AUDIO_S16SYS; // sample type (here: unsigned short i.e. 16 bit)
	want.channels = 1; // only one channel
	want.samples = 4800; // buffer-size
	want.callback = audio_callback; // function SDL calls periodically to refill the buffer
	want.userdata = &sample;

	SDL_AudioSpec have;
	if (SDL_OpenAudio(&want, &have) != 0) {
		printf("Failed to open audio: %s", SDL_GetError());
	}
	if (want.format != have.format) {
		printf("Failed to get the desired AudioSpec");
	}
	printf("%d\n", have.freq);

	SDL_PauseAudio(0); // start playing sound
	SDL_Delay(2000); // wait while sound is playing
	SDL_PauseAudio(1); // stop playing sound

	SDL_CloseAudio();

	return 0;
}
