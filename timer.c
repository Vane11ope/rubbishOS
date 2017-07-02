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

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	++timerctl.timernum;
	if (timerctl.timernum == 1) {
		timerctl.firsttimer = timer;
		timer->next = 0;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	t = timerctl.firsttimer;
	if (timer->timeout <= t->timeout) {
		timerctl.firsttimer = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	for (;;) {
		s = t;
		t = t->next;
		if (t == 0) {
			break;
		}
		if (timer->timeout <= t->timeout) {
			s->next = timer;
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
	s->next = timer;
	timer->next = 0;
	io_store_eflags(e);
	return;
}

void inthandler20(int *esp)
{
	int i;
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);
	++timerctl.count;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.firsttimer;
	for (i = 0; i < timerctl.timernum; ++i) {
		if (timer->timeout <= timerctl.count) {
			timer->flags = TIMER_FLAGS_ALLOC;
			fifo32_put(timer->fifo, timer->data);
			timer = timer->next;
			continue;
		}
		break;
	}
	timerctl.timernum -= i;
	timerctl.firsttimer = timer;
	if (timerctl.timernum > 0) {
		timerctl.next = timer->timeout;
	} else {
		timerctl.next = 0xffffffff;
	}
	return;
}
