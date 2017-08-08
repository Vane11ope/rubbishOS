#include "bootpack.h"
#define ADR_BINFO   0x00000ff0
#define ADR_GDT     0x00270000
#define PORT_KEYDAT 0x0060
#define KEYCMD_LED  0xed
#define MEMMAN_ADDR 0x003c0000
#define CONSOLE_ON  2
#define CONSOLE_OFF 3

extern struct TIMERCTL timerctl;
int keywin_on(struct SHEET *key_win, struct SHEET *sht_win, int cursor_color);
int keywin_off(struct SHEET *key_win, struct SHEET *sht_win, int cursor_color, int cursor_x);

void RubbMain(void)
{
	// variables in use
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BINFO;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_sub, *sht_console[2], *sheet = 0, *key_win;
	struct FIFO32 fifo, keycmd;
	struct TIMER *timer;
	struct TASK *task_a, *task_console[2], *task;
	struct CONSOLE *console;
	unsigned char *sht_buf_back, sht_buf_mouse[256], *sht_buf_win, *sht_buf_win_sub, *sht_buf_console[2];
	char s[40];
	const short CONSOLE_TEXTBOX_WIDTH = CONSOLE_WIDTH - CHAR_WIDTH * 2;
	const short CONSOLE_TEXTBOX_HEIGHT = CONSOLE_HEIGHT - 37;
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
	int key_shift = 0, key_ctrl = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
	int i = 0, j, x, y, mmx = -1, mmy = -1;
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
	task_run(task_a, 1, 2);

	// sheet settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	*((int *)0xfe4) = shtctl;
	// sheet alloc
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	sht_win_sub = sheet_alloc(shtctl);
	// sheet buf memory alloc
	sht_buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sht_buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 100);
	sht_buf_win_sub = (unsigned char *)memman_alloc_4k(memman, 144 * 52);
	// set buf for each sheet
	sheet_setbuf(sht_back, sht_buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, sht_buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, sht_buf_win, 160, 100, -1);
	sheet_setbuf(sht_win_sub, sht_buf_win_sub, 144, 52, -1);
	// console settings
	for (i = 0; i < 2; ++i) {
		sht_console[i] = sheet_alloc(shtctl);
		sht_buf_console[i] = (unsigned char *)memman_alloc_4k(memman, CONSOLE_WIDTH * CONSOLE_HEIGHT);
		sheet_setbuf(sht_console[i], sht_buf_console[i], CONSOLE_WIDTH, CONSOLE_HEIGHT, -1);
		make_window(sht_buf_console[i], CONSOLE_WIDTH, CONSOLE_HEIGHT, "terminal", 0);
		make_textbox8(sht_console[i], 8, WINDOW_TITLE_HEIGHT, CONSOLE_TEXTBOX_WIDTH, CONSOLE_TEXTBOX_HEIGHT, COL8_000000);
		task_console[i] = task_alloc();
		task_console[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
		task_console[i]->tss.eip = (int) &console_task;
		task_console[i]->tss.es = 1 * 8;
		task_console[i]->tss.cs = 2 * 8;
		task_console[i]->tss.ss = 1 * 8;
		task_console[i]->tss.ds = 1 * 8;
		task_console[i]->tss.fs = 1 * 8;
		task_console[i]->tss.gs = 1 * 8;
		*((int *)(task_console[i]->tss.esp + 4)) = (int) sht_console[i];
		*((int *)(task_console[i]->tss.esp + 8)) = (int) memtotal;
		task_run(task_console[i], 2, 2);
		sht_console[i]->task = task_console[i];
		sht_console[i]->flags |= 0x20;
	}

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

	// drawing some information on the screen
	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 240, 192, COL8_FFFFFF, "VANELLOPE");
	putfonts8_asc(sht_buf_back, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	// make window
	make_textbox8(sht_win_sub, 8, WINDOW_TITLE_HEIGHT, 128, 16, COL8_FFFFFF);
	win_cursor_x = 8;
	win_cursor_color = COL8_FFFFFF;

	// sheet positionings(refresh included)
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mouse_x, mouse_y);
	sheet_slide(sht_console[0], 500, 26);
	sheet_slide(sht_console[1], 0, 26);
	sheet_slide(sht_win, 600, 72);
	sheet_slide(sht_win_sub, 600, 500);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_console[1], 2);
	sheet_updown(sht_console[0], 3);
	sheet_updown(sht_win_sub, 4);
	sheet_updown(sht_mouse, 5);
	key_win = sht_win_sub;

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
			if (key_win->flags == 0) {
				key_win = shtctl->sheets[shtctl->top - 1];
				win_cursor_color = keywin_on(key_win, sht_win_sub, win_cursor_color);
			}
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
					if (key_win == sht_win_sub) {
						if (win_cursor_x < 128) {
							s[1] = 0;
							putfonts8_asc_sht(sht_win_sub, win_cursor_x, WINDOW_TITLE_HEIGHT, COL8_000000, COL8_FFFFFF, s);
							win_cursor_x += 8;
						}
					} else if (key_win == sht_console[0] || key_win == sht_console[1]) {
						if (key_ctrl != 0 ) {
							task = key_win->task;
							if (s[0] == 'c' && task->tss.ss0 != 0) {
								console_putstr(task->console, "\nBreak(key) :\n");
								io_cli();
								task->tss.eax = (int) &(task->tss.esp0);
								task->tss.eip = (int) asm_end_app;
								timer_cancelall(&task->fifo);
								io_sti();
							} else if ((s[0] == 'L' || s[0] == 'l') && key_ctrl != 0) {
								fifo32_put(&task_console[0]->fifo, 1111);
							} else {
								fifo32_put(&task_console[0]->fifo, s[0] + 256);
							}
						} else {
							fifo32_put(&key_win->task->fifo, s[0] + 256);
						}
					} else {
						fifo32_put(&key_win->task->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x3b && shtctl->top > 2) {
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				if (i == 256 + 0x0e) {
					if (key_win == sht_win_sub) {
						if (win_cursor_x > 8) {
							putfonts8_asc_sht(sht_win_sub, win_cursor_x, WINDOW_TITLE_HEIGHT, COL8_000000, COL8_FFFFFF, " ");
							win_cursor_x -= 8;
						}
					} else {
						fifo32_put(&key_win->task->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x0f) {  // tab
					win_cursor_color = keywin_off(key_win, sht_win_sub, win_cursor_color, win_cursor_x);
					j = key_win->height - 1;
					if (j == 0) { j = shtctl->top - 1; }
					key_win = shtctl->sheets[j];
					win_cursor_color = keywin_on(key_win, sht_win_sub, win_cursor_color);
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
				if (i == 256 + 0x1d) {
					key_ctrl |= 1;
				}
				if (i == 256 + 0x9d) {
					key_ctrl &= ~1;
				}
				if (i == 256 + 0x1c) { // Enter
					if (key_win != sht_win_sub) {
						fifo32_put(&key_win->task->fifo, 10 + 256);
					}
				}
				if (win_cursor_color >= 0) {
					boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, win_cursor_color, win_cursor_x, WINDOW_TITLE_HEIGHT, win_cursor_x + 7, 43);
				}
				sheet_refresh(sht_win_sub, win_cursor_x, WINDOW_TITLE_HEIGHT, win_cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) {
				if (mouse_decode(&mdec, i - 512) != 0) {
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
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
						if (mmx < 0) {
							for (j = shtctl->top - 1; j > 0; --j) {
								sheet = shtctl->sheets[j];
								x = mouse_x - sheet->vx0;
								y = mouse_y - sheet->vy0;
								if (0 <= x && x < sheet->bxsize && 0 <= y && y < sheet->bysize) {
									if (sheet->buf[y * sheet->bxsize + x] != sheet->opacity) {
										sheet_updown(sheet, shtctl->top - 1);
										if (sheet != key_win) {
											win_cursor_color = keywin_off(key_win, sht_win_sub, win_cursor_color, win_cursor_x);
											key_win = sheet;
											win_cursor_color = keywin_on(key_win, sht_win_sub, win_cursor_color);
										}
										if (3 <= x && x < sheet->bxsize - 3 && 3 <= y && y < 21) {
											mmx = mouse_x;
											mmy = mouse_y;
										}
										if (sheet->bxsize - 21 <= x && x < sheet->bxsize - 5 && 5 <= y && y < 19 && (sheet->flags & 0x10) != 0) {
											task = sheet->task;
											console_putstr(task->console, "\nBreak(mouse) :\n");
											io_cli();
											task->tss.eax = (int)&(task->tss.esp0);
											task->tss.eip = (int)asm_end_app;
											io_sti();
										}
										break;
									}
								}
							}
						} else {
							x = mouse_x - mmx;
							y = mouse_y - mmy;
							sheet_slide(sheet, sheet->vx0 + x, sheet->vy0 + y);
							mmx = mouse_x;
							mmy = mouse_y;
						}
					} else {
						mmx = -1;
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 50, 0, COL8_FFFFFF, COL8_000000, s);
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
					boxfill8(sht_win_sub->buf, sht_win_sub->bxsize, win_cursor_color, win_cursor_x, WINDOW_TITLE_HEIGHT,  win_cursor_x + 7, 43);
					sheet_refresh(sht_win_sub, win_cursor_x, WINDOW_TITLE_HEIGHT, win_cursor_x + 8, 44);
				}
			}
		}
	}
}

int keywin_on(struct SHEET *key_win, struct SHEET *sht_win, int cursor_color)
{
	change_window_title(key_win, 1);
	if (key_win == sht_win) {
		cursor_color = COL8_000000;
	} else {
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, CONSOLE_ON);
		}
	}
	return cursor_color;
}

int keywin_off(struct SHEET *key_win, struct SHEET *sht_win, int cursor_color, int cursor_x)
{
	change_window_title(key_win, 0);
	if (key_win == sht_win) {
		cursor_color = -1;
		boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
	} else {
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, CONSOLE_OFF);
		}
	}
	return cursor_color;
}
