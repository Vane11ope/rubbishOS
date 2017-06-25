#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void RubbMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *sht_buf_back, sht_buf_mouse[256];
	char s[40], keybuf[32], mousebuf[128];
	short tweetx = 11;
	short tweety = binfo->scrny - 20;
	unsigned int memtotal;
	int mouse_x = (binfo->scrnx - 16) / 2;
	int mouse_y = (binfo->scrny - 28 - 16) / 2;
	int mouse_w = 16;
	int mouse_h = 16;
	int mouse_s = 16;
	int i = 0;

	init_gdtidt();
	init_pic();
	io_sti();

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_keyboard();
	init_palette();

	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, sht_buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, sht_buf_mouse, 16, 16, 99);
	init_screen(sht_buf_back, binfo->scrnx, binfo->scrny);
	init_mouse(sht_buf_mouse, 99);
	sheet_slide(shtctl, sht_back, 0, 0);
	sheet_slide(shtctl, sht_mouse, mouse_x, mouse_y);
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse, 1);
	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024*1024), memman_total(memman) / 1024);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	//putblock8_8(binfo->vram, binfo->scrnx, mouse_w, mouse_h, mouse_x, mouse_y, sht_buf_mouse, mouse_s);
	putfonts8_asc(sht_buf_back, binfo->scrnx, 240, 145, COL8_FFFFFF, "VANELLOPE");
	putfonts8_asc(sht_buf_back, binfo->scrnx, tweetx, tweety, COL8_000000, "TWEET");

	sheet_refresh(shtctl);

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
				boxfill8(sht_buf_back, binfo->scrnx, COL8_000000, 0, 0, 15, 31);
				putfonts8_asc(sht_buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
				sheet_refresh(shtctl);
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
					boxfill8(sht_buf_back, binfo->scrnx, COL8_000000, 50, 0, 170, 31);
					putfonts8_asc(sht_buf_back, binfo->scrnx, 50, 0, COL8_FFFFFF, s);
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
					sheet_slide(shtctl, sht_mouse, mouse_x, mouse_y);
				}
			}
		}
	}
}

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflag, cr0, i;
	eflag = io_load_eflags();
	eflag |= EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if ((eflag & EFLAGS_AC_BIT) != 0) { flg486 = 1; }
	eflag &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	i = memtest_sub(start, end);
	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end)
{
	volatile unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for (i = start; i <= end; i += 0x1000) {
		p = (unsigned int *) (i + 0xffc);
		old = *p;
		*p = pat0;
		*p ^= 0xffffffff;
		if (*p != pat1) {
not_memory:
			*p = old;
			break;
		}
		*p ^= 0xffffffff;
		if (*p != pat0) {
			goto not_memory;
		}
		*p = old;
	}
	return i;
}
