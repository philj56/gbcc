#include "gbcc.h"
#include "gbcc_apu.h"
#include "gbcc_bit_utils.h"
#include "gbcc_debug.h"
#include "gbcc_memory.h"
#include "gbcc_time.h"
#include <stdint.h>
#include <time.h>

#define SAMPLE_RATE 48000
#define CLOCKS_PER_SAMPLE (GBC_CLOCK_FREQ / SAMPLE_RATE)
#define SLEEP_TIME (SECOND / SAMPLE_RATE)
#define SEQUENCER_CLOCKS 8192
#define TIMER_RESTART (3600 * SAMPLE_RATE) /* restart after x samples */
#define SLEEP_DETECT SECOND

static const uint8_t duty_table[4] = {
	0x01, 	/* 00000001b */
	0x81, 	/* 10000001b */
	0x87, 	/* 10000111b */
	0x7E 	/* 01111110b */
};

static bool timer_clock(struct timer *timer);
static bool duty_clock(struct duty *duty);
static void envelope_clock(struct envelope *envelope);
static void sequencer_clock(struct gbc *gbc);
static void time_sync(struct gbc *gbc);
static void ch1_trigger(struct gbc *gbc);
static void ch2_trigger(struct gbc *gbc);
static void ch3_trigger(struct gbc *gbc);
static void ch4_trigger(struct gbc *gbc);

void gbcc_apu_init(struct gbc *gbc)
{
	gbc->apu.sample_clock = 0;
	gbc->apu.sample = 0;
	gbc->apu.index = 0;
	gbc->apu.start_time.tv_sec = 0;
	gbc->apu.start_time.tv_nsec = 0;
	gbc->apu.sequencer_timer.period = 8;
	timer_reset(&gbc->apu.sequencer_timer);
	gbc->apu.ch1.duty.duty_timer.period = 8;
	gbc->apu.ch2.duty.duty_timer.period = 8;
	timer_reset(&gbc->apu.ch1.duty.duty_timer);
	timer_reset(&gbc->apu.ch2.duty.duty_timer);
	gbc->apu.ch1.envelope.timer.period = 8;
	gbc->apu.ch2.envelope.timer.period = 8;
	gbc->apu.ch4.envelope.timer.period = 8;
	timer_reset(&gbc->apu.ch1.envelope.timer);
	timer_reset(&gbc->apu.ch2.envelope.timer);
	timer_reset(&gbc->apu.ch4.envelope.timer);
	gbc->apu.ch1.length_enable = false;
	gbc->apu.ch2.length_enable = false;
	gbc->apu.ch3.length_enable = false;
	gbc->apu.ch4.length_enable = false;
	gbc->apu.sweep.enabled = false;
	gbc->apu.sweep.timer.period = 8;
	timer_reset(&gbc->apu.sweep.timer);
	gbc->apu.noise.timer.period = 8;
	timer_reset(&gbc->apu.noise.timer);
	gbc->apu.wave.timer.period = 8;
	timer_reset(&gbc->apu.wave.timer);
	gbc->apu.wave.addr = WAVE_START;
	gbc->apu.noise.width_mode = false;
	gbc->apu.left_vol = 0;
	gbc->apu.right_vol = 0;
	gbc->apu.ch1.enabled = false;
	gbc->apu.ch2.enabled = false;
	gbc->apu.ch3.enabled = false;
	gbc->apu.ch4.enabled = false;
}

