#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>

const int AMPLITUDE = 32767 / 4;
const int SAMPLE_RATE = 48000;
const int BUFSIZE = 256;
const int note = -1;
const int delay = 50;

void beep(int freq, int len, Sint16 *buffer, size_t bufsize);

int main()
{
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		printf("Failed to initialize SDL: %s", SDL_GetError());
	}
	SDL_AudioSpec want;
	want.freq = SAMPLE_RATE; // number of samples per second
	want.format = AUDIO_S16SYS; // sample type (here: unsigned short i.e. 16 bit)
	want.channels = 1; // only one channel
	want.samples = 4096; // buffer-size
	want.callback = NULL; // function SDL calls periodically to refill the buffer
	want.userdata = NULL;

	SDL_AudioSpec have;
	if (SDL_OpenAudio(&want, &have) != 0) {
		printf("Failed to open audio: %s", SDL_GetError());
	}
	if (want.format != have.format) {
		printf("Failed to get the desired AudioSpec");
	}

	Sint16 buffer[BUFSIZE];
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_PauseAudio(0);
	for (int note = 0; note < 18; note++) {
		for (size_t i = 0; i < 1; i++) {
			beep(262*pow(1.066, note), delay, buffer, BUFSIZE);
			beep(330*pow(1.066, note), delay, buffer, BUFSIZE);
			beep(392*pow(1.066, note), delay, buffer, BUFSIZE);
			beep(523*pow(1.066, note), delay, buffer, BUFSIZE);
			beep(392*pow(1.066, note), delay, buffer, BUFSIZE);
			beep(330*pow(1.066, note), delay, buffer, BUFSIZE);
		}
		beep(262*pow(1.066, note), 2*delay, buffer, BUFSIZE);
	}
	while (SDL_GetQueuedAudioSize(1)) {
		SDL_Delay(1000);
	}
	SDL_PauseAudio(1);

	SDL_CloseAudio();

	return 0;
}

void beep(int freq, int len, Sint16 *buffer, size_t bufsize)
{
	static long long int sample = 0;
	int delay = 1000 * (double)bufsize / SAMPLE_RATE;
	for (size_t i = 0; i < len / delay; i++) {
		for (size_t j = 0; j < bufsize / 2; j++, sample++) {
			unsigned int time = lround(2 * sample * (double)freq / SAMPLE_RATE);
			buffer[j] = (Sint16)(AMPLITUDE * (time & 1u));
		}
		SDL_QueueAudio(1, buffer, bufsize);
	}
	//SDL_Delay(delay);
	//SDL_Delay(delay);
}
