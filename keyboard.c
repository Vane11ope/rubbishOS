#include "bootpack.h"
#define PORT_KEYDAT          0x0060
#define PORT_KEYSTA          0x0064
#define PORT_KEYCMD          0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE    0x60
#define KBC_MODE             0x47

struct FIFO32 *keyfifo;
int keydata;

void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata);
	return;
}

void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(struct FIFO32 *fifo, int data)
{
	keyfifo = fifo;
	keydata = data;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}
