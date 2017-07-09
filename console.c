#include "bootpack.h"

void console_task(struct SHEET *sheet)
{
	struct FIFO32 fifo;
	struct TIMER *timer;
	struct TASK *task = task_now();

	int i, fifobuf[128], cursor_x = 8, cursor_c = COL8_000000;
	fifo32_init(&fifo, 128, fifobuf, task);

	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i <= 1) {
				if (i != 0) {
					timer_init(timer, &fifo, 0);
					cursor_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &fifo, 1);
					cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}
