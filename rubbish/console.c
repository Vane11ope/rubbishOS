#include "bootpack.h"
#include "string.h"
#define ADR_GDT      0x00270000
#define ADR_DISKIMG  0x00100000
#define CONSOLE_ON   2
#define CONSOLE_OFF  3
#define CONSOLE_SHUT 4

extern struct TASKCTL *taskctl;
const short CONSOLE_TEXTBOX_WIDTH = CONSOLE_WIDTH - (CHAR_WIDTH * 2);
const short CONSOLE_TEXTBOX_HEIGHT = CONSOLE_HEIGHT - 37;

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct CONSOLE console;
	struct FILEHANDLE fhandle[8];
	int i;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	unsigned char *nihongo = (char *)*((int *)0x0fe8);
	char cmdline[64];

	console.sheet = sheet;
	console.cursor_x = CHAR_WIDTH;
	console.cursor_y = WINDOW_TITLE_HEIGHT;
	console.cursor_color = -1;
	task->console = &console;
	task->cmdline = cmdline;

	if (console.sheet != 0) {
		console.timer = timer_alloc();
		timer_init(console.timer, &task->fifo, 1);
		timer_settime(console.timer, 50);
	}

	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	for (i = 0; i < 8; ++i) {
		fhandle[i].buf = 0;
	}
	if (nihongo[4096] != 0xff) { task->langmode = 1; }
	else { task->langmode = 0; }
	task->fhandle = fhandle;
	task->fat = fat;
	task->langbyte = 0;

	console_putchar(&console, '>', 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && console.sheet != 0) {
				if (i != 0) {
					timer_init(console.timer, &task->fifo, 0);
					if (console.cursor_color >= 0) { console.cursor_color = COL8_FFFFFF; }
				} else {
					timer_init(console.timer, &task->fifo, 1);
					if (console.cursor_color >= 0) { console.cursor_color = COL8_000000; }
				}
				timer_settime(console.timer, 50);
			}
			if (i == CONSOLE_ON) {
				console.cursor_color = COL8_FFFFFF;
			}
			if (i == CONSOLE_OFF) {
				if (console.sheet != 0) {
					boxfill8(sheet->buf, sheet->bxsize, COL8_000000, console.cursor_x, WINDOW_TITLE_HEIGHT, console.cursor_x + 7, 43);
				}
				console.cursor_color = -1;
			}
			if (i == CONSOLE_SHUT) {
				shut(&console, fat);
			}
			if (256 <= i && i <= 511) {
				switch (i) {
					case 8 + 256: // delete
						if (console.cursor_x > CHAR_WIDTH * 2) {
							console_putchar(&console, ' ', 0);
							console.cursor_x -= CHAR_WIDTH;
						}
						break;
					case 10 + 256: // Enter
						console_putchar(&console, ' ', 0);
						cmdline[console.cursor_x / CHAR_WIDTH -2] = 0;
						console_newline(&console);
						console_command(cmdline, &console, fat, memtotal);
						if (console.sheet == 0) { shut(&console, fat); }
						console_putchar(&console, '>', 1);
						break;
					default:
						if (console.cursor_x < CONSOLE_TEXTBOX_WIDTH) {
							cmdline[console.cursor_x / CHAR_WIDTH - 2] = i - 256;
							console_putchar(&console, i - 256, 1);
						}
				}
			} else if (i == 1111) {
				console_ctrl_l(&console);
				console.cursor_y = WINDOW_TITLE_HEIGHT;
			}
			if (console.sheet != 0) {
				if (console.cursor_color >= 0) {
					boxfill8(console.sheet->buf, console.sheet->bxsize, console.cursor_color, console.cursor_x, console.cursor_y, console.cursor_x + CHAR_WIDTH - 1, console.cursor_y + CHAR_HEIGHT - 1);
				}
				sheet_refresh(console.sheet, console.cursor_x, console.cursor_y, console.cursor_x + CHAR_WIDTH, console.cursor_y + CHAR_HEIGHT);
			}
		}
	}
}

