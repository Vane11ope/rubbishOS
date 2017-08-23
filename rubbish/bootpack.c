#include "bootpack.h"
#define ADR_BINFO   0x00000ff0
#define ADR_GDT     0x00270000
#define ADR_DISKIMG  0x00100000
#define PORT_KEYDAT 0x0060
#define KEYCMD_LED  0xed
#define MEMMAN_ADDR 0x003c0000
#define CONSOLE_ON   2
#define CONSOLE_OFF  3
#define CONSOLE_SHUT 4

extern struct TIMERCTL timerctl;
extern struct TASKCTL *taskctl;
extern const short CONSOLE_TEXTBOX_WIDTH;
extern const short CONSOLE_TEXTBOX_HEIGHT;
void keywin_on(struct SHEET *key_win);
void keywin_off(struct SHEET *key_win);
struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal);
struct TASK* open_console_task(struct SHEET *sheet, unsigned int memtotal);
void close_console_task(struct TASK *task);
void close_console(struct SHEET* sheet);

void RubbMain(void)
{
	// variables in use
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BINFO;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sheet = 0, *sheet2, *key_win;
	struct FIFO32 fifo, keycmd;
	struct TASK *task_a, *task_console[2], *task;
	struct CONSOLE *console;
	struct FILEINFO *finfo;
	extern char hankaku[4096];
	unsigned char *sht_buf_back, sht_buf_mouse[256], *sht_buf_console[2], *nihongo;
	char s[40];
	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	unsigned int memtotal, count = 0;
	int fifobuf[128], keycmd_buf[32], *cons_fifo[2], *fat;
	int mouse_x = (binfo->scrnx - 16) / 2;
	int mouse_y = (binfo->scrny - 28 - 16) / 2;
	int mouse_w = 16;
	int mouse_h = 16;
	int mouse_s = 16;
	int mouse_offset = 5;
	int key_shift = 0, key_ctrl = 0, key_command = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
	int i = 0, j, x, y, mmx = -1, mmy = -1, mmx2 = 0, new_mouse_y = 0, new_mouse_x = -1, new_window_x = 0x7fffffff, new_window_y = 0;
	// each key
	static char keytable[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable_shift[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '(J~(B', '=', '(J~(B', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};

	// fifo init
	fifo32_init(&fifo, 128, fifobuf, 0);
	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	*((int *)0x0fec) = (int)&fifo;

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

	// sheet settings
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	*((int *)0xfe4) = shtctl;

	// multitasking
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	task_a->langmode = 0;

	// sheet alloc
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	// sheet buf memory alloc
	sht_buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	// set buf for each sheet
	sheet_setbuf(sht_back, sht_buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, sht_buf_mouse, 16, 16, 99);

	// console
	key_win = open_console(shtctl, memtotal);

	// init screens and mouse graphics after sheet settings
	init_screen(sht_buf_back, binfo->scrnx, binfo->scrny);
	init_mouse(sht_buf_mouse, 99);

	// drawing some information on the screen
	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 240, 192, COL8_FFFFFF, "VANELLOPE");
	putfonts8_asc(sht_buf_back, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	// sheet positionings(refresh included)
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mouse_x, mouse_y);
	sheet_slide(key_win, 16, 32);
	sheet_updown(sht_back, 0);
	sheet_updown(key_win, 1);
	sheet_updown(sht_mouse, 2);
	keywin_on(key_win);

	// keyboard
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);

	// start accepting keyboard and mouse interrupts
	io_out8(PIC0_IMR, 0xf8);
	io_out8(PIC1_IMR, 0xef);

	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	// load nihongo
	nihongo = (unsigned char *)memman_alloc_4k(memman, 16 * 256 + 32 * 94 * 47);
	fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));
	finfo = file_search("nihongo.fnt", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo != 0) {
		file_loadfile(finfo->cluster_no, finfo->size, nihongo, fat, (char *)(ADR_DISKIMG + 0x003e00));
	} else {
		for (i = 0; i < 16 * 256; ++i) {
			nihongo[i] = hankaku[i];
		}
		for (i = 16 * 256; i < 16 * 256 + 32 * 94 * 47; ++i) {
			nihongo[i] = 0xff;
		}
	}
	*((int *)0x0fe8) = (int) nihongo;
	memman_free_4k(memman, (int)fat, 4 * 2880);

	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) <= 0) {
			if (new_mouse_x >= 0) {
				io_sti();
				sheet_slide(sht_mouse, new_mouse_x, new_mouse_y);
				new_mouse_x = -1;
			} else if (new_window_x != 0x7fffffff) {
				io_sti();
				sheet_slide(sheet, new_window_x, new_window_y);
				new_window_x = 0x7fffffff;
			} else {
				task_sleep(task_a);
				io_sti();
			}
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win != 0 && key_win->flags == 0) {
				if (shtctl->top == 1) {
					key_win = 0;
				} else {
					key_win = shtctl->sheets[shtctl->top - 1];
					keywin_on(key_win);
				}
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
				if (s[0] != 0 && key_win != 0) {
					if (key_command != 0) {
						if (s[0] == 'n' && (key_win->flags & 0x20) != 0) { // open new console
							keywin_off(key_win);
							key_win = open_console(shtctl, memtotal);
							sheet_slide(key_win, 200, 16);
							sheet_updown(key_win, shtctl->top);
							keywin_on(key_win);
							continue;
						} else if (s[0] == 'w') { // close console
							if ((key_win->flags & 0x20) != 0) {
								io_cli();
								fifo32_put(&key_win->task->fifo, CONSOLE_SHUT);
								io_sti();
							} else if ((key_win->flags & 0x10) != 0) {
								io_cli();
								key_win->task->tss.eax = (int) &(key_win->task->tss.esp0);
								key_win->task->tss.eip = (int) asm_end_app;
								timer_cancelall(&key_win->task->fifo);
								io_sti();
								task_run(key_win->task, -1, 0);
							}
						}
					} else if (key_ctrl != 0 && (key_win->flags & 0x20) != 0) {
						task = key_win->task;
						if (s[0] == 'c' && task->tss.ss0 != 0) {
							console_putstr(task->console, "\nBreak(key) :\n");
							io_cli();
							task->tss.eax = (int) &(task->tss.esp0);
							task->tss.eip = (int) asm_end_app;
							timer_cancelall(&task->fifo);
							io_sti();
							task_run(task, -1, 0);
						} else if ((s[0] == 'L' || s[0] == 'l') && key_ctrl != 0) {
							fifo32_put(&task->fifo, 1111);
						} else {
							fifo32_put(&task->fifo, s[0] + 256);
						}
					} else if ((key_win->flags & 0x20) != 0) {
						fifo32_put(&key_win->task->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x3b && shtctl->top > 2) {
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				if (i == 256 + 0x0f && key_win != 0) {  // tab
					keywin_off(key_win);
					j = key_win->height - 1;
					if (j == 0) { j = shtctl->top - 1; }
					key_win = shtctl->sheets[j];
					keywin_on(key_win);
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
				if (i == 256 + 0x5b) {
					key_command |= 1;
				}
				if (i == 256 + 0xdb) {
					key_command &= ~1;
				}
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
					new_mouse_x = mouse_x;
					new_mouse_y = mouse_y;
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
											keywin_off(key_win);
											key_win = sheet;
											keywin_on(key_win);
										}
										if (3 <= x && x < sheet->bxsize - 3 && 3 <= y && y < 21) {
											mmx = mouse_x;
											mmy = mouse_y;
											mmx2 = sheet->vx0;
											new_window_y = sheet->vy0;
										}
										if (sheet->bxsize - 21 <= x && x < sheet->bxsize - 5 && 5 <= y && y < 19) {
											if ((sheet->flags & 0x10) != 0) {
												task = sheet->task;
												console_putstr(task->console, "\nBreak(mouse) :\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
												task_run(task, -1, 0);
											} else if ((sheet->flags & 0x20) != 0) {
												sheet_updown(sheet, -1);
												keywin_off(key_win);
												key_win = shtctl->sheets[shtctl->top - 1];
												keywin_on(key_win);
												io_cli();
												fifo32_put(&key_win->task->fifo, CONSOLE_SHUT);
												io_sti();
											}
										}
										break;
									}
								}
							}
						} else {
							x = mouse_x - mmx;
							y = mouse_y - mmy;
							new_window_x = (mmx2 + x + 2) & ~3;
							new_window_y = new_window_y + y;
							mmy = mouse_y;
						}
					} else {
						mmx = -1;
						if (new_window_x != 0x7fffffff) {
							sheet_slide(sheet, new_window_x, new_window_y);
							new_window_x = 0x7fffffff;
						}
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 50, 0, COL8_FFFFFF, COL8_000000, s);
				}
			} else if (768 <= i && i <= 1023) {
				close_console(shtctl->sheets0 + (i - 768));
			} else if (1024 <= i && i <= 2023) {
				close_console_task(taskctl->tasks + (i - 1024));
			} else if (2024 <= i && i <= 2279) {
				sheet2 = shtctl->sheets0 + (i - 2024);
				memman_free_4k(memman, (int) sheet2->buf, CONSOLE_WIDTH * CONSOLE_HEIGHT);
				sheet_free(sheet2);
			}
		}
	}
}

