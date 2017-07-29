#include "bootpack.h"
#include "string.h"
#define MEMMAN_ADDR  0x003c0000
#define ADR_GDT      0x00270000
#define ADR_DISKIMG  0x00100000
#define CONSOLE_ON   2
#define CONSOLE_OFF  3

const short CONSOLE_TEXTBOX_WIDTH = CONSOLE_WIDTH - (CHAR_WIDTH * 2);
const short CONSOLE_TEXTBOX_HEIGHT = CONSOLE_HEIGHT - 37;

void console_task(struct SHEET *sheet, unsigned int memtotal)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	struct CONSOLE console;
	int i, x, y, fifobuf[128];
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	char s[64], cmdline[64], *p;

	console.sheet = sheet;
	console.cursor_x = CHAR_WIDTH;
	console.cursor_y = WINDOW_TITLE_HEIGHT;
	console.cursor_color = -1;
	(*((int *) 0x0fec)) = (int) &console;

	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	console_putchar(&console, '>', 1);

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
					if (console.cursor_color >= 0) { console.cursor_color = COL8_FFFFFF; }
				} else {
					timer_init(timer, &task->fifo, 1);
					if (console.cursor_color >= 0) { console.cursor_color = COL8_000000; }
				}
				timer_settime(timer, 50);
			}
			if (i == CONSOLE_ON) {
				console.cursor_color = COL8_FFFFFF;
			}
			if (i == CONSOLE_OFF) {
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, console.cursor_x, WINDOW_TITLE_HEIGHT, console.cursor_x + 7, 43);
				console.cursor_color = -1;
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
			if (console.cursor_color >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, console.cursor_color, console.cursor_x, console.cursor_y, console.cursor_x + CHAR_WIDTH - 1, console.cursor_y + CHAR_HEIGHT - 1);
			}
			sheet_refresh(sheet, console.cursor_x, console.cursor_y, console.cursor_x + CHAR_WIDTH, console.cursor_y + CHAR_HEIGHT);
		}
	}
}

void console_newline(struct CONSOLE *console)
{
	int x, y;
	short threshold = WINDOW_TITLE_HEIGHT + (CONSOLE_TEXTBOX_HEIGHT - CHAR_HEIGHT);
	if (console->cursor_y < threshold) {
		console->cursor_y += CHAR_HEIGHT;
	} else {
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
	console->cursor_x = CHAR_WIDTH;
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
				putfonts8_asc_sht(console->sheet, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, " ");
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
			putfonts8_asc_sht(console->sheet, console->cursor_x, console->cursor_y, COL8_FFFFFF, COL8_000000, s);
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
	if (strcmp(cmdline, "mem") == 0) {
		mem(console, memtotal);
	} else if (strcmp(cmdline, "ls") == 0) {
		ls(console);
	} else if (strncmp(cmdline, "cat ", 4) == 0) {
		cat(console, fat, cmdline);
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

void cat(struct CONSOLE *console, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 4, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	char *p;
	int i;
	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->cluster_no, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		console_putstr_with_length(console, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		console_putstr(console, "File not found.\n");
	}
	console_newline(console);
	return;
}

int app(struct CONSOLE *console, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct TASK *task = task_now();
	char name[18], *p, *q;
	int i, segsize, datasize, esp, datarub;

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
		p = (char *)memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->cluster_no, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		if (finfo->size >= 36 && strncmp(p + 4, "main", 4) == 0 && *p ==  0x00) {
			segsize = *((int *) (p + 0x0000));
			esp = *((int *) (p + 0x000c));
			datasize = *((int *) (p + 0x0010));
			datarub = *((int *) (p + 0x0014));
			q = (char *)memman_alloc_4k(memman, segsize);
			(*((int *)0xfe8)) = (int)q;
			set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
			set_segmdesc(gdt + 1004, segsize - 1, (int) q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datasize; ++i) {
				q[esp + i] = p[datarub + i];
			}
			start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
			memman_free_4k(memman, (int) q, segsize);
		} else {
			console_putstr(console, ".rub file format error.\n");
		}
		memman_free_4k(memman, (int) p, finfo->size);
		console_newline(console);
		return 1;
	}
	return 0;
}

int rub_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct CONSOLE *console = (struct CONSOLE *)(*(int *) 0xfec);
	struct SHTCTL* shtctl = (struct SHTCTL *)(*(int *) 0xfe4);
	struct TASK *task = task_now();
	struct SHEET *sheet;
	int *reg = &eax + 1; /* rewrite the values assigned to each register */
	int ds_base = (*((int *)0xfe8));
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
			sheet_setbuf(sheet, (char *)ebx + ds_base, esi, edi, eax);
			make_window((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
			sheet_slide(sheet, 700, 200);
			sheet_updown(sheet, 5);
			reg[7] = (int)sheet;
			break;
		case 6:
			sheet = (struct SHEET *)(ebx & 0xfffffffe);
			putfonts8_asc(sheet->buf, sheet->bxsize, esi, edi, eax, (char *) ebp + ds_base);
			if ((ebx & 1) == 0) {
				sheet_refresh(sheet, esi, edi, esi + ecx * 8, edi + 16);
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
				sheet_refresh(sheet, eax, ecx, esi + 1, edi + 1);
			}
			break;
		default:
			console_putstr(console, "edx is illegal");
			return &(task->tss.esp0);
	}
	return 0;
}

int inthandler0d(int *esp)
{
	struct CONSOLE *console = (struct CONSOLE *)*((int *)0x0fec);
	struct TASK *task = task_now();
	char s[30];
	console_putstr(console, "\nINT 0D :\nGeneral Protected Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	console_putstr(console, s);
	return &(task->tss.esp0);
}

int inthandler0c(int *esp)
{
	struct CONSOLE *console = (struct CONSOLE *)*((int *)0x0fec);
	struct TASK *task = task_now();
	char s[30];
	console_putstr(console, "\nINT 0D :\nStack Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	console_putstr(console, s);
	return &(task->tss.esp0);
}
