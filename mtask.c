#include "bootpack.h"
#define ADR_GDT           0x00270000
#define AR_TSS32          0x0089
#define TASK_INIT         0
#define TASK_ALLOC        1
#define TASK_RUNNING      2
#define LEVEL_NOCHANGE    -1
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
		taskctl->tasks[i].flags = TASK_INIT;
		taskctl->tasks[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks[i].tss, AR_TSS32);
	}
	for (i = 0; i < MAX_TASKLEVEL; ++i) {
		taskctl->tasklevels[i].activetasks = 0;
		taskctl->tasklevels[i].now = 0;
	}
	task = task_alloc();
	task->flags = TASK_RUNNING;
	task->priority = 2;
	task->level = 0;
	task_add(task);
	task_switchsub();
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
		if (taskctl->tasks[i].flags == TASK_INIT) {
			task = &taskctl->tasks[i];
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

void task_run(struct TASK *task, int level, int priority)
{
	if (level <= LEVEL_NOCHANGE) { level = task->level; }
	if (priority > PRIORITY_NOCHANGE) { task->priority = priority; }
	if (task->flags == TASK_RUNNING && task->level != level) { task_remove(task); }
	if (task->flags != TASK_RUNNING) {
		task->level = level;
		task_add(task);
	}
	taskctl->lv_change = 1;
	return;
}

void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->tasklevels[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->p_tasks[tl->now];
	++tl->now;
	if (tl->now >= tl->activetasks) { tl->now = 0; }
	if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->tasklevels[taskctl->now_lv];
	}
	new_task = tl->p_tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task !=  now_task) {
		farjmp(0, new_task->sel);
	}
	return;
}

void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == TASK_RUNNING) {
		task_remove(task);
		now_task = task_now();
		if (task == now_task) {
			task_switchsub();
			now_task = task_now();
			farjmp(0, now_task->sel);
		}
	}
	return;
}

struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->tasklevels[taskctl->now_lv];
	return tl->p_tasks[tl->now];
}

void task_add(struct TASK* task)
{
	if (task->level >= MAX_TASKLEVEL) { return; }
	struct TASKLEVEL *tl = &taskctl->tasklevels[task->level];
	if (tl->activetasks >= MAX_TASKS_EACH_LV) { return; }
	tl->p_tasks[tl->activetasks++] = task;
	task->flags = TASK_RUNNING;
	return;
}

void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->tasklevels[task->level];
	for (i = 0; i < tl->activetasks; ++i) {
		if (tl->p_tasks[i] == task) { break; }
	}
	--tl->activetasks;
	if (i < tl->now) { --tl->now; }
	if (tl->now >= tl->activetasks) { tl->now = 0; }
	task->flags = TASK_ALLOC;
	for (; i < tl->activetasks; ++i) {
		tl->p_tasks[i] = tl->p_tasks[i + 1];
	}
	return;
}

void task_switchsub(void)
{
	int i;
	for (i = 0; i < MAX_TASKLEVEL; ++i) {
		if (taskctl->tasklevels[i].activetasks > 0) { break; }
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}
