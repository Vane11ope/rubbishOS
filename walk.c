void api_init_malloc(void);
char *api_malloc(int size);
int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_boxfill_on_window(int win, int x0, int y0, int x1, int y1, int opacity);
void api_putstr_on_window(int win, int x, int y, int color, int len, char *str);
void api_refresh_window(int win, int x0, int y0, int x1, int y1);
//void api_close_window(int window);
int api_getkey(int mode);
void api_end(void);

void RubbMain(void)
{
	char *buf;
	int window, i, x, y;
	api_init_malloc();
	buf = api_malloc(160 * 100);
	window = api_open_window(buf, 160, 100, -1, "wolk");
	api_boxfill_on_window(window, 4, 24, 155, 95, 0);
	x = 76;
	y = 56;
	api_putstr_on_window(window, x, y, 3, 1, "*");
	for (;;) {
		i = api_getkey(1);
		api_putstr_on_window(window, x, y, 0, 1, "*");
		if (i == 'h' && x > 4) { x -= 8; }
		if (i == 'l' && x < 148) { x += 8; }
		if (i == 'k' && y > 24) { y -= 8; }
		if (i == 'j' && y < 80) { y += 8; }
		if (i == 0x0a) { break; }
		api_putstr_on_window(window, x, y, 3, 1, "*");
	}
	//api_close_window(window);
	api_end();
}

