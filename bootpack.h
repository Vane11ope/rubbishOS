#include <stdio.h>

/* asmhead.nas */
struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

/* bootpack.c */
#define PORT_KEYDAT          0x0060
#define PORT_KEYSTA          0x0064
#define PORT_KEYCMD          0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KBC_MODE             0x47
#define KEYCMD_SENDTO_MOUSE  0xd4
#define MOUSECMD_ENABLE      0xf4
#define EFLAGS_AC_BIT        0x00040000
#define CR0_CACHE_DISABLE    0x60000000
#define MAX_TASKS            1000
#define TASK_GDT0            3

struct TSS32 {
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};

struct TASK {
	int sel, flags;
	struct TSS32 tss;
};

struct TASKCTL {
	int activetasks;
	int now;
	struct TASK *tasks[MAX_TASKS];
	struct TASK tasks0[MAX_TASKS];
};

struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, opacity, height, flags;
	struct SHTCTL *shtctl;
};
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void task_b_main(struct SHEET *sht_back);

/* func.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
int load_cr0(void);
void store_cr0(int cr0);
void load_tr(int tr);
void farjmp(int eip, int cs);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

/* memory.c */
#define MEMMAN_FREES 4090
#define MEMMAN_ADDR  0x003c0000
struct FREEINFO {
	unsigned int addr, size;
};
struct MEMMAN {
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
void memman_init(struct MEMMAN *memman);
unsigned int memman_total(struct MEMMAN *memman);
unsigned int memman_alloc(struct MEMMAN *memman, unsigned int size);
int memman_free(struct MEMMAN *memman, unsigned int addr, unsigned int size);
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

/* fifo.c */
#define FLAGS_OVERRUN 0x0001
struct FIFO32 {
	int *buf;
	int next_w, next_r, size, free, flags;
	struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

/* dsctbl.c */
#define AR_INTGATE32 0x008e
#define AR_TSS32 0x0089
#define ADR_GDT 0x00270000
#define ADR_IDT 0x0026f800
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

/* int.c */
#define PIC0_ICW1            0x0020
#define PIC0_ICW2            0x0021
#define PIC0_ICW3            0x0021
#define PIC0_ICW4            0x0021
#define PIC1_ICW1            0x00a0
#define PIC1_ICW2            0x00a1
#define PIC1_ICW3            0x00a1
#define PIC1_ICW4            0x00a1
#define PIC0_IMR             0x0021
#define PIC1_IMR             0x00a1
#define PIC0_OCW2            0x0020
#define PIC1_OCW2            0x00a0
void init_pic(void);
void inthandler21(int *esp);

/* timer.c */
#define MAX_TIMER 500
struct TIMER {
	struct TIMER *next;
	unsigned int timeout, flags;
	struct FIFO32 *fifo;
	int data;
};
struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *firsttimer;
	struct TIMER timer0[MAX_TIMER];
};
void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);

/* mtask.c */
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task);
void task_switch(void);
void task_sleep(struct TASK *task);

/* sheet.c */
#define MAX_SHEETS 256
#define SHEET_USE 1
struct SHTCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int opacity);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_refreshsub(struct SHTCTL *shtctl, int vx0, int vy0, int vx1, int vy1, int height0, int height1);
void sheet_refreshmap(struct SHTCTL *shtctl, int vx0, int vy0, int vx1, int vy1, int height);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);

/* graphic.c */
#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(char *vram, int x, int y);
void init_mouse(char *mouse, char bc);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void make_textbox8(struct SHEET *sheet, int x0, int y0, int sx, int sy, int color);
void make_window(unsigned char *buf, int xsize, int ysize, char *title);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putfonts8_asc_sht(struct SHEET *sheet, int x, int y, int color, int backcolor, char *str);
void putblock8_8 (char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize);

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data);

/* mouse.c */
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

/* utility */
static inline int max(int a, int b) { return a >= b ? a : b; }
static inline int min(int a, int b) { return a <= b ? a : b; }
static inline int length(unsigned char* s) {
	int len = 0;
	for (; *s != 0x00; ++s) {
		++len;
	}
	return len;
}
