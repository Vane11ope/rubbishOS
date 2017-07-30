#include "app.h"

void RubbMain(void)
{
	char *buf;
	int window;

	api_init_malloc();
	buf = api_malloc(150 * 50);
	window = api_open_window(buf, 150, 50, -1, "fuck you");
	api_boxfill_on_window(window, 8, 36, 141, 43, 3);
	api_putstr_on_window(window, 28, 28, 0, 12, "Hell Above");
	api_end();
}
