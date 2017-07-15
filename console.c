#include "bootpack.h"
#include "string.h"
#define MEMMAN_ADDR 0x003c0000
#define ADR_DISKIMG 0x00100000
#define CONSOLE_ON  2
#define CONSOLE_OFF 3

const short CONSOLE_TEXTBOX_WIDTH = CONSOLE_WIDTH - 16;
const short CONSOLE_TEXTBOX_HEIGHT = CONSOLE_HEIGHT - 37;

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, x, y, fifobuf[128], cursor_x = 16, cursor_y = WINDOW_TITLE_HEIGHT, cursor_c = -1;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	char s[64], cmdline[64], *p;

	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	putfonts8_asc_sht(sheet, 8, WINDOW_TITLE_HEIGHT, COL8_FFFFFF, COL8_000000, ">");
	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) {
				if (i != 0) {
					timer_init(timer, &task->fifo, 0);
					if (cursor_c >= 0) { cursor_c = COL8_FFFFFF; }
				} else {
					timer_init(timer, &task->fifo, 1);
					if (cursor_c >= 0) { cursor_c = COL8_000000; }
				}
				timer_settime(timer, 50);
			}
			if (i == CONSOLE_ON) {
				cursor_c = COL8_FFFFFF;
			}
			if (i == CONSOLE_OFF) {
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, WINDOW_TITLE_HEIGHT, cursor_x + 7, 43);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) {
				if (i == 8 + 256) {
					if (cursor_x > 16) {
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ");
						cursor_x -= 8;
					}
				} else if (i == 10 + 256) { // Enter
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ");
					cmdline[cursor_x / 8 - 2] = 0;
					cursor_y = console_newline(cursor_y, sheet);
					if (strcmp(cmdline, "mem") == 0) {
						sprintf(s, "total   %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s);
						cursor_y = console_newline(cursor_y, sheet);
						sprintf(s, "free %dKB", memman_total(memman) / 1024 );
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s);
						cursor_y = console_newline(cursor_y, sheet);
						cursor_y = console_newline(cursor_y, sheet);
					} else if (strcmp(cmdline, "ls") == 0) {
						for (x = 0; x < 224; ++x) {
							if (finfo[x].name[0] == 0x00) { break; }
							if ((finfo[x].type & 0x18) == 0) {
								sprintf(s, "filename.ext %7d", finfo[x].size);
								for (y = 0; y < 8; ++y) {
									s[y] = finfo[x].name[y];
								}
								s[9] = finfo[x].ext[0];
								s[10] = finfo[x].ext[1];
								s[11] = finfo[x].ext[2];
								putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s);
								cursor_y = console_newline(cursor_y, sheet);
							}
						}
						cursor_y = console_newline(cursor_y, sheet);
					} else if (strncmp(cmdline, "cat ", 4) == 0) {
						for (y = 0; y < 11; ++y) { s[y] = ' '; }
						y = 0;
						for (x = 4; y < 11 && cmdline[x] != 0; ++x) {
							if (cmdline[x] == '.' && y <= 8) {
								y = 8;
							} else {
								s[y] = cmdline[x];
								if ('a' <= s[y] && s[y] <= 'z') {
									s[y] -= 0x20;
								}
								++y;
							}
						}
						for (x = 0; x < 224; ) {
							if (finfo[x].name[0] == 0x00) {
								break;
							}
							if ((finfo[x].type & 0x18) == 0) {
								for (y = 0; y < 11; ++y) {
									if (finfo[x].name[y] != s[y]) {
										goto type_next_file;
									}
								}
								break;
							}
type_next_file:
							++x;
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							file_loadfile(finfo[x].cluster_no, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							cursor_x = 8;
							for (y = 0; y < finfo[x].size; ++y) {
								s[0] = p[y];
								s[1] = 0;
								if (s[0] == 0x09) {
									for (;;) {
										putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ");
										cursor_x += 8;
										if (cursor_x == 8 + CONSOLE_TEXTBOX_WIDTH) {
											cursor_x = 8;
											cursor_y = console_newline(cursor_y, sheet);
										}
										if (((cursor_x - 8) & 0x1f) == 0) {
											break;
										}
									}
								} else if (s[0] == 0x0a) {
									cursor_x = 8;
									cursor_y = console_newline(cursor_y, sheet);
								} else if (s[0] == 0x0d) {
								} else {
									putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s);
									cursor_x += 8;
									if (cursor_x == 8 + CONSOLE_TEXTBOX_WIDTH) {
										cursor_x = 8;
										cursor_y = console_newline(cursor_y, sheet);
									}
								}
							}
							memman_free_4k(memman, (int) p, finfo[x].size);
						} else {
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.");
							cursor_y = console_newline(cursor_y, sheet);
						}
						cursor_y = console_newline(cursor_y, sheet);
					} else if (cmdline[0] != 0) {
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.");
						cursor_y = console_newline(cursor_y, sheet);
						cursor_y = console_newline(cursor_y, sheet);
					}
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">");
					cursor_x = 16;
				} else {
					if (cursor_x < CONSOLE_TEXTBOX_WIDTH) {
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s);
						cursor_x += 8;
					}
				}
			} else if (i == 1111) {
				console_ctrl_l(cursor_y, sheet);
				cursor_y = WINDOW_TITLE_HEIGHT;
			}
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

int console_newline(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	short threshold = WINDOW_TITLE_HEIGHT + (CONSOLE_TEXTBOX_HEIGHT - 16);
	if (cursor_y < threshold) {
		cursor_y += 16;
	} else {
		for (y = WINDOW_TITLE_HEIGHT; y < threshold; ++y) {
			for (x = 8; x < 8 + CONSOLE_TEXTBOX_WIDTH; ++x) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = threshold; y < threshold + 16; ++y) {
			for (x = 8; x < 8 + CONSOLE_TEXTBOX_WIDTH; ++x) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, WINDOW_TITLE_HEIGHT, 8 + CONSOLE_TEXTBOX_WIDTH, WINDOW_TITLE_HEIGHT + CONSOLE_TEXTBOX_HEIGHT);
	}
	return cursor_y;
}

void console_ctrl_l(int cursor_y, struct SHEET *sheet)
{
	int x, y;
	for (y = WINDOW_TITLE_HEIGHT; y < 44; ++y) {
		for (x = 8; x < 8 + CONSOLE_TEXTBOX_WIDTH; ++x) {
			sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (cursor_y + (y - WINDOW_TITLE_HEIGHT))  * sheet->bxsize];
		}
	}
	for (y = 44; y < 28 + CONSOLE_TEXTBOX_HEIGHT; ++y) {
		for (x = 8; x < 8 + CONSOLE_TEXTBOX_WIDTH; ++x) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, WINDOW_TITLE_HEIGHT, 8 + CONSOLE_TEXTBOX_WIDTH, WINDOW_TITLE_HEIGHT + CONSOLE_TEXTBOX_HEIGHT);
}
