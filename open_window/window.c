#include "../app.h"

char buf[150 * 50];

void RubbMain(void)
{
	int window;

	window = api_open_window(buf, 150, 50, -1, "fuck you");
	for (;;) {
		if (api_getkey(1) == 0x0a) { break; }
	}
	api_end();
}