void gbcc_apu_clock(struct gbc *gbc)
{
	gbc->apu.clock += 4;
	if (gbc->apu.start_time.tv_sec == 0) {
		clock_gettime(CLOCK_REALTIME, &gbc->apu.start_time);
	}

	/* Duty */
	gbc->apu.ch1.state = duty_clock(&gbc->apu.ch1.duty);
	gbc->apu.ch2.state = duty_clock(&gbc->apu.ch2.duty);

	/* Noise */
	if (gbc->apu.noise.shift < 14 && timer_clock(&gbc->apu.noise.timer)) {
		uint8_t lfsr_low = gbc->apu.noise.lfsr & 0xFFu;
		uint8_t tmp = check_bit(lfsr_low, 0) ^ check_bit(lfsr_low, 1);
		gbc->apu.noise.lfsr >>= 1u;
		gbc->apu.noise.lfsr |= tmp * (1u << 14u);
		if (gbc->apu.noise.width_mode) {
			gbc->apu.noise.lfsr |= tmp * bit(6);
		}
		gbc->apu.ch4.state = !(gbc->apu.noise.lfsr & bit(0));
	}

	/* Wave */
	if (timer_clock(&gbc->apu.wave.timer)) {
		gbc->apu.wave.addr += gbc->apu.wave.nibble;
		gbc->apu.wave.nibble = !gbc->apu.wave.nibble;
		if (gbc->apu.wave.addr == WAVE_END) {
			gbc->apu.wave.addr = WAVE_START;
		}
		gbc->apu.wave.buffer = gbcc_memory_read(gbc, gbc->apu.wave.addr, true);
		gbc->apu.wave.buffer &= 0x0Fu << (4u * gbc->apu.wave.nibble);
		gbc->apu.wave.buffer >>= 4u * gbc->apu.wave.nibble;
	}

	uint64_t clocks = gbc->apu.clock - gbc->apu.sample_clock;
	if (!(gbc->apu.clock % SEQUENCER_CLOCKS)) {
		sequencer_clock(gbc);
	}
	if (clocks >= CLOCKS_PER_SAMPLE) {
		if (gbc->keys.turbo) {
			gbc->apu.start_time.tv_sec = 0;
			gbc->apu.sample = 0;
		} else {
			time_sync(gbc);
		}
		gbc->apu.sample_clock = gbc->apu.clock;
		gbc->apu.sample++;
	}
}

bool timer_clock(struct timer *timer)
{
	if (--timer->counter == 0) {
		timer_reset(timer);
		return true;
	}
	return false;
}

void timer_reset(struct timer *timer)
{
	timer->counter = timer->period;
}

bool duty_clock(struct duty *duty)
{
	duty->freq_timer.period = 2048u - duty->freq;
	if (timer_clock(&duty->freq_timer)) {
		timer_clock(&duty->duty_timer);
	}
	return duty_table[duty->cycle] & bit((uint8_t)(8u-duty->duty_timer.counter));
}

void envelope_clock(struct envelope *envelope)
{
	if (!envelope->enabled) {
		return;
	}
	if (envelope->timer.period == 0) {
		envelope->timer.period = 8;
	}
	if (timer_clock(&envelope->timer)) {
		envelope->volume += envelope->dir;
		if (envelope->volume == 0x10u || envelope->volume == 0xFFu) {
			envelope->volume -= envelope->dir;
			envelope->enabled = false;
		}
	}
}

