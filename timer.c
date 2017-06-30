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
	timerctl.timernum = 0;
	for (i = 0; i < MAX_TIMER; ++i) {
		timerctl.timer0[i].flags = TIMER_FLAGS_INIT;
	}
	return;
}

struct TIMER *timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; ++i) {
		if (timerctl.timer0[i].flags == TIMER_FLAGS_INIT) {
			timerctl.timer0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer0[i];
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
	int e, i, j;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < timerctl.timernum; ++i) {
		if (timerctl.timer[i]->timeout >= timer->timeout) {
			break;
		}
	}
	for (j = timerctl.timernum; j > i; --j) {
		timerctl.timer[j] = timerctl.timer[j - 1];
	}
	++timerctl.timernum;
	timerctl.timer[i] = timer;
	timerctl.next = timerctl.timer[0]->timeout;
	io_store_eflags(e);
	return;
}

void inthandler20(int *esp)
{
	int i, j;
	io_out8(PIC0_OCW2, 0x60);
	++timerctl.count;
	if (timerctl.next > timerctl.count) {
		return;
	}
	for (i = 0; i < timerctl.timernum; ++i) {
		struct TIMER *timer = timerctl.timer[i];
		if (timer->timeout <= timerctl.count) {
			timer->flags = TIMER_FLAGS_ALLOC;
			fifo8_put(timer->fifo, timer->data);
			continue;
		}
		break;
	}
	timerctl.timernum -= i;
	for (j = 0; j < timerctl.timernum; ++j) {
		timerctl.timer[j] = timerctl.timer[j + i];
	}
	if (timerctl.timernum > 0) {
		timerctl.next = timerctl.timer[0]->timeout;
	} else {
		timerctl.next = 0xffffffff;
	}
	return;
}
