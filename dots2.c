void api_init_malloc(void);
char *api_malloc(int size);
int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_boxfill_on_window(int win, int x0, int y0, int x1, int y1, int opacity);
void api_dot(int window, int x, int y, int color);
void api_refresh_window(int win, int x0, int y0, int x1, int y1);
void api_end(void);
int rand(void);

void RubbMain(void)
{
	char *buf;
	int window, i, x, y;
	api_init_malloc();
	buf = api_malloc(150 * 100);
	window = api_open_window(buf, 150, 100, -1, "dots2");
	api_boxfill_on_window(window + 1, 6, 26, 143, 93, 0);
	for (i = 0; i < 50; ++i) {
		x = (rand() % 137) + 6;
		y = (rand() % 67) + 26;
		api_dot(window + 1, x, y, 3);
	}
	api_refresh_window(window, 6, 26, 144, 94);
	api_end();
}
