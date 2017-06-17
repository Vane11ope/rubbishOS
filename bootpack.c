#include <stdio.h>

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

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

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

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void init_screen(char *vram, int x, int y);
void init_mouse(char *mouse, char bc);
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putblock8_8 (char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize);

void RubbMain(void)
{
	init_gdtidt();

	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	char mouse[256];
	init_mouse(mouse, COL8_000000);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 140, 100, mouse, 16);

	//static char font_A[16] = {
	//	0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
	//	0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	//};

	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "VANELLOPE");
	char* s;
	sprintf(s, "scrnx is %d", binfo->scrnx);
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 24, COL8_FFFFFF, s);

	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	putfonts8_asc(binfo->vram, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	for (;;) {
		io_hlt();
	}
}