void console_newline(struct CONSOLE *console)
{
	int x, y;
	struct TASK *task = task_now();
	short threshold = WINDOW_TITLE_HEIGHT + (CONSOLE_TEXTBOX_HEIGHT - CHAR_HEIGHT);
	if (console->cursor_y < threshold) {
		console->cursor_y += CHAR_HEIGHT;
	} else {
		if (console->sheet != 0) {
			for (y = WINDOW_TITLE_HEIGHT; y < threshold; ++y) {
				for (x = CHAR_WIDTH; x < CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH; ++x) {
					console->sheet->buf[x + y * console->sheet->bxsize] = console->sheet->buf[x + (y + CHAR_HEIGHT) * console->sheet->bxsize];
				}
			}
			for (y = threshold; y < threshold + CHAR_HEIGHT; ++y) {
				for (x = CHAR_WIDTH; x < CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH; ++x) {
					console->sheet->buf[x + y * console->sheet->bxsize] = COL8_000000;
				}
			}
			sheet_refresh(console->sheet, CHAR_WIDTH, WINDOW_TITLE_HEIGHT, CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH, WINDOW_TITLE_HEIGHT + CONSOLE_TEXTBOX_HEIGHT);
		}
	}
	console->cursor_x = CHAR_WIDTH;
	if (task->langmode == 1 && task->langbyte != 0) { console->cursor_x = CHAR_WIDTH * 2; }
	return;
}

void console_ctrl_l(struct CONSOLE *console)
{
	int x, y;
	for (y = WINDOW_TITLE_HEIGHT; y < WINDOW_TITLE_HEIGHT + CHAR_HEIGHT; ++y) {
		for (x = CHAR_WIDTH; x < CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH; ++x) {
			console->sheet->buf[x + y * console->sheet->bxsize] = console->sheet->buf[x + (console->cursor_y + (y - WINDOW_TITLE_HEIGHT))  * console->sheet->bxsize];
		}
	}
	for (y = WINDOW_TITLE_HEIGHT + CHAR_HEIGHT; y < WINDOW_TITLE_HEIGHT + CONSOLE_TEXTBOX_HEIGHT; ++y) {
		for (x = CHAR_WIDTH; x < CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH; ++x) {
			console->sheet->buf[x + y * console->sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(console->sheet, CHAR_WIDTH, WINDOW_TITLE_HEIGHT, CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH, WINDOW_TITLE_HEIGHT + CONSOLE_TEXTBOX_HEIGHT);
}

void console_putchar(struct CONSOLE *console, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	switch (s[0]) {
		case 0x09:
			for (;;) {
				if (console->sheet != 0) {
					putfonts8_asc_sht(console->sheet, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, " ");
				}
				console->cursor_x += CHAR_WIDTH;
				if (console->cursor_x == CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH) {
					console_newline(console);
				}
				if (((console->cursor_x - CHAR_WIDTH) & 0x1f) == 0) {
					break;
				}
			}
			break;
		case 0x0a:
			console_newline(console);
			break;
		case 0x0d:
			break;
		default:
			if (console->sheet != 0) {
				putfonts8_asc_sht(console->sheet, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, s);
			}
			if (move != 0) {
				console->cursor_x += CHAR_WIDTH;
				if (console->cursor_x == CHAR_WIDTH + CONSOLE_TEXTBOX_WIDTH) {
					console_newline(console);
				}
			}
			break;
	}
	return;
}

void console_putstr(struct CONSOLE *console, char *s)
{
	for (; *s != 0; ++s) {
		console_putchar(console, *s, 1);
	}
	return;
}

void console_putstr_with_length(struct CONSOLE *console, char *s, int length)
{
	int i;
	for (i = 0; i < length; ++i) {
		console_putchar(console, s[i], 1);
	}
	return;
}

void console_command(char *cmdline, struct CONSOLE *console, int *fat, unsigned int memtotal)
{
	if (strcmp(cmdline, "mem") == 0 && console->sheet != 0) {
		mem(console, memtotal);
	} else if (strcmp(cmdline, "ls") == 0 && console->sheet != 0) {
		ls(console);
	} else if (strncmp(cmdline, "start ", 6) == 0) {
		start(console, cmdline, memtotal);
	} else if (strncmp(cmdline, "ncst ", 5) == 0) {
		ncst(console, cmdline, memtotal);
	} else if (strncmp(cmdline, "langmode ", 9) == 0) {
		langmode(console, cmdline);
	} else if (cmdline[0] != 0) {
		if ( app(console, fat, cmdline) == 0) {
			console_putstr(console, "Bad command.\n\n");
		}
	}
	return;
}

void mem(struct CONSOLE *console, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[30];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	console_putstr(console, s);
	return;
}

void ls(struct CONSOLE *console)
{
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int x, y;
	char s[30];
	for (x = 0; x < 224; ++x) {
		if (finfo[x].name[0] == 0x00) { break; }
		if ((finfo[x].type & 0x18) == 0) {
			sprintf(s, "filename.ext %7d\n", finfo[x].size);
			for (y = 0; y < 8; ++y) {
				s[y] = finfo[x].name[y];
			}
			s[9] = finfo[x].ext[0];
			s[10] = finfo[x].ext[1];
			s[11] = finfo[x].ext[2];
			console_putstr(console, s);
		}
	}
	console_newline(console);
	return;
}

void start(struct CONSOLE *console, char *cmdline, int memtotal)
{
	struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0x0fe4);
	struct SHEET *sheet = open_console(shtctl, memtotal);
	struct FIFO32 *fifo = &sheet->task->fifo;
	int i;
	sheet_slide(sheet, 200, 16);
	sheet_updown(sheet, shtctl->top);
	for(i = 6; cmdline[i] != 0; ++i) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	console_newline(console);
	return;
}

void ncst(struct CONSOLE *console, char *cmdline, int memtotal)
{
	struct TASK *task = open_console_task(0, memtotal);
	struct FIFO32 *fifo = &task->fifo;
	int i;
	for (i = 5; cmdline[i] != 0; ++i) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);
	console_newline(console);
	return;
}

void shut(struct CONSOLE *console, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = task_now();
	struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0x0fe4);
	struct FIFO32 *fifo = (struct FIFO32 *)*((int *)0x0fec);
	if (console->sheet != 0) { timer_cancel(console->timer); }
	memman_free_4k(memman, (int )fat, 4 * 2880);
	io_cli();
	if (console->sheet != 0) {
		fifo32_put(fifo, console->sheet - shtctl->sheets0 + 768);
	} else {
		fifo32_put(fifo, task - taskctl->tasks + 1024);
	}
	io_sti();
	for (;;) {
		task_sleep(task);
	}
}

