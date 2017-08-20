#include "../app.h"

void RubbMain(void)
{
	char *buf;
	int window, x, y, r, g, b;
	api_init_malloc();
	buf = api_malloc(144 * 164);
	window = api_open_window(buf, 144, 164, -1, "color");
	for (y = 0; y < 128; ++y) {
		for (x = 0; x < 128; ++x) {
			r = x * 2;
			g = y * 2;
			b = 0;
			buf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
		}
	}
	api_refresh_window(window, 8, 28, 136, 156);
	api_getkey(1);
	api_end();
}
