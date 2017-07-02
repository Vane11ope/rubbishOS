#include "bootpack.h"

extern struct TIMERCTL timerctl;

void RubbMain(void)
{
	// variables in use
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x00000ff0;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_sub;
	struct FIFO32 fifo;
	struct TIMER *timer, *timer2, *timer3;
	unsigned char *sht_buf_back, sht_buf_mouse[256], *sht_buf_win, *sht_buf_win_sub;
	unsigned char temp10[7] = "10[sec]";
	unsigned char temp3[6] = "3[sec]";
	char s[40];
	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	unsigned int memtotal, count = 0;
	int fifobuf[128];
	int mouse_x = (binfo->scrnx - 16) / 2;
	int mouse_y = (binfo->scrny - 28 - 16) / 2;
	int mouse_w = 16;
	int mouse_h = 16;
	int mouse_s = 16;
	int mouse_offset = 5;
	int i = 0;

	// memory manager
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	// segment settings
	init_gdtidt();
	// interrupt settings
	init_pic();
	// timerctl init
	init_pit();
	// color settings
	init_palette();

	// fifo init
	fifo32_init(&fifo, 128, fifobuf);

	// sheet settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	// sheet alloc
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	sht_win_sub = sheet_alloc(shtctl);
	// sheet buf memory alloc
	sht_buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sht_buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 100);
	sht_buf_win_sub = (unsigned char *)memman_alloc_4k(memman, 160 * 50);
	// set buf for each sheet
	sheet_setbuf(sht_back, sht_buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, sht_buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, sht_buf_win, 160, 100, -1);
	sheet_setbuf(sht_win_sub, sht_buf_win_sub, 160, 50, -1);

	// init screens and mouse graphics after sheet settings
	init_screen(sht_buf_back, binfo->scrnx, binfo->scrny);
	init_mouse(sht_buf_mouse, 99);
	make_window(sht_buf_win, 160, 100, "window");
	make_window(sht_buf_win_sub, 160, 50, "Counter");
	int tempX = 20;
	int tempY = 28;
	putfonts8_asc(sht_buf_win, 160, tempX, tempY, COL8_000000, "Composite");
	putfonts8_asc(sht_buf_win, 160, tempX + 16, tempY + 16, COL8_000000, "Number");
	putfonts8_asc(sht_buf_win, 160, tempX + 32, tempY + 32, COL8_000000, "Gorilla");
	putfonts8_asc(sht_buf_win, 160, tempX + 48, tempY + 48, COL8_000000, "Vanellope");

	// drawing some information on the screen
	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 240, 145, COL8_FFFFFF, "VANELLOPE");
	putfonts8_asc(sht_buf_back, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	// show timer
	sprintf(s, "%010d", timerctl.count);
	putfonts8_asc(sht_buf_win_sub, 160, 70, 28, COL8_000000, s);

	// sheet positionings(refresh included)
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mouse_x, mouse_y);
	sheet_slide(sht_win, 80, 72);
	sheet_slide(sht_win_sub, 150, 50);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_win_sub, 2);
	sheet_updown(sht_mouse, 3);

	// set each timer
	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);

	// keyboard
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	// start accepting keyboard and mouse interrupts
	io_out8(PIC0_IMR, 0xf8);
	io_out8(PIC1_IMR, 0xef);

	// finally the interrupt flags are on
	io_sti();

	for (;;) {
		sprintf(s, "%010d", timerctl.count);
		putfonts8_asc_sht(sht_win_sub, 70, 28, COL8_000000, COL8_C6C6C6, s);
		io_cli();
		if (fifo32_status(&fifo) <= 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) {
				sprintf(s, "%02X", i);
				putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_000000, s);
			} else if (512 <= i && i <= 767) {
				if (mouse_decode(&mdec, i) != 0) {
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 50, 0, COL8_FFFFFF, COL8_000000, s);
					mouse_x += mdec.x;
					mouse_y += mdec.y;
					if (mouse_x < 0) {
						mouse_x = 0;
					}
					if (mouse_y < 0) {
						mouse_y = 0;
					}
					if (mouse_x > binfo->scrnx - mouse_offset) {
						mouse_x = binfo->scrnx - mouse_offset;
					}
					if (mouse_y > binfo->scrny - mouse_offset) {
						mouse_y = binfo->scrny - mouse_offset;
					}
					sprintf(s, "(%3d, %3d)", mouse_x, mouse_y);
					putfonts8_asc_sht(sht_back, 0, 50, COL8_FFFFFF, COL8_000000, s);
					sheet_slide(sht_mouse, mouse_x, mouse_y);
				}
			} else if (i == 10) {
				putfonts8_asc_sht(sht_back, 0, 70, COL8_FFFFFF, COL8_000000, temp10);
			} else if (i == 3) {
				putfonts8_asc_sht(sht_back, 0, 86, COL8_FFFFFF, COL8_000000, temp3);
			} else if (i == 1) {
				timer_init(timer3, &fifo, 0);
				boxfill8(sht_buf_back, binfo->scrnx, COL8_FFFFFF, 8, 102, 15, 117);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 102, 16, 119);
			} else if (i == 0) {
				timer_init(timer3, &fifo, 1);
				boxfill8(sht_buf_back, binfo->scrnx, COL8_000000, 8, 102, 15, 117);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 102, 16, 119);
			}
		}
	}
}

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflag, cr0, i;
	eflag = io_load_eflags();
	eflag |= EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if ((eflag & EFLAGS_AC_BIT) != 0) { flg486 = 1; }
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	i = memtest_sub(start, end);
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	volatile unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for (i = start; i <= end; i += 0x1000) {
		p = (unsigned int *) (i + 0xffc);
		old = *p;
		*p = pat0;
		*p ^= 0xffffffff;
		if (*p != pat1) {
not_memory:
			*p = old;
			break;
		}
		*p ^= 0xffffffff;
		if (*p != pat0) {
			goto not_memory;
		}
		*p = old;
	}
	return i;
}

void make_window(unsigned char *buf, int xsize, int ysize, char *title)
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
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
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
