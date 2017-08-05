#include "app.h"

void RubbMain(void)
{
	int i, timer;
	timer = api_alloc_timer();
	api_init_timer(timer, 128);
	for (i = 20000000; i >= 20000; i -= i / 100) {
		api_beep(i);
		api_set_timer(timer, 1);
		if (api_getkey(1) != 128) { break; }
	}
	api_beep(0);
	api_end();
}
