#include "app.h"

void RubbMain(void)
{
	char *buf;
	int window;

	api_init_malloc();
	buf = api_malloc(150 * 100);
	window = api_open_window(buf, 150, 100, -1, "dot");
	api_boxfill_on_window(window, 6, 26, 143, 93, 0);
	api_dot(window, 75, 59, 3);
	api_end();
}

