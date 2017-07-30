// memory
void api_init_malloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);

// window
int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
void api_boxfill_on_window(int win, int x0, int y0, int x1, int y1, int opacity);
void api_putstr_on_window(int win, int x, int y, int color, int len, char *str);
void api_refresh_window(int win, int x0, int y0, int x1, int y1);
void api_close_window(int window);

// key
int api_getkey(int mode);

// graphic
void api_putstr(char *s);
void api_putchar(int c);
void api_drawline(int window, int x0, int y0, int x1, int y1, int color);
void api_dot(int window, int x, int y, int color);

// system
void api_end(void);
int rand(void);
