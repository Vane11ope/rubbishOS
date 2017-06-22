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

	int mx, my, mw, mh, ms;
	mx = 140;
	my = 100;
	mw = 16;
	mh = 16;
	ms = 16;
	char mouse[256];
	init_mouse(mouse, COL8_000000);
	putblock8_8(binfo->vram, binfo->scrnx, mw, mh, mx, my, mouse, ms);

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

	struct MOUSE_DEC mdec;
	init_keyboard();
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

					boxfill8(binfo->vram, binfo->scrnx, COL8_000000, mx, my, mx + mw, my + mh);
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

void enable_mouse(struct MOUSE_DEC *mdec) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat) {
	switch (mdec->phase) {
		case 0:
			if (dat == 0xfa) { mdec->phase = 1; }
			return 0;
		case 1:
			if ((dat & 0xc8) == 0x08) {
				mdec->buf[0] = dat;
				mdec->phase = 2;
			}
			return 0;
		case 2:
			mdec->buf[1] = dat;
			mdec->phase = 3;
			return 0;
		case 3:
			mdec->buf[2] = dat;
			mdec->phase = 1;
			mdec->btn = mdec->buf[0] & 0x07;
			mdec->x = mdec->buf[1];
			mdec->y = mdec->buf[2];
			if ((mdec->buf[0] & 0x10) != 0) {
				mdec->x |= 0xffffff00;
			}
			if ((mdec->buf[0] & 0x20) != 0) {
				mdec->y |= 0xffffff00;
			}
			mdec->y *= -1;
			return 1;
	}
	return -1;
}