void sequencer_clock(struct gbc *gbc)
{
	/* DAC */
	if (!(gbcc_memory_read(gbc, NR12, true) & 0xF7u)) {
		gbc->apu.ch1.enabled = false;
	}
	if (!(gbcc_memory_read(gbc, NR22, true) & 0xF7u)) {
		gbc->apu.ch2.enabled = false;
	}
	if (!(gbcc_memory_read(gbc, NR30, true) & 0x80u)) {
		gbc->apu.ch3.enabled = false;
	}
	if (!(gbcc_memory_read(gbc, NR42, true) & 0xF7u)) {
		gbc->apu.ch4.enabled = false;
	}

	/* Length counters every other clock */
	if (!(gbc->apu.sequencer_timer.counter & 0x01u)) {
		if (gbc->apu.ch1.length_enable) {
			gbc->apu.ch1.counter--;
			if (gbc->apu.ch1.counter == 0) {
				gbc->apu.ch1.enabled = false;
			}
		}
		if (gbc->apu.ch2.length_enable) {
			gbc->apu.ch2.counter--;
			if (gbc->apu.ch2.counter == 0) {
				gbc->apu.ch2.enabled = false;
			}
		}
		if (gbc->apu.ch3.length_enable) {
			gbc->apu.ch3.counter--;
			if (gbc->apu.ch3.counter == 0) {
				gbc->apu.ch3.enabled = false;
			}
		}
		if (gbc->apu.ch4.length_enable) {
			gbc->apu.ch4.counter--;
			if (gbc->apu.ch4.counter == 0) {
				gbc->apu.ch4.enabled = false;
			}
		}
	}

	/* Sweep on clocks 2 & 6 */
	if (gbc->apu.sweep.enabled && (gbc->apu.sequencer_timer.counter == 2u || gbc->apu.sequencer_timer.counter == 6u)) {
		if (gbc->apu.sweep.timer.period == 0) {
			gbc->apu.sweep.timer.period = 8;
		}
		if (timer_clock(&gbc->apu.sweep.timer)) {
			uint16_t freq = gbc->apu.sweep.freq >> gbc->apu.sweep.shift;
			freq = (uint16_t)(gbc->apu.sweep.freq + gbc->apu.sweep.dir * freq);
			if (gbc->apu.sweep.shift != 0 && freq < 2048) {
				gbc->apu.sweep.freq = freq;
				gbc->apu.ch1.duty.freq = gbc->apu.sweep.freq;
			}
		}
	}

	/* Envelope */
	if (gbc->apu.sequencer_timer.counter == 7u) {
		envelope_clock(&gbc->apu.ch1.envelope);
		envelope_clock(&gbc->apu.ch2.envelope);
		envelope_clock(&gbc->apu.ch4.envelope);
	}

	timer_clock(&gbc->apu.sequencer_timer);
}

void time_sync(struct gbc *gbc)
{
	clock_gettime(CLOCK_REALTIME, &gbc->apu.cur_time);
	uint64_t diff = gbcc_time_diff(&gbc->apu.cur_time, &gbc->apu.start_time);
	if (diff > SLEEP_DETECT + (SECOND * gbc->apu.sample) / SAMPLE_RATE) {
		const struct timespec time = {.tv_sec = 2, .tv_nsec = 0};
		nanosleep(&time, NULL);
		gbc->apu.start_time.tv_sec = 0;
		gbc->apu.sample = 0;
		gbc->apu.index = 0;
		return;
	}
	while (diff < (SECOND * gbc->apu.sample) / SAMPLE_RATE) {
		const struct timespec time = {.tv_sec = 0, .tv_nsec = SLEEP_TIME};
		nanosleep(&time, NULL);
		clock_gettime(CLOCK_REALTIME, &gbc->apu.cur_time);
		diff = gbcc_time_diff(&gbc->apu.cur_time, &gbc->apu.start_time);
	}
	if (!(gbc->apu.sample % TIMER_RESTART)) {
		gbc->apu.start_time = gbc->apu.cur_time;
		gbc->apu.sample = 0;
	}
}

