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
#include "debug.h"
#include "camera.h"
#include "save.h"

void *gbcc_emulation_loop(void *_gbc)
{
	struct gbcc *gbc = (struct gbcc *)_gbc;
	gbcc_load(gbc);
	while (!gbc->quit) {
		for (int i = 1000; i > 0; i--) {
			/* Only check for savestates, pause etc.
			 * every 1000 cycles */
			gbcc_emulate_cycle(&gbc->core);
			if (gbc->core.error) {
				gbcc_log_error("Invalid opcode: 0x%02X\n", gbc->core.cpu.opcode);
				gbcc_print_registers(&gbc->core, false);
				gbc->quit = true;
				return 0;
			}
			gbcc_audio_update(gbc);
			gbcc_camera_clock(gbc);
		}
		if (gbc->load_state > 0) {
			gbcc_load_state(gbc);
		} else if (gbc->save_state > 0) {
			gbcc_save_state(gbc);
		}
		if (gbc->autosave && gbc->core.cart.mbc.sram_changed) {
			if (time(NULL) > gbc->core.cart.mbc.last_save_time) {
				gbcc_save(gbc);
				gbc->core.cart.mbc.sram_changed = false;
			}
		}
		while (gbc->pause || gbc->menu.show || !(gbc->has_focus || gbc->background_play)) {
			const struct timespec time = {.tv_sec = 0, .tv_nsec = 10000000};
			nanosleep(&time, NULL);
			if (gbc->quit) {
				break;
			}
		}
	}
	gbcc_save(gbc);
	return 0;
}
