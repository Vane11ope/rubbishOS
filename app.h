#include <stdio.h>
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
int api_cmdline(char *buf, int maxsize);
int api_fread(char *buf, int maxsize, int fhandle);
int api_fsize(int fhandle, int mode);
int api_fseek(int fhandle, int offset, int mode);
int api_fclose(int fhandle);
int api_fopen(char *fname);
int api_getlang(void);
int rand(void);

// timer
int api_alloc_timer(void);
void api_init_timer(int timer, int data);
void api_set_timer(int timer, int time);
void api_free_timer(int timer);

// sound
void api_beep(int tone);

/* utility */
static inline int max(int a, int b) { return a >= b ? a : b; }
static inline int min(int a, int b) { return a <= b ? a : b; }
static inline int length(unsigned char* s) {
	int len = 0;
	for (; *s != 0x00; ++s) {
		++len;
	}
	return len;
}
