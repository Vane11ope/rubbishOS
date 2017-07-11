#include "bootpack.h"
#define CONSOLE_ON  2
#define CONSOLE_OFF 3

void console_task(struct SHEET *sheet)
{
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, x, y, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;
	char s[2];

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">");
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
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, 28, cursor_x + 7, 43);
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
					if (cursor_y < 28 + 112) {
						cursor_y += 16;
					} else {
						for (y = 28; y < 28 + 112; ++y) {
							for (x = 8; x < 8 + 240; ++x) {
								sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
							}
						}
						for (y = 28 + 112; y < 28 + 128; ++y) {
							for (x = 8; x < 8 + 240; ++x) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
					}
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">");
					cursor_x = 16;
				} else {
					if (cursor_x < 240) {
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s);
						cursor_x += 8;
					}
				}
			}
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}
