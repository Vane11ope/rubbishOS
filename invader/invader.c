#include <stdio.h>
#include <string.h>
#include "../app.h"

void putstr(int window, char *windowbuf, int x, int y, int color, unsigned char *s);
void wait(int i, int timer, char *keyflag);

static unsigned char charset[16 * 8] = {

	/* invader(0) */
	0x00, 0x00, 0x00, 0x43, 0x5f, 0x5f, 0x5f, 0x7f,
	0x1f, 0x1f, 0x1f, 0x1f, 0x00, 0x20, 0x3f, 0x00,

	/* invader(1) */
	0x00, 0x0f, 0x7f, 0xff, 0xcf, 0xcf, 0xcf, 0xff,
	0xff, 0xe0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0x00,

	/* invader(2) */
	0x00, 0xf0, 0xfe, 0xff, 0xf3, 0xf3, 0xf3, 0xff,
	0xff, 0x07, 0xff, 0xff, 0x03, 0x03, 0x03, 0x00,

	/* invader(3) */
	0x00, 0x00, 0x00, 0xc2, 0xfa, 0xfa, 0xfa, 0xfe,
	0xf8, 0xf8, 0xf8, 0xf8, 0x00, 0x04, 0xfc, 0x00,

	/* fighter(0) */
	0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x43, 0x47, 0x4f, 0x5f, 0x7f, 0x7f, 0x00,

	/* fighter(1) */
	0x18, 0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xff,
	0xff, 0xff, 0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0x00,

	/* fighter(2) */
	0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0xc2, 0xe2, 0xf2, 0xfa, 0xfe, 0xfe, 0x00,

	/* laser */
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00
};

void RubbMain(void)
{
	int window, timer, i, j, fx, laserwait, lx = 0, ly;
	int ix, iy, movewait0, movewait, idir;
	int invisibleline, score, high, point;
	char windowbuf[336 * 261], invisiblestr[32 * 6], s[12], keyflag[4], *p;
	static char invisiblestr0[32] = " abcd abcd abcd abcd abcd ";

	window = api_open_window(windowbuf, 336, 261, -1, "invader");
	api_boxfill_on_window(window, 6, 27, 329, 254, 0);
	timer = api_alloc_timer();
	api_init_timer(timer, 128);

	high = 0;
	putstr(window, windowbuf, 22, 0, 7, "HIGH:00000000");

restart:
	score = 0;
	point = 1;
	putstr(window, windowbuf, 4, 0, 7, "SCORE:00000000");
	movewait0 = 20;
	fx = 18;
	putstr(window, windowbuf, fx, 13, 6, "efg");
	wait(100, timer, keyflag);

next_group:
	wait(100, timer, keyflag);
	ix = 7;
	iy = 1;
	invisibleline = 6;
	for (i = 0; i < 6; ++i) {
		for (j = 0; j < 27; ++j) {
			invisiblestr[i * 32 + j] = invisiblestr0[j];
		}
		putstr(window, windowbuf, ix, iy + i, 2, invisiblestr + i * 32);
	}
	keyflag[0] = 0;
	keyflag[1] = 0;
	keyflag[2] = 0;

	ly = 0;
	laserwait = 0;
	movewait = movewait0;
	idir = 1;
	wait(100, timer, keyflag);

	for (;;) {
		if (laserwait != 0) {
			--laserwait;
			keyflag[2] = 0;
		}
		wait(4, timer, keyflag);
		if (keyflag[0] != 0 && fx > 0) {
			--fx;
			putstr(window, windowbuf, fx, 13, 6, "efg ");
			keyflag[0] = 0;
		}
		if (keyflag[1] != 0 && fx < 37) {
			putstr(window, windowbuf, fx, 13, 6, " efg");
			++fx;
			keyflag[1] = 0;
		}
		if (keyflag[2] != 0 && laserwait == 0) {
			laserwait = 15;
			lx = fx + 1;
			ly = 13;
		}
		if (movewait != 0) {
			--movewait;
		} else {
			movewait = movewait0;
			if (ix + idir > 14 || ix + idir < 0) {
				if (iy + invisibleline == 13) { break; }
				idir = -idir;
				putstr(window, windowbuf, ix + 1, iy, 0, "              ");
				++iy;
			} else {
				ix += idir;
			}
			for (i = 0; i < invisibleline; ++i) {
				putstr(window, windowbuf, ix, iy + i, 2, invisiblestr + i * 32);
			}
		}
		if (ly > 0) {
			if (ly < 13) {
				if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invisibleline) {
					putstr(window, windowbuf, ix, ly, 2, invisiblestr + (ly - iy) * 32);
				} else {
					putstr(window, windowbuf, lx, ly, 0, " ");
				}
				--ly;
				if (ly > 0) {
					putstr(window, windowbuf, lx, ly, 3, "h");
				} else {
					point -= 10;
					if (point <= 0) { point = 1; }
				}
			}
			if (ix < lx && lx < ix + 25 && iy <= ly && ly < iy + invisibleline) {
				p = invisiblestr + (ly - iy) * 32 + (lx - ix);
				if (*p != ' ') {
					score += point;
					++point;
					sprintf(s, "%08d", score);
					putstr(window, windowbuf, 10, 0, 7, s);
					if (high < score) {
						high = score;
						putstr(window, windowbuf, 27, 0, 7, s);
					}
					for (--p; *p != ' '; --p) { }
					for (i = 1; i < 5; ++i) { p[i] = ' '; }
					putstr(window, windowbuf, ix, ly, 2, invisiblestr + (ly - iy) * 32);
					for (; invisibleline > 0; --invisibleline) {
						for (p = invisiblestr + (invisibleline - 1) * 32; *p != 0; ++p) {
							if (*p != ' ') { goto alive; }
						}
					}
					movewait0 -= movewait0 / 3;
					goto next_group;
alive:
					ly = 0;
				}
			}
		}
	}
	putstr(window, windowbuf, 15, 6, 1, "GAME OVER");
	wait(0, timer, keyflag);
	for (i = 1; i < 14; ++i) {
		putstr(window, windowbuf, 0, i, 0, "              ");
	}
	goto restart;
}

