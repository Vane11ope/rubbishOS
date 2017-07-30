void api_init_malloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_putstr_on_window(int win, int x, int y, int color, int len, char *str);
void api_boxfill_on_window(int win, int x0, int y0, int x1, int y1, int opacity);
void api_end(void);

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
