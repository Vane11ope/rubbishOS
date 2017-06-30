#include "bootpack.h"
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040
#define TIMER_FLAGS_INIT  0
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMERCTL timerctl;

void init_pit(void)
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.next = 0xffffffff;
	for (i = 0; i < MAX_TIMER; ++i) {
		timerctl.timer[i].flags = TIMER_FLAGS_INIT;
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; ++i) {
		if (timerctl.timer[i].flags == TIMER_FLAGS_INIT) {
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;
}

void timer_free(struct TIMER *timer)
{
	timer->flags = TIMER_FLAGS_INIT;
	return;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	if (timerctl.next > timeout) {
		timerctl.next = timeout;
	}
	return;
}

void inthandler20(int *esp)
{
	int i;
	io_out8(PIC0_OCW2, 0x60);
	++timerctl.count;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timerctl.next = 0xffffffff;
	for (i = 0; i < MAX_TIMER; ++i) {
		struct TIMER timer = timerctl.timer[i];
		if (timer.flags == TIMER_FLAGS_USING) {
			if (timer.timeout <= timerctl.count) {
				timer.flags = TIMER_FLAGS_ALLOC;
				fifo8_put(timer.fifo, timer.data);
			} else {
				if (timerctl.next > timer.timeout) {
					timerctl.next = timer.timeout;
				}
			}
		}
	}
	return;
}
