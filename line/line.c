#include "../app.h"

void RubbMain(void)
{
	char *buf;
	int window, i;
	api_init_malloc();
	buf = api_malloc(160 * 100);
	window = api_open_window(buf, 160, 100, -1, "line");
	for (i = 0; i < 8; ++i) {
		api_drawline(window + 1, 8, 26, 77, i * 9 + 26, i);
		api_drawline(window + 1, 88, 26, i * 9 + 88, 89, i);
	}
	api_refresh_window(window, 6, 26, 154, 90);
	for (;;) {
		if (api_getkey(1) == 0x0a) { break; }
	}
	//api_close_window(window);
	api_end();
}
