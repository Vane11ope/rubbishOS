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
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void init_keyboard(void);
void wait_KBC_sendready(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

/* dsctbl.c */
#define AR_INTGATE32 0x008e
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
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

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
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putblock8_8 (char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize);

/* fifo.c */
#define FLAGS_OVERRUN 0x0001

struct FIFO8 {
	unsigned char *buf;
	int next_w, next_r, size, free, flags;
};
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

/* mouse.c */
void inthandler2c(int *esp);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

/* keyboard.c */
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(void);

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
