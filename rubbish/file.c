#include "bootpack.h"
#define ALL_SECTORS 2880
#define ADR_DISKIMG  0x00100000

void file_readfat (int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < ALL_SECTORS; i += 2) {
		fat[i + 0] = (img[j + 0] | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

void file_loadfile(int cluster_no, int size, char *buf, int *fat, char *img)
{
	int i;
	const short CLUSTER_SIZE = 512;
	for (;;) {
		if (size <= CLUSTER_SIZE) {
			for (i = 0; i < size; ++i) {
				buf[i] = img[cluster_no * CLUSTER_SIZE + i];
			}
			break;
		}
		for (i = 0; i < CLUSTER_SIZE; ++i) {
			buf[i] = img[cluster_no * CLUSTER_SIZE + i];
		}
		size -= CLUSTER_SIZE;
		buf += CLUSTER_SIZE;
		cluster_no = fat[cluster_no];
	}
	return;
}

char *file_loadfile2(int cluster_no, int *psize, int *fat)
{
	int size = *psize, size2;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	char *buf, *buf2;
	buf = (char *)memman_alloc_4k(memman, size);
	file_loadfile(cluster_no, size, buf, fat, (char *)(ADR_DISKIMG + 0x003e00));
	if (size >= 17) {
		size2 = tek_getsize(buf);
		if (size2 > 0) {
			buf2 = (char *)memman_alloc_4k(memman, size2);
			tek_decomp(buf, buf2, size2);
			memman_free_4k(memman, (int)buf, size);
			buf = buf2;
			*psize = size2;
		}
	}
	return buf;
}

struct FILEINFO* file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; ++j) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; ++i) {
		if (j >= 11) { return 0; }
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') { s[j] -= 0x20; }
			++j;
		}
	}
	for (i = 0; i < max; ) {
		if (finfo[i].name[0] == 0x00) { break; }
		if ((finfo[i].type & 0x18) == 0) {
			for (j = 0; j < 11; ++j) {
				if (finfo[i].name[j] != s[j]) { goto next; }
			}
			return finfo + i;
		}
next:
		++i;
	}
	return 0;
}
