#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void RubbMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;

	init_gdtidt();
	init_pic();
	io_sti();
	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	char mouse[256];
	init_mouse(mouse, COL8_000000);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, 140, 100, mouse, 16);

	//static char font_A[16] = {
	//	0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
	//	0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
	//};

	putfonts8_asc(binfo->vram, binfo->scrnx, 240, 145, COL8_FFFFFF, "VANELLOPE");
	//char* s;
	//sprintf(s, "scrnx is %d", binfo->scrnx);
	//putfonts8_asc(binfo->vram, binfo->scrnx, 8, 24, COL8_FFFFFF, s);

	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	putfonts8_asc(binfo->vram, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);

	int i;
	char s[40], keybuf[32], mousebuf[128];
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	unsigned char mouse_dbuf[3], mouse_phase;
	mouse_phase = 0;
	init_keyboard();
	enable_mouse();

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
				switch (mouse_phase) {
					case 0:
						if (i == 0xfa) { mouse_phase = 1; }
						break;
					case 1:
						mouse_dbuf[0] = i;
						mouse_phase = 2;
						break;
					case 2:
						mouse_dbuf[1] = i;
						mouse_phase = 3;
						break;
					case 3:
						mouse_dbuf[2] = i;
						mouse_phase = 1;
						sprintf(s, "%02X %02X %02X", mouse_dbuf[0], mouse_dbuf[1], mouse_dbuf[2]);
						boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 50, 0, 114, 31);
						putfonts8_asc(binfo->vram, binfo->scrnx, 50, 0, COL8_FFFFFF, s);
						break;
				}
			}
		}
	}
}

void wait_KBC_sendready(void) {
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

void enable_mouse(void) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}