void gbcc_apu_memory_write(struct gbc *gbc, uint16_t addr, uint8_t val)
{
	switch (addr) {
		case NR10:
			gbc->apu.sweep.timer.period = (val & 0x70u) >> 4u;
			gbc->apu.sweep.dir = check_bit(val, 3u) ? -1 : 1;
			gbc->apu.sweep.shift = val & 0x07u;
			break;
		case NR11:
			gbc->apu.ch1.duty.cycle = (val & 0xC0u) >> 6u;
			gbc->apu.ch1.counter = 64 - (val & 0x3Fu);
			break;
		case NR12:
			gbc->apu.ch1.envelope.start_volume = (val & 0xF0u) >> 4u;
			gbc->apu.ch1.envelope.dir = val & 0x08u ? 1 : -1;
			gbc->apu.ch1.envelope.timer.period = val & 0x07u;
			break;
		case NR13:
			gbc->apu.ch1.duty.freq &= ~0x00FFu;
			gbc->apu.ch1.duty.freq |= val;
			break;
		case NR14:
			gbc->apu.ch1.length_enable = check_bit(val, 6);
			gbc->apu.ch1.duty.freq &= ~0xFF00u;
			gbc->apu.ch1.duty.freq |= (val & 0x07u) << 8u;
			if (check_bit(val, 7)) {
				ch1_trigger(gbc);
			}
			break;
		case NR21:
			gbc->apu.ch2.duty.cycle = (val & 0xC0u) >> 6u;
			gbc->apu.ch2.counter = 64 - (val & 0x3Fu);
			break;
		case NR22:
			gbc->apu.ch2.envelope.start_volume = (val & 0xF0u) >> 4u;
			gbc->apu.ch2.envelope.dir = val & 0x08u ? 1 : -1;
			gbc->apu.ch2.envelope.timer.period = val & 0x07u;
			break;
		case NR23:
			gbc->apu.ch2.duty.freq &= ~0x00FFu;
			gbc->apu.ch2.duty.freq |= val;
			break;
		case NR24:
			gbc->apu.ch2.length_enable = check_bit(val, 6);
			gbc->apu.ch2.duty.freq &= ~0xFF00u;
			gbc->apu.ch2.duty.freq |= (val & 0x07u) << 8u;
			if (check_bit(val, 7)) {
				ch2_trigger(gbc);
			}
			break;
		case NR31:
			gbc->apu.ch3.counter = 256 - val;
			if (gbc->apu.ch3.counter == 0) {
				gbc->apu.ch3.counter = 256;
			}
			break;
		case NR32:
			gbc->apu.wave.shift = (val & 0x60u) >> 5u;
			break;
		case NR33:
			gbc->apu.wave.freq &= ~0x00FFu;
			gbc->apu.wave.freq |= val;
			//gbc->apu.wave.timer.period = (2048u - gbc->apu.wave.freq) / 2;
			break;
		case NR34:
			gbc->apu.ch3.length_enable = check_bit(val, 6);
			gbc->apu.wave.freq &= ~0xFF00u;
			gbc->apu.wave.freq |= (val & 0x07u) << 8u;
			//gbc->apu.wave.timer.period = (2048u - gbc->apu.wave.freq) / 2;
			if (check_bit(val, 7)) {
				ch3_trigger(gbc);
			}
			break;
		case NR41:
			gbc->apu.ch4.counter = 64 - (val & 0x3Fu);
			break;
		case NR42:
			gbc->apu.ch4.envelope.start_volume = (val & 0xF0u) >> 4u;
			gbc->apu.ch4.envelope.dir = val & 0x08u ? 1 : -1;
			gbc->apu.ch4.envelope.timer.period = val & 0x07u;
			break;
		case NR43:
			gbc->apu.noise.shift = (val & 0xF0u) >> 4u;
			gbc->apu.noise.width_mode = check_bit(val, 3);
			gbc->apu.noise.timer.period = (uint16_t)((val & 0x07u) << 4u);
			if (gbc->apu.noise.timer.period == 0) {
				gbc->apu.noise.timer.period = 0x08u;
			}
			gbc->apu.noise.timer.period <<= gbc->apu.noise.shift;
		//	timer_reset(&gbc->apu.noise.timer);
			break;
		case NR44:
			gbc->apu.ch4.length_enable = check_bit(val, 6);
			if (check_bit(val, 7)) {
				ch4_trigger(gbc);
			}
			break;
		case NR50:
			gbc->apu.left_vol = (val & 0x70u) >> 4u;
			gbc->apu.right_vol = val & 0x07u;
			break;
		case NR51:
			gbc->apu.ch4.left = check_bit(val, 7);
			gbc->apu.ch3.left = check_bit(val, 6);
			gbc->apu.ch2.left = check_bit(val, 5);
			gbc->apu.ch1.left = check_bit(val, 4);
			gbc->apu.ch4.right = check_bit(val, 3);
			gbc->apu.ch3.right = check_bit(val, 2);
			gbc->apu.ch2.right = check_bit(val, 1);
			gbc->apu.ch1.right = check_bit(val, 0);
			break;
	}
}

