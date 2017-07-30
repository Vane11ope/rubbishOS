#include "app.h"

void RubbMain(void)
{
	char *buf;
	int window, i, x, y;

	api_init_malloc();
	buf = api_malloc(150 * 100);
	window = api_open_window(buf, 150, 100, -1, "dots");
	api_boxfill_on_window(window, 6, 26, 143, 93, 0);
	for (i = 0; i < 50; ++i) {
		x = (rand() % 137) + 6;
		y = (rand() % 67) + 26;
		api_dot(window, x, y, 3);
	}
	api_end();
}


