#include "../app.h"

int strtol(char *s, char **endp, int base);

char *skipspace(char *p);
void textview(int window, int width, int height, int xskip, char *p, int tab, int lang);
char *lineview(int window, int width, int y, int xskip, unsigned char *p, int tab, int lang);
int puttab(int x, int width, int xskip, char *s, int tab);

void RubbMain(void)
{
	char windowbuf[1024 * 757], textbuf[240 * 1024];
	int width = 30, height = 10, t = 4, spd_x = 1, spd_y = 1;
	int window, i, j, lang = api_getlang(), xskip = 0;
	char s[30], *p, *q = 0, *r = 0;

	api_cmdline(s, 30);
	for (p = s; *p > ' '; ++p) { }
	for (; *p != 0; ) {
		p = skipspace(p);
		if (*p == '-') {
			if (p[1] == 'w') {
				width = strtol(p + 2, &p, 0);
				if (width < 20) {
					width = 20;
				}
				if (width > 126) {
					width = 126;
				}
			} else if (p[1] == 'h') {
				height = strtol(p + 2, &p, 0);
				if (height < 1) {
					height = 1;
				}
				if (height > 45) {
					height = 45;
				}
			} else if (p[1] == 't') {
				t = strtol(p + 2, &p, 0);
				if (t < 1) {
					t = 4;
				}
			} else {
err:
				api_putstr(" >textview file [-w30 -h10 -t4]\n");
				api_end();
			}
		} else {
			if (q != 0) {
				goto err;
			}
			q = p;
			for (; *p > ' '; ++p) { }
			r = p;
		}
	}
	if (q == 0) {
		goto err;
	}
	window = api_open_window(windowbuf, width * 8 + 16, height * 16 + 37, -1, "textview");
	api_boxfill_on_window(window, 6, 27, width * 8 + 9, height * 16 + 30, 7);

	*r = 0;
	i = api_fopen(q);
	if (i == 0) {
		api_putstr("file open error.\n");
		api_end();
	}
	j = api_fsize(i, 0);
	if (j >= 240 * 1024 - 1) {
		j = 240 * 1024 - 2;
	}
	textbuf[0] = 0x0a;
	api_fread(textbuf + 1, j, i);
	api_fclose(i);
	textbuf[j + 1] = 0;
	q = textbuf + 1;
	for (p = textbuf + 1; *p != 0; ++p) {
		if (*p != 0x0d) {
			*q = *p;
			q++;
		}
	}
	*q = 0;

	p = textbuf + 1;
	for (;;) {
		textview(window, width, height, xskip, p, t, lang);
		i = api_getkey(1);
		if (i == 'Q' || i == 'q') {
			api_end();
		}
		if ('A' <= i && i <= 'F') {
			spd_x = 1 << (i - 'A');
		}
		if ('a' <= i && i <= 'f') {
			spd_y = 1 << (i - 'a');
		}
		if (i == '<' && t > 1) {
			t /= 2;
		}
		if (i == '>' && t < 256) {
			t *= 2;
		}
		if (i == '4') {
			for (;;) {
				xskip -= spd_x;
				if (xskip < 0) {
					xskip = 0;
				}
				if (api_getkey(0) != '4') {
					break;
				}
			}
		}
		if (i == '6') {
			for (;;) {
				xskip += spd_x;
				if (api_getkey(0) != '6') {
					break;
				}
			}
		}
		if (i == '8') {
			for (;;) {
				for (j = 0; j < spd_y; ++j) {
					if (p == textbuf + 1) {
						break;
					}
					for(--p; p[-1] != 0x0a; --p) { }
				}
				if (api_getkey(0) != '8') {
					break;
				}
			}
		}
		if (i == '2') {
			for (;;) {
				for (j = 0; j < spd_y; ++j) {
					for (q = p; *q != 0 && *q != 0x0a; ++q) { }
					if (*q == 0) {
						break;
					}
					p = q + 1;
				}
				if (api_getkey(0) != '2') {
					break;
				}
			}
		}
	}
}

char *skipspace(char *p)
{
	for (; *p == ' '; ++p) { }
	return p;
}

void textview(int window, int width, int height, int xskip, char *p, int tab, int lang)
{
	int i;
	api_boxfill_on_window(window + 1, 8, 29, width * 8 + 7, height * 16 + 28, 7);
	for (i = 0; i < height; ++i) {
		p = lineview(window, width, i * 16 + 29, xskip, p, tab, lang);
	}
	api_refresh_window(window, 8, 29, width * 8 + 8, height * 16 + 29);
	return;
}

char *lineview(int window, int width, int y, int xskip, unsigned char *p, int tab, int lang)
{
	int x = -xskip;
	char s[130];
	for (;;) {
		if (*p == 0) { break; }
		if (*p == 0x0a) {
			++p;
			break;
		}
		if (lang == 0) {
			if (*p == 0x09) {
				x = puttab(x, width, xskip, s, tab);
			} else {
				if (0 <= x && x < width) {
					s[x] = *p;
				}
				++x;
			}
			++p;
		}
		if (lang == 1) {
			if (*p == 0x09) {
				x = puttab(x, width, xskip, s, tab);
				++p;
			} else if ((0x81 <= *p && *p <= 0x9f) || (0xe0 <= *p && *p <= 0xfc)) {
				if (x == -1) {
					s[0] = ' ';
				}
				if (0 <= x && x < width - 1) {
					s[x] = *p;
					s[x + 1] = p[1];
				}
				if (x == width - 1) {
					s[x] = ' ';
				}
				x += 2;
				p += 2;
			} else {
				if (0 <= x && x < width) {
					s[x] = *p;
				}
				++x;
				++p;
			}
		}
		if (lang == 2) {
			if (*p == 0x09) {
				x = puttab(x, width, xskip, s, tab);
				++p;
			} else if (0xa1 <= *p && *p <= 0xfe) {
				if (x == -1) {
					s[0] = ' ';
				}
				if (0 <= x && x < width - 1) {
					s[x] = *p;
					s[x + 1] = p[1];
				}
				if (x == width - 1) {
					s[x] = ' ';
				}
				x += 2;
				p += 2;
			} else {
				if (0 <= x && x < width) {
					s[x] = *p;
				}
				++x;
				++p;
			}
		}
	}
	if (x > width) {
		x = width;
	}
	if (x > 0) {
		s[x] = 0;
		api_putstr_on_window(window + 1, 8, y, 0, x, s);
	}
	return p;
}

int puttab(int x, int width, int xskip, char *s, int tab)
{
	for (;;) {
		if (0 <= x && x < width) { s[x] = ' '; }
		++x;
		if ((x + xskip) % tab == 0) { break; }
	}
	return x;
}
