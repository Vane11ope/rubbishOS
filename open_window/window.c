#include "../app.h"

void RubbMain(void)
{
	int window;
	char buf[150 * 50];
	window = api_open_window(buf, 150, 50, -1, "fuck you");
	for (;;) {
		if (api_getkey(1) == 0x0a) { break; }
	}
	api_end();
}