void putstr(int window, char *windowbuf, int x, int y, int color, unsigned char *s)
{
	int c, x0, i;
	char *p, *q, t[2];
	x = x * 8 + 8;
	y = y * 16 + 29;
	x0 = x;
	i = strlen(s);
	api_boxfill_on_window(window + 1, x, y, x + i * 8 - 1, y + 15, 0);
	q = windowbuf + y * 336;
	t[1] = 0;
	for (;;) {
		c = *s;
		if (c == 0) { break; }
		if (c != ' ') {
			if ('a' <= c && c <= 'h') {
				p = charset + 16 * (c - 'a');
				q += x;
				for (i = 0; i < 16; ++i) {
					if ((p[i] & 0x80) != 0) { q[0] = color; }
					if ((p[i] & 0x40) != 0) { q[1] = color; }
					if ((p[i] & 0x20) != 0) { q[2] = color; }
					if ((p[i] & 0x10) != 0) { q[3] = color; }
					if ((p[i] & 0x08) != 0) { q[4] = color; }
					if ((p[i] & 0x04) != 0) { q[5] = color; }
					if ((p[i] & 0x02) != 0) { q[6] = color; }
					if ((p[i] & 0x01) != 0) { q[7] = color; }
					q += 336;
				}
				q -= 336 * 16 + x;
			} else {
				t[0] = *s;
				api_putstr_on_window(window + 1, x, y, color, 1, t);
			}
		}
		++s;
		x += 8;
	}
	api_refresh_window(window, x0, y, x, y + 16);
	return;
}

void wait(int i, int timer, char *keyflag)
{
	int j;
	if (i > 0) {
		api_set_timer(timer, i);
		i = 128;
	} else {
		i = 0x0a;
	}
	for (;;) {
		j = api_getkey(1);
		if (i == j) { break; }
		if (j == '4') { keyflag[0] = 1; }
		if (j == '6') { keyflag[1] = 1; }
		if (j == ' ') { keyflag[2] = 1; }
	}
	return;
}
