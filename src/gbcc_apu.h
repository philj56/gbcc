#ifndef GBCC_APU_H
#define GBCC_APU_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

struct gbc;

struct timer {
	uint16_t period;
	uint16_t counter;
};

struct duty {
	struct timer freq_timer;
	struct timer duty_timer;
	uint8_t cycle;
	uint16_t freq;
};

struct sweep {
	uint16_t freq;
	struct timer timer;
	uint8_t shift;
	int8_t dir;
	bool enabled;
};

struct noise {
	uint8_t shift;
	bool width_mode;
	struct timer timer;
	uint16_t lfsr;
};

struct wave {
	struct timer timer;
	uint16_t addr;
	uint8_t buffer;
	uint8_t nibble;
	uint8_t shift;
	uint16_t freq;
};

struct envelope {
	struct timer timer;
	uint8_t start_volume;
	uint8_t volume;
	int8_t dir;
	bool enabled;
};

struct channel {
	uint16_t counter;
	bool length_enable;
	bool state;
	bool enabled;
	bool left;
	bool right;
	struct envelope envelope;
	struct duty duty;
};

struct apu {
	uint64_t clock;
	uint64_t sample_clock;
	uint64_t sample;
	size_t index;
	struct timespec cur_time;
	struct timespec start_time;
	struct channel ch1; 	/* Tone & Sweep */
	struct channel ch2; 	/* Tone */
	struct channel ch3; 	/* Wave Output */
	struct channel ch4; 	/* Noise */
	struct sweep sweep;
	struct noise noise;
	struct wave wave;
	struct timer sequencer_timer;
};

void timer_reset(struct timer *timer);
void gbcc_apu_init(struct gbc *gbc);
void gbcc_apu_clock(struct gbc *gbc);
void gbcc_apu_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val);

#endif /* GBCC_AUDIO_H */
