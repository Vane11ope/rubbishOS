int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_end(void);

char buf[150 * 50];

void RubbMain(void)
{
	int window;
	window = api_open_window(buf, 150, 50, -1, "fuck you");
	api_end();
}
