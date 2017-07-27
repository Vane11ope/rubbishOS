void api_init_malloc(void);
char *api_malloc(int size);
int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_boxfill_on_window(int win, int x0, int y0, int x1, int y1, int opacity);
void api_dot(int window, int x, int y, int opacity);
void api_end(void);

void RubbMain(void)
{
	char *buf;
	int window;

	api_init_malloc();
	buf = api_malloc(150 * 100);
	window = api_open_window(buf, 150, 100, -1, "fuck you");
	api_boxfill_on_window(window, 6, 26, 143, 93, 0);
	api_dot(window, 75, 59, 3);
	api_end();
}