void ch1_trigger(struct gbc *gbc)
{
	uint8_t nr11 = gbcc_memory_read(gbc, NR11, true);
	uint8_t nr13 = gbcc_memory_read(gbc, NR13, true);
	uint8_t nr14 = gbcc_memory_read(gbc, NR14, true);
	gbc->apu.ch1.duty.cycle = (nr11 & 0xC0u) >> 6u;
	gbc->apu.ch1.duty.freq = nr13;
	gbc->apu.ch1.duty.freq |= (nr14 & 0x07u) << 8u;
	gbc->apu.ch1.enabled = true;
	gbc->apu.ch1.counter = 64 - (nr11 & 0x3Fu);
	if (gbc->apu.ch1.counter == 0) {
		gbc->apu.ch1.counter = 64;
	}
	timer_reset(&gbc->apu.ch1.duty.freq_timer);
	gbc->apu.ch1.envelope.enabled = true;
	gbc->apu.ch1.envelope.volume = gbc->apu.ch1.envelope.start_volume;
	timer_reset(&gbc->apu.ch1.envelope.timer);
	gbc->apu.sweep.freq = gbc->apu.ch1.duty.freq;
	timer_reset(&gbc->apu.sweep.timer);
	if (gbc->apu.sweep.shift == 0 && gbc->apu.sweep.timer.period == 0) {
		gbc->apu.sweep.enabled = false;
	} else {
		gbc->apu.sweep.enabled = true;
	}
}

void ch2_trigger(struct gbc *gbc)
{
	uint8_t nr21 = gbcc_memory_read(gbc, NR21, true);
	uint8_t nr23 = gbcc_memory_read(gbc, NR23, true);
	uint8_t nr24 = gbcc_memory_read(gbc, NR24, true);
	gbc->apu.ch2.duty.cycle = (nr21 & 0xC0u) >> 6u;
	gbc->apu.ch2.duty.freq = nr23;
	gbc->apu.ch2.duty.freq |= (nr24 & 0x07u) << 8u;
	gbc->apu.ch2.enabled = true;
	if ((nr21 & 0x3Fu) == 0) {
		gbc->apu.ch2.counter = 64;
	} else {
		gbc->apu.ch2.counter = 64 - (nr21 & 0x3Fu);
	}
	timer_reset(&gbc->apu.ch2.duty.freq_timer);
	gbc->apu.ch2.envelope.enabled = true;
	gbc->apu.ch2.envelope.volume = gbc->apu.ch2.envelope.start_volume;
	timer_reset(&gbc->apu.ch2.envelope.timer);
}

void ch3_trigger(struct gbc *gbc)
{
	gbc->apu.ch3.enabled = true;
	gbc->apu.ch3.counter = 256 - gbcc_memory_read(gbc, NR31, true);
	if (gbc->apu.ch3.counter == 0) {
		gbc->apu.ch3.counter = 256;
	}
	gbc->apu.wave.timer.period = (2048u - gbc->apu.wave.freq) / 2;
	timer_reset(&gbc->apu.wave.timer);
	gbc->apu.wave.addr = WAVE_START;
}

void ch4_trigger(struct gbc *gbc)
{
	gbc->apu.ch4.enabled = true;
	gbc->apu.ch4.counter = 64 - (gbcc_memory_read(gbc, NR41, true) & 0x3Fu);
	if (gbc->apu.ch4.counter == 0) {
		gbc->apu.ch4.counter = 64;
	}
	timer_reset(&gbc->apu.noise.timer);
	gbc->apu.ch4.envelope.enabled = true;
	gbc->apu.ch4.envelope.volume = gbc->apu.ch4.envelope.start_volume;
	timer_reset(&gbc->apu.ch4.envelope.timer);
	gbc->apu.noise.lfsr = 0xFFFFu;
}