void langmode(struct CONSOLE *console, char *cmdline)
{
	struct TASK *task = task_now();
	unsigned char mode = cmdline[9] - '0';
	if (mode <= 2) { task->langmode = mode; }
	else { console_putstr(console, "mode number error.\n"); }
	console_newline(console);
	return;
}

int app(struct CONSOLE *console, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct TASK *task = task_now();
	struct SHTCTL *shtctl;
	struct SHEET *sheet;
	char name[18], *p, *q;
	int i, segsize, datasize, esp, datarub, appsize;

	for (i = 0; i < 13; ++i) {
		if (cmdline[i] <= ' ') { break; }
		name[i] = cmdline[i];
	}
	name[i] = 0;
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		name[i] = '.';
		name[i + 1] = 'R';
		name[i + 2] = 'U';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}
	if (finfo != 0) {
		appsize = finfo->size;
		p = file_loadfile2(finfo->cluster_no, &appsize, fat);
		file_loadfile(finfo->cluster_no, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		if (appsize >= 36 && strncmp(p + 4, "main", 4) == 0 && *p ==  0x00) {
			segsize = *((int *) (p + 0x0000));
			esp = *((int *) (p + 0x000c));
			datasize = *((int *) (p + 0x0010));
			datarub = *((int *) (p + 0x0014));
			q = (char *)memman_alloc_4k(memman, segsize);
			task->ds_base = (int)q;
			set_segmdesc(task->ldt + 0, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
			set_segmdesc(task->ldt + 1, segsize - 1, (int)q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datasize; ++i) {
				q[esp + i] = p[datarub + i];
			}
			start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));
			for (i = 0; i < 8; ++i) {
				if (task->fhandle[i].buf != 0) {
					memman_free_4k(memman, (int)task->fhandle[i].buf, task->fhandle[i].size);
					task->fhandle[i].buf = 0;
				}
			}
			shtctl = (struct SHTCTL *) *((int *)0x0fe4);
			for (i = 0; i < MAX_SHEETS; ++i) {
				sheet = &(shtctl->sheets0[i]);
				if ((sheet->flags & 0x11) == 0x11 && sheet->task == task) {
					sheet_free(sheet);
				}
			}
			timer_cancelall(&task->fifo);
			memman_free_4k(memman, (int) q, segsize);
			task->langbyte = 0;
		} else {
			console_putstr(console, ".rub file format error.\n");
		}
		memman_free_4k(memman, (int) p, appsize);
		console_newline(console);
		return 1;
	}
	return 0;
}