void keywin_on(struct SHEET *key_win)
{
	change_window_title(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, CONSOLE_ON);
	}
}

void keywin_off(struct SHEET *key_win)
{
	change_window_title(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, CONSOLE_OFF);
	}
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sheet = sheet_alloc(shtctl);
	unsigned char *buf = (unsigned char *)memman_alloc_4k(memman, CONSOLE_WIDTH * CONSOLE_HEIGHT);
	sheet_setbuf(sheet, buf, CONSOLE_WIDTH, CONSOLE_HEIGHT, -1);
	make_window(buf, CONSOLE_WIDTH, CONSOLE_HEIGHT, "Terminal", 0);
	make_textbox8(sheet, CHAR_WIDTH, WINDOW_TITLE_HEIGHT, CONSOLE_TEXTBOX_WIDTH, CONSOLE_TEXTBOX_HEIGHT, COL8_000000);
	sheet->task = open_console_task(sheet, memtotal);
	sheet->flags |= 0x20;
	return sheet;
}

struct TASK* open_console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = task_alloc();
	int console_fifo = (int *)memman_alloc_4k(memman, 128 * 4);
	task->console_stack = memman_alloc_4k(memman, 64 * 1024);
	task->tss.esp = task->console_stack + 64 * 1024 - 12;
	task->tss.eip = (int)&console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *) (task->tss.esp + 4)) = (int) sheet;
	*((int *) (task->tss.esp + 8)) = (int) memtotal;
	task_run(task, 2, 2);
	fifo32_init(&task->fifo, 128, console_fifo, task);
	return task;
}

void close_console_task(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->console_stack, 64 * 1024);
	memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
	task->flags = 0;
	return;
}

void close_console(struct SHEET* sheet)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = sheet->task;
	memman_free_4k(memman, (int) sheet->buf, CONSOLE_WIDTH * CONSOLE_HEIGHT);
	sheet_free(sheet);
	close_console_task(task);
	return;
}
