#include "bootpack.h"
#define ADR_BINFO   0x00000ff0
#define ADR_GDT     0x00270000
#define PORT_KEYDAT 0x0060
#define KEYCMD_LED  0xed
#define MEMMAN_ADDR 0x003c0000
#define CONSOLE_ON  2
#define CONSOLE_OFF 3

extern struct TIMERCTL timerctl;

void RubbMain(void)
{
	// variables in use
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BINFO;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_sub, *sht_console;
	struct FIFO32 fifo, keycmd;
	struct TIMER *timer;
	struct TASK *task_a, *task_console;
	unsigned char *sht_buf_back, sht_buf_mouse[256], *sht_buf_win, *sht_buf_win_sub, *sht_buf_console;
	char s[40];
	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	unsigned int memtotal, count = 0;
	int fifobuf[128], keycmd_buf[32];
	int mouse_x = (binfo->scrnx - 16) / 2;
	int mouse_y = (binfo->scrny - 28 - 16) / 2;
	int mouse_w = 16;
	int mouse_h = 16;
	int mouse_s = 16;
	int mouse_offset = 5;
	int win_cursor_x, win_cursor_color;
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
	int i = 0;
	// each key
	static char keytable[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable_shift[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '(J~(B', '=', '(J~(B', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
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
	// interrupt flags on
	io_sti();
	// timerctl init
	init_pit();
	// color settings
	init_palette();

	// fifo init
	fifo32_init(&fifo, 128, fifobuf, 0);
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	// multitasking
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	// sheet settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	// sheet alloc
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	sht_win_sub = sheet_alloc(shtctl);
	sht_console = sheet_alloc(shtctl);
	// sheet buf memory alloc
	sht_buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sht_buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 100);
	sht_buf_win_sub = (unsigned char *)memman_alloc_4k(memman, 144 * 52);
	sht_buf_console = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	// set buf for each sheet
	sheet_setbuf(sht_back, sht_buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, sht_buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, sht_buf_win, 160, 100, -1);
	sheet_setbuf(sht_win_sub, sht_buf_win_sub, 144, 52, -1);
	sheet_setbuf(sht_console, sht_buf_console, 256, 165, -1);

	// init screens and mouse graphics after sheet settings
	init_screen(sht_buf_back, binfo->scrnx, binfo->scrny);
	init_mouse(sht_buf_mouse, 99);
	make_window(sht_buf_win, 160, 100, "window", 0);
	make_window(sht_buf_win_sub, 144, 52, "window", 1);
	int tempX = 20;
	int tempY = 28;
	putfonts8_asc(sht_buf_win, 160, tempX, tempY, COL8_000000, "Composite");
	putfonts8_asc(sht_buf_win, 160, tempX + 16, tempY + 16, COL8_000000, "Number");
	putfonts8_asc(sht_buf_win, 160, tempX + 32, tempY + 32, COL8_000000, "Gorilla");
	putfonts8_asc(sht_buf_win, 160, tempX + 48, tempY + 48, COL8_000000, "Vanellope");

	// init console
	make_window(sht_buf_console, 256, 165, "terminal", 0);
	make_textbox8(sht_console, 8, 28, 240, 128, COL8_000000);
	task_console = task_alloc();
	task_console->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_console->tss.eip = (int) &console_task;
	task_console->tss.es = 1 * 8;
	task_console->tss.cs = 2 * 8;
	task_console->tss.ss = 1 * 8;
	task_console->tss.ds = 1 * 8;
	task_console->tss.fs = 1 * 8;
	task_console->tss.gs = 1 * 8;
	*((int *)(task_console->tss.esp + 4)) = (int) sht_console;
	task_run(task_console, 2, 2);

	// drawing some information on the screen
	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 240, 192, COL8_FFFFFF, "VANELLOPE");
	putfonts8_asc(sht_buf_back, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	// make window
	make_textbox8(sht_win_sub, 8, 28, 128, 16, COL8_FFFFFF);
	win_cursor_x = 8;
	win_cursor_color = COL8_FFFFFF;

	// sheet positionings(refresh included)
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mouse_x, mouse_y);
	sheet_slide(sht_console, 64, 26);
	sheet_slide(sht_win, 80, 72);
	sheet_slide(sht_win_sub, 8, 56);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_console, 2);
	sheet_updown(sht_win_sub, 3);
	sheet_updown(sht_mouse, 4);

	// set each timer
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	// keyboard
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	// start accepting keyboard and mouse interrupts
	io_out8(PIC0_IMR, 0xf8);
	io_out8(PIC1_IMR, 0xef);

	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) <= 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) {
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_000000, s);
				if (i < 0x80 + 256) {
					if (key_shift == 0) {
						s[0] = keytable[i - 256];
					} else {
						s[0] = keytable_shift[i - 256];
					}
				} else {
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {
					if ((key_leds & 4) == 0 && key_shift == 0) {
						s[0] += 0x20;
					}
				}
				if (s[0] != 0) {
					if (key_to == 0) {
						if (win_cursor_x < 128) {
							s[1] = 0;
							putfonts8_asc_sht(sht_win_sub, win_cursor_x, 28, COL8_000000, COL8_FFFFFF, s);
							win_cursor_x += 8;
						}
					} else {
						fifo32_put(&task_console->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) {
					if (key_to == 0) {
						if (win_cursor_x > 8) {
							putfonts8_asc_sht(sht_win_sub, win_cursor_x, 28, COL8_000000, COL8_FFFFFF, " ");
							win_cursor_x -= 8;
						}
					} else {
						fifo32_put(&task_console->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x0f) {  // tab
					if (key_to == 0) {
						key_to = 1;
						make_window_title(sht_buf_win_sub, sht_win_sub->bxsize, "window", 0);
						make_window_title(sht_buf_console, sht_console->bxsize, "terminal", 1);
						win_cursor_color = -1;
						boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, COL8_FFFFFF, win_cursor_x, 28, win_cursor_x + 7, 43);
						fifo32_put(&task_console->fifo, CONSOLE_ON);
					} else {
						key_to = 0;
						make_window_title(sht_buf_win_sub, sht_win_sub->bxsize, "window", 1);
						make_window_title(sht_buf_console, sht_console->bxsize, "terminal", 0);
						win_cursor_color = COL8_000000;
						fifo32_put(&task_console->fifo, CONSOLE_OFF);
					}
					sheet_refresh(sht_win_sub, 0, 0, sht_win_sub->bxsize, 21);
					sheet_refresh(sht_console, 0, 0, sht_console->bxsize, 21);
				}
				if (i == 256 + 0x3a) {
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0xfa) {
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if (i == 256 + 0x2a) {
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {
					key_shift &= ~2;
				}
				if (win_cursor_color >= 0) {
					boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, win_cursor_color, win_cursor_x, 28, win_cursor_x + 7, 43);
				}
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
			} else if (i <= 1) {
				if (i != 0) {
					timer_init(timer, &fifo, 0);
					if (win_cursor_color >= 0) {
						win_cursor_color = COL8_000000;
					}
				} else {
					timer_init(timer, &fifo, 1);
					if (win_cursor_color >= 0) {
						win_cursor_color = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (win_cursor_color >= 0) {
					boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, win_cursor_color, win_cursor_x, 28,  win_cursor_x + 7, 43);
					sheet_refresh(sht_win_sub, win_cursor_x, 28, win_cursor_x + 8, 44);
				}
			}
		}
	}
}
