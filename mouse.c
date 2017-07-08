#include "bootpack.h"
#define PORT_KEYDAT          0x0060
#define PORT_KEYCMD          0x0064
#define KEYCMD_SENDTO_MOUSE  0xd4
#define MOUSECMD_ENABLE      0xf4

struct FIFO32 *mousefifo;
int mousedata;

void inthandler2c(int *esp)
{
	int data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata);
	return;
}

void enable_mouse(struct FIFO32 *fifo, int data, struct MOUSE_DEC *mdec) {
	mousefifo = fifo;
	mousedata = data;
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
