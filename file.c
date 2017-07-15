#include "bootpack.h"
#define ALL_SECTORS 2880

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
