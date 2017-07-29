void api_init_malloc(void);
char *api_malloc(int size);
int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_refresh_window(int win, int x0, int y0, int x1, int y1);
void api_drawline(int window, int x0, int y0, int x1, int y1, int color);
void api_close_window(int window);
void api_end(void);

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
	api_close_window(window);
	api_end();
}

