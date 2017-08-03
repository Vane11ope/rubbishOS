#include "app.h"

void RubbMain(void)
{
	char *buf, s[12];
	int window, timer, sec = 0, min = 0, hour = 0;
	api_init_malloc();
	buf = api_malloc(150 * 50);
	window = api_open_window(buf, 150, 50, -1, "noodle");
	timer = api_alloc_timer();
	api_init_timer(timer, 128);
	for (;;) {
		sprintf(s, "%5d:%02d:%02d", hour, min, sec);
		api_boxfill_on_window(window, 28, 27, 115, 41, 7);
		api_putstr_on_window(window, 28, 27, 0, 11, s);
		api_set_timer(timer, 100);
		if (api_getkey(1) != 128) { break; }
		++sec;
		if (sec == 60) {
			sec = 0;
			++min;
			if (min == 60) {
				min = 0;
				++hour;
			}
		}
	}
	api_end();
}