int rub_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct SHTCTL* shtctl = (struct SHTCTL *)(*(int *) 0xfe4);
	struct TASK *task = task_now();
	struct CONSOLE *console = task->console;
	struct SHEET *sheet;
	struct FIFO32 *sys_fifo = (struct FIFO32 *)*((int *)0x0fec);
	struct FILEINFO *finfo;
	struct FILEHANDLE *fh;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	int *reg = &eax + 1; /* rewrite the values assigned to each register */
	int i, ds_base = task->ds_base;
	char s[30];
	switch (edx) {
		case 1:
			console_putchar(console, eax & 0xff, 1);
			break;
		case 2:
			console_putstr(console, (char *)ebx + ds_base);
			break;
		case 3:
			console_putstr_with_length(console, (char *)ebx + ds_base, ecx);
			break;
		case 4:
			return &(task->tss.esp0);
		case 5:
			sheet = sheet_alloc(shtctl);
			sheet->task = task;
			sheet->flags |= 0x10;
			sheet_setbuf(sheet, (char *)ebx + ds_base, esi, edi, eax);
			make_window((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
			sheet_slide(sheet, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
			sheet_updown(sheet, shtctl->top);
			reg[7] = (int)sheet;
			break;
		case 6:
			sheet = (struct SHEET *)(ebx & 0xfffffffe);
			putfonts8_asc(sheet->buf, sheet->bxsize, esi, edi, eax, (char *) ebp + ds_base);
			if ((ebx & 1) == 0) {
				sheet_refresh(sheet, esi, edi, esi + ecx * CHAR_WIDTH, edi + CHAR_HEIGHT);
			}
			break;
		case 7:
			sheet = (struct SHEET *)(ebx & 0xfffffffe);
			boxfill8(sheet->buf, sheet->bxsize, ebp, eax, ecx, esi, edi);
			if ((ebx & 1) == 0) {
				sheet_refresh(sheet, eax, ecx, esi + 1, edi + 1);
			}
			break;
		case 8:
			memman_init((struct MEMMAN *)(ebx + ds_base));
			ecx &= 0xfffffff0;
			memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
			break;
		case 9:
			ecx = (ecx + 0x0f) & 0xfffffff0;
			reg[7] = memman_alloc((struct MEMMAN *)(ebx + ds_base), ecx);
			break;
		case 10:
			ecx = (ecx + 0x0f) & 0xfffffff0;
			memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
			break;
		case 11:
			sheet = (struct SHEET *)(ebx & 0xfffffffe);
			sheet->buf[sheet->bxsize * edi + esi] = eax;
			if ((ebx & 1) == 0) {
				sheet_refresh(sheet, esi, edi, esi + 1, edi + 1);
			}
			break;
		case 12:
			sheet = (struct SHEET *)ebx;
			sheet_refresh(sheet, eax, ecx, esi, edi);
			break;
		case 13:
			sheet = (struct SHEET *)(ebx & 0xfffffffe);
			drawline(sheet, eax, ecx, esi, edi, ebp);
			if ((ebx & 1) == 0) {
				if (eax > esi) {
					i = eax;
					eax = esi;
					esi = i;
				}
				if (ecx > edi) {
					i = ecx;
					ecx = edi;
					edi = i;
				}
				sheet_refresh(sheet, eax, ecx, esi + 1, edi + 1);
			}
			break;
		case 14:
			sheet_free((struct SHEET *) ebx);
			break;
		case 15:
			for (;;) {
				io_cli();
				if (fifo32_status(&task->fifo) == 0) {
					if (eax != 0) {
						task_sleep(task);
					} else {
						io_sti();
						reg[7] = -1;
						return 0;
					}
				}
				i = fifo32_get(&task->fifo);
				io_sti();
				if (i <= 1 && console->sheet != 0) {
					timer_init(console->timer, &task->fifo, 1);
					timer_settime(console->timer, 50);
				}
				if (i == CONSOLE_ON) {
					console->cursor_color = COL8_FFFFFF;
				}
				if (i == CONSOLE_OFF) {
					console->cursor_color = -1;
				}
				if (i == CONSOLE_SHUT) {
					timer_cancel(console->timer);
					io_cli();
					fifo32_put(sys_fifo, console->sheet - shtctl->sheets0 + 2024);
					console->sheet = 0;
					io_sti();
				}
				if (i >= 256) {
					reg[7] = i - 256;
					return 0;
				}
			}
			break;
		case 16:
			reg[7] = (int) timer_alloc();
			((struct TIMER *)reg[7])->flags2 = 1;
			break;
		case 17:
			timer_init((struct TIMER *)ebx, &task->fifo, eax + 256);
			break;
		case 18:
			timer_settime((struct TIMER *)ebx, eax);
			break;
		case 19:
			timer_free((struct TIMER *)ebx);
			break;
		case 20:
			if (eax == 0) {
				i = io_in8(0x61);
				io_out8(0x61, i & 0x0d);
			} else {
				i = 1193180000 / eax;
				io_out8(0x43, 0xb6);
				io_out8(0x42, i & 0xff);
				io_out8(0x42, i >> 8);
				i = io_in8(0x61);
				io_out8(0x61, (i | 0x03) & 0x0f);
			}
			break;
		case 21:
			for (i = 0; i < 8; ++i) {
				if (task->fhandle[i].buf == 0) { break; }
			}
			fh = &task->fhandle[i];
			reg[7] = 0;
			if (i < 8) {
				finfo = file_search((char *)ebx + ds_base, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
				if (finfo != 0) {
					reg[7] = (int)fh;
					fh->buf = file_loadfile2(finfo->cluster_no, &fh->size, task->fat);
					fh->size = finfo->size;
					fh->pos = 0;
				}
			}
			break;
		case 22:
			fh = (struct FILEHANDLE *)eax;
			memman_free_4k(memman, (int)fh->buf, fh->size);
			fh->buf = 0;
			break;
		case 23:
			fh = (struct FILEHANDLE *)eax;
			switch (ecx) {
				case 0:
					fh->pos = ebx;
					break;
				case 1:
					fh->pos += ebx;
					break;
				case 2:
					fh->pos = fh->size + ebx;
					break;
				default:
					console_putstr(console, "ecx is illegal");
					return &(task->tss.esp0);
			}
			fh->pos = max(fh->pos, 0);
			fh->pos = min(fh->pos, fh->size);
			break;
		case 24:
			fh = (struct FILEHANDLE *)eax;
			switch (ecx) {
				case 0:
					reg[7] = fh->size;
					break;
				case 1:
					reg[7] = fh->pos;
					break;
				case 2:
					reg[7] = fh->pos - fh->size;
					break;
				default:
					console_putstr(console, "ecx is illegal");
					return &(task->tss.esp0);
			}
			break;
		case 25:
			fh = (struct FILEHANDLE *)eax;
			for (i = 0; i < ecx; ++i) {
				if (fh->pos == fh->size) { break; }
				*((char *)ebx + ds_base + i) = fh->buf[fh->pos];
				++fh->pos;
			}
			reg[7] = i;
			break;
		case 26:
			i = 0;
			for (;;) {
				*((char *)ebx + ds_base + i) = task->cmdline[i];
				if (task->cmdline[i] == 0) { break; }
				if (i >= ecx) { break; }
				++i;
			}
			reg[7] = i;
			break;
		case 27:
			reg[7] = task->langmode;
			break;
		default:
			console_putstr(console, "edx is illegal");
			return &(task->tss.esp0);
	}
	return 0;
}

int inthandler0d(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *console = task->console;
	char s[30];
	console_putstr(console, "\nINT 0D :\nGeneral Protected Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	console_putstr(console, s);
	return &(task->tss.esp0);
}

int inthandler0c(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *console = task->console;
	char s[30];
	console_putstr(console, "\nINT 0D :\nStack Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	console_putstr(console, s);
	return &(task->tss.esp0);
}
