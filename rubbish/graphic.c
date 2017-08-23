#include "bootpack.h"

void init_palette(void)
{
	static unsigned char table_rgb[16*3] = {
		0x00, 0x00, 0x00,
		0xff, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0xff, 0xff, 0x00,
		0x00, 0x00, 0xff,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0xff,
		0xff, 0xff, 0xff,
		0xc6, 0xc6, 0xc6,
		0x84, 0x00, 0x00,
		0x00, 0x84, 0x00,
		0x84, 0x84, 0x00,
		0x00, 0x00, 0x84,
		0x84, 0x00, 0x84,
		0x00, 0x84, 0x84,
		0x84, 0x84, 0x84
	};
	unsigned char table2[216 * 3];
	int r, g, b;
	set_palette(0, 15, table_rgb);
	for (b = 0; b < 6; ++b) {
		for (g = 0; g < 6; ++g) {
			for (r = 0; r < 6; ++r) {
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
		}
	}
	set_palette(16, 231, table2);
	return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03c8, start);
	for(i = start; i <= end; ++i) {
		io_out8(0x3c9, rgb[0] / 4);
		io_out8(0x3c9, rgb[1] / 4);
		io_out8(0x3c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}

void init_screen(char *vram, int x, int y)
{
	boxfill8(vram, x, COL8_000000, 0, 0, x - 1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6, 0, y - 28, x - 1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF, 0, y - 27, x - 1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6, 0, y - 26, x - 1, y - 1);

	boxfill8(vram, x, COL8_FFFFFF, 3, y - 24, 59, y - 24);
	boxfill8(vram, x, COL8_FFFFFF, 2, y - 24, 2, y - 4);
	boxfill8(vram, x, COL8_848484, 3, y - 4, 59, y - 4);
	boxfill8(vram, x, COL8_848484, 59, y - 23, 59, y - 5);
	boxfill8(vram, x, COL8_000000, 2, y - 3, 59, y - 3);
	boxfill8(vram, x, COL8_000000, 60, y - 24, 60, y - 3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x - 4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y - 4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y - 3, x - 4, y - 3);
	boxfill8(vram, x, COL8_FFFFFF, x - 3, y - 24, x - 3, y - 3);
}

void init_mouse(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
    int x, y;
	for (y = 0; y < 16; ++y) {
		for (x = 0; x < 16; ++x) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; ++y) {
		for (x = x0; x<= x1; ++x) {
			*(vram + y * xsize + x) = c;
		}
	}
	return;
}

void make_textbox8(struct SHEET *sheet, int x0, int y0, int sx, int sy, int color)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sheet->buf, sheet->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sheet->buf, sheet->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sheet->buf, sheet->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sheet->buf, sheet->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sheet->buf, sheet->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sheet->buf, sheet->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sheet->buf, sheet->bxsize, color, x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void make_window(unsigned char *buf, int xsize, int ysize, char *title, char isactive)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	make_window_title(buf, xsize, title, isactive);
	return;
}

void make_window_title(unsigned char *buf, int xsize, char *title, char isactive)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c, titlecolor, titleback;
	if (isactive == 0) {
		titlecolor = COL8_C6C6C6;
		titleback = COL8_848484;
	} else {
		titlecolor = COL8_FFFFFF;
		titleback = COL8_000084;
	}
	boxfill8(buf, xsize, titleback, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, titlecolor, title);
	for (y = 0; y < 14; ++y) {
		for (x = 0; x < 16; ++x) {
			c = closebtn[y][x];
			switch (c) {
				case '@':
					c = COL8_000000;
					break;
				case '$':
					c = COL8_848484;
					break;
				case 'Q':
					c = COL8_C6C6C6;
					break;
				default:
					c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char color, char *font)
{
	int i;
	char *p, data;
	for (i = 0; i < 16; ++i) {
		p = vram + (y + i) * xsize + x;
		data = font[i];
		if ((data & 0x80) != 0) {p[0] = color;}
		if ((data & 0x40) != 0) {p[1] = color;}
		if ((data & 0x20) != 0) {p[2] = color;}
		if ((data & 0x10) != 0) {p[3] = color;}
		if ((data & 0x08) != 0) {p[4] = color;}
		if ((data & 0x04) != 0) {p[5] = color;}
		if ((data & 0x02) != 0) {p[6] = color;}
		if ((data & 0x01) != 0) {p[7] = color;}
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char color, unsigned char *s)
{
	extern char hankaku[4096];
	struct TASK *task = task_now();
	char *nihongo = (char *)*((int *)0x0fe8), *font;
	int k, t;
	if (task->langmode == 0) {
		for (; *s != 0x00; ++s) {
			putfont8(vram, xsize, x, y, color, hankaku + *s * 16);
			x += CHAR_WIDTH;
		}
	}
	if (task->langmode == 1) {
		for (; *s != 0x00; ++s) {
			if (task->langbyte == 0) {
				if ((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc)) { task->langbyte = *s; }
				else { putfont8(vram, xsize, x, y, color, nihongo + *s * 16); }
			} else {
				if (0x81 <= task->langbyte && task->langbyte <= 0x9f) {
					k = (task->langbyte - 0x81) * 2;
				} else {
					k = (task->langbyte - 0xe0) * 2 + 62;
				}
				if (0x40 <= *s && *s <= 0x7e) {
					t = *s - 0x40;
				} else if (0x80 <= *s && *s <= 0x9e) {
					t = *s - 0x80 + 63;
				} else {
					t = *s - 0x9f;
					++k;
				}
				task->langbyte = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - CHAR_WIDTH, y, color, font);
				putfont8(vram, xsize, x, y, color, font + 16);
			}
			x += CHAR_WIDTH;
		}
	}
	return;
}

void putfonts8_asc_sht(struct SHEET *sheet, int x, int y, int color, int backcolor, char *str)
{
	int len = length(str);
	boxfill8(sheet->buf, sheet->bxsize, backcolor, x, y, x + len * 8 - 1, y + 15);
	putfonts8_asc(sheet->buf, sheet->bxsize, x, y, color, str);
	sheet_refresh(sheet, x, y, x + len * 8, y + 16);
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; ++y) {
		for (x = 0; x < pxsize; ++x) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}

void drawline(struct SHEET *sheet, int x0, int y0, int x1, int y1, int color)
{
	int i, x, y, len, dx, dy;
	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0) { dx = -dx; }
	if (dy < 0) { dy = -dy; }
	if (dx >= dy) {
		len = dx + 1;
		if (x0 > x1) { dx = -1024; }
		else { dx = 1024; }
		if (y0 <= y1) { dy = ((y1 - y0 + 1) << 10) / len; }
		else { dy = ((y1 - y0 - 1) << 10) / len; }
	} else {
		len = dy + 1;
		if (y0 > y1) { dy = -1024; }
		else { dy = 1024; }
		if (x0 <= x1) { dx = ((x1 - x0 + 1) << 10) / len; }
		else { dx = ((x1 - x0 - 1) << 10) / len; }
	}
	for (i = 0; i < len; ++i) {
		sheet->buf[(y >> 10) * sheet->bxsize + (x >> 10)] = color;
		x += dx;
		y += dy;
	}
	return;
}

void change_window_title(struct SHEET *sheet, char isactive)
{
	int x, y, xsize = sheet->bxsize;
	char color, title_color_new, title_color_new_back, title_color_old, title_color_old_back, *buf = sheet->buf;
	if (isactive != 0) {
		title_color_new = COL8_FFFFFF;
		title_color_new_back = COL8_000084;
		title_color_old = COL8_C6C6C6;
		title_color_old_back = COL8_848484;
	} else {
		title_color_new = COL8_C6C6C6;
		title_color_new_back = COL8_848484;
		title_color_old = COL8_FFFFFF;
		title_color_old_back = COL8_000084;
	}
	for (y = 3; y <= 20; ++y) {
		for (x = 3; x <= xsize - 4; ++x) {
			color = buf[y * xsize + x];
			if (color == title_color_old && x <= xsize - 22) {
				color = title_color_new;
			} else if (color == title_color_old_back) {
				color = title_color_new_back;
			}
			buf[y * xsize + x] = color;
		}
	}
	sheet_refresh(sheet, 3, 3, xsize, 21);
	return;
}
