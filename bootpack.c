#include "bootpack.h"

extern struct TIMERCTL timerctl;

void RubbMain(void)
{
	// variables in use
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x00000ff0;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct TSS32 tss_a, tss_b;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_sub;
	struct FIFO32 fifo;
	struct TIMER *timer, *timer2, *timer3, *timer_ts;
	unsigned char *sht_buf_back, sht_buf_mouse[256], *sht_buf_win, *sht_buf_win_sub;
	char s[40];
	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	unsigned int memtotal, count = 0;
	int task_b_esp;
	int fifobuf[128];
	int mouse_x = (binfo->scrnx - 16) / 2;
	int mouse_y = (binfo->scrny - 28 - 16) / 2;
	int mouse_w = 16;
	int mouse_h = 16;
	int mouse_s = 16;
	int mouse_offset = 5;
	int win_cursor_x, win_cursor_color;
	int i = 0;
	// each key
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};

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

	// multitasking
	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	tss_b.eip = (int) &task_b_main;
	tss_b.eflags = 0x00000202;
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;
	set_segmdesc(gdt + 3, 103, &tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, &tss_b, AR_TSS32);
	load_tr(3 * 8);

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
	// memorize sht_back
	(*(int *)(task_b_esp + 4)) = (int)sht_back;

	// init screens and mouse graphics after sheet settings
	init_screen(sht_buf_back, binfo->scrnx, binfo->scrny);
	init_mouse(sht_buf_mouse, 99);
	make_window(sht_buf_win, 160, 100, "window");
	make_window(sht_buf_win_sub, 160, 50, "window");
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

	// make window
	make_textbox8(sht_win_sub, 8, 28, 144, 16, COL8_FFFFFF);
	win_cursor_x = 8;
	win_cursor_color = COL8_FFFFFF;

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
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);

	// keyboard
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	// start accepting keyboard and mouse interrupts
	io_out8(PIC0_IMR, 0xf8);
	io_out8(PIC1_IMR, 0xef);

	// finally the interrupt flags are on
	io_sti();

	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) <= 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 2) {
				farjmp(0, 4 * 8);
				timer_settime(timer_ts, 2);
			} else if (256 <= i && i <= 511) {
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_000000, s);
				if (i < 0x54 + 256) {
					if (keytable[i - 256] != 0 && win_cursor_x < 144) {
						s[0] = keytable[i - 256];
						s[1] = 0;
						putfonts8_asc_sht(sht_win_sub, win_cursor_x, 28, COL8_000000, COL8_FFFFFF, s);
						win_cursor_x += 8;
					}
				}
				if (i == 256 + 0x0e && win_cursor_x > 8) {
					putfonts8_asc_sht(sht_win_sub, win_cursor_x, 28, COL8_000000, COL8_FFFFFF, " ");
					win_cursor_x -= 8;
				}
				boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, win_cursor_color, win_cursor_x, 28, win_cursor_x + 7, 43);
				sheet_refresh(sht_win_sub, win_cursor_x, 28, win_cursor_x + 8, 44);
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
					if (mdec.btn & 0x01 != 0) {
						sheet_slide(sht_win_sub, mouse_x - 80, mouse_y - 8);
					}
				}
			} else if (i == 10) {
				putfonts8_asc_sht(sht_back, 0, 70, COL8_FFFFFF, COL8_000000, "10[sec]");
				farjmp(0, 4 * 8);
			} else if (i == 3) {
				putfonts8_asc_sht(sht_back, 0, 86, COL8_FFFFFF, COL8_000000, "3[sec]");
			} else if (i <= 1) {
				if (i != 0) {
					timer_init(timer3, &fifo, 0);
					win_cursor_color = COL8_000000;
				} else {
					timer_init(timer3, &fifo, 1);
					win_cursor_color = COL8_FFFFFF;
				}
				timer_settime(timer3, 50);
				boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, win_cursor_color, win_cursor_x, 28,  win_cursor_x + 7, 43);
				sheet_refresh(sht_win_sub, win_cursor_x, 28, win_cursor_x + 8, 44);
			}
		}
	}
}

void task_b_main(struct SHEET *sht_back)
{
	struct FIFO32 fifo;
	struct TIMER *timer_1s, *timer_put;
	int i, fifobuf[128], count0 = 0, count = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);
	timer_put = timer_alloc();
	timer_init(timer_put, &fifo, 1);
	timer_settime(timer_put, 1);

	for(;;) {
		++count;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 1) {
				sprintf(s, "%10d", count);
				putfonts8_asc_sht(sht_back, 0, 192, COL8_FFFFFF, COL8_000000, s);
				timer_settime(timer_put, 1);
			} else if (i == 100) {
				sprintf(s, "%10d", count - count0);
				putfonts8_asc_sht(sht_back, 0, 208, COL8_FFFFFF, COL8_000000, s);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}
