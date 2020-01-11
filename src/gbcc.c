#include "gbcc.h"
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
			gbcc_audio_update(gbc);
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
