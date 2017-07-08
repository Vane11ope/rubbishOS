#include "bootpack.h"
#define ADR_GDT           0x00270000
#define AR_TSS32          0x0089
#define TASK_INIT         0
#define TASK_ALLOC        1
#define TASK_RUNNING      2
#define PRIORITY_NOCHANGE 0

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	for (i = 0; i < MAX_TASKS; ++i) {
		taskctl->tasks0[i].flags = TASK_INIT;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = TASK_RUNNING;
	task->priority = 2;
	taskctl->activetasks = 1;
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for(i = 0; i < MAX_TASKS; ++i) {
		if (taskctl->tasks0[i].flags == TASK_INIT) {
			task = &taskctl->tasks0[i];
			task->flags = TASK_ALLOC;
			task->tss.eflags = 0x00000202;
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0;
}

void task_run(struct TASK *task, int priority)
{
	if (priority > PRIORITY_NOCHANGE) {
		task->priority = priority;
	}
	if (task->flags != TASK_RUNNING) {
		task->flags = TASK_RUNNING;
		taskctl->tasks[taskctl->activetasks++] = task;
	}
	return;
}

void task_switch(void)
{
	struct TASK *task;
	++taskctl->now;
	if (taskctl->now == taskctl->activetasks) {
		taskctl->now = 0;
	}
	task = taskctl->tasks[taskctl->now];
	timer_settime(task_timer, task->priority);
	if (taskctl->activetasks >= 2) {
		farjmp(0, task->sel);
	}
	return;
}

void task_sleep(struct TASK *task)
{
	int i;
	char ts = 0;
	if (task->flags == TASK_RUNNING) {
		for (i = 0; i < taskctl->activetasks; ++i) {
			if (taskctl->tasks[i] == task) {
				if (i == taskctl->now) {ts = 1;}
				else if (i < taskctl->now) {--taskctl->now;}
				break;
			}
		}
		--taskctl->activetasks;
		for (; i < taskctl->activetasks; ++i) {
			taskctl->tasks[i] = taskctl->tasks[i + 1];
		}
		task->flags = TASK_ALLOC;
		if (ts != 0) {
			if (taskctl->now >= taskctl->activetasks) {
				taskctl->now = 0;
			}
			farjmp(0, taskctl->tasks[taskctl->now]->sel);
		}
	}
	return;
}
