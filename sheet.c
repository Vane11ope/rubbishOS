#include "bootpack.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	for (i = 0; i < MAX_SHEETS; ++i) {
		ctl->sheets0[i].flags = 0;
		ctl->sheets0[i].shtctl = ctl;
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for(i = 0; i < MAX_SHEETS; ++i) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;
			sht->height = -1;
			return sht;
		}
	}
	return 0;
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int opacity)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->opacity = opacity;
	return;
}

void sheet_updown(struct SHEET *sht, int height)
{
	int h, old = sht->height;
	if (height > sht->shtctl->top + 1) {
		height = sht->shtctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height;
	if (old > height) {
		if(height >= 0) {
			for (h = height + 1; h <= old; ++h) {
				sht->shtctl->sheets[h] = sht->shtctl->sheets[h - 1];
				sht->shtctl->sheets[h]->height = h;
			}
			sht->shtctl->sheets[height] = sht;
			sheet_refreshsub(sht->shtctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
		} else {
			if (sht->shtctl->top > old) {
				for (h = old; h < sht->shtctl->top; ++h) {
					sht->shtctl->sheets[h] = sht->shtctl->sheets[h + 1];
					sht->shtctl->sheets[h]->height = h;
				}
			}
			--sht->shtctl->top;
			sheet_refreshsub(sht->shtctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
		}
	} else if (old < height) {
		if (old >= 0) {
			for (h = old; h < height; ++h) {
				sht->shtctl->sheets[h] = sht->shtctl->sheets[h + 1];
				sht->shtctl->sheets[h]->height = h;
			}
		} else {
			for (h = sht->shtctl->top; h >= height; --h) {
				sht->shtctl->sheets[h + 1] = sht->shtctl->sheets[h];
				sht->shtctl->sheets[h + 1]->height = h;
			}
			++sht->shtctl->top;
		}
		sht->shtctl->sheets[height] = sht;
		sheet_refreshsub(sht->shtctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
	}
	return;
}

void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) {
		sheet_refreshsub(sht->shtctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height);
	}
	return;
}

void sheet_refreshsub(struct SHTCTL *shtctl, int vx0, int vy0, int vx1, int vy1, int height)
{
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = shtctl->vram;
	struct SHEET *sht;
	vx0 = max(vx0, 0);
	vy0 = max(vy0, 0);
	vx1 = min(vx1, shtctl->xsize);
	vy1 = min(vy1, shtctl->ysize);
	for (h = height; h <= shtctl->top; ++h) {
		sht = shtctl->sheets[h];
		buf = sht->buf;
		bx0 = max(vx0 - sht->vx0, 0);
		by0 = max(vy0 - sht->vy0, 0);
		bx1 = min(vx1 - sht->vx0, sht->bxsize);
		by1 = min(vy1 - sht->vy0, sht->bysize);
		for (by = by0; by < by1; ++by) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; ++bx) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->opacity) {
					vram[vy * shtctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}

void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) {
		sheet_refreshsub(sht->shtctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshsub(sht->shtctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
	}
	return;
}

void sheet_free(struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(sht, -1);
	}
	sht->flags = 0;
	return;
}
