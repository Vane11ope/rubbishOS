#include "bootpack.h"

void RubbMain(void)
{
	init_gdtidt();
	init_pic();

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
