#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void RubbMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
	struct MOUSE_DEC mdec;
	char s[40], keybuf[32], mouse[256], mousebuf[128];
	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	int mouse_x = 140;
	int mouse_y = 100;
	int mouse_w = 16;
	int mouse_h = 16;
	int mouse_s = 16;
	int i = 0;

	init_gdtidt();
	init_pic();
	io_sti();
	init_keyboard();
	init_mouse(mouse, COL8_000000);
	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	putblock8_8(binfo->vram, binfo->scrnx, mouse_w, mouse_h, mouse_x, mouse_y, mouse, mouse_s);
	putfonts8_asc(binfo->vram, binfo->scrnx, 240, 145, COL8_FFFFFF, "VANELLOPE");
	putfonts8_asc(binfo->vram, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	enable_mouse(&mdec);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) <= 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) > 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
			} else {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 50, 0, 170, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 50, 0, COL8_FFFFFF, s);

					boxfill8(binfo->vram, binfo->scrnx, COL8_000000, mouse_x, mouse_y, mouse_x + mouse_w, mouse_y + mouse_h);
					mouse_x += mdec.x;
					mouse_y += mdec.y;
					if (mouse_x < 0) {
						mouse_x = 0;
					}
					if (mouse_y < 0) {
						mouse_y = 0;
					}
					if (mouse_x > binfo->scrnx - mouse_w) {
						mouse_x = binfo->scrnx - mouse_w;
					}
					if (mouse_y > binfo->scrny - mouse_h) {
						mouse_y = binfo->scrny - mouse_h;
					}
					sprintf(s, "(%3d, %3d)", mouse_x, mouse_y);
					boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 15, 79, 30);
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 15, COL8_FFFFFF, s);
					putblock8_8(binfo->vram, binfo->scrnx, mouse_w, mouse_h, mouse_x, mouse_y, mouse, mouse_s);
				}
			}
		}
	}
}
