#include "bootpack.h"
void memman_init(struct MEMMAN *memman)
{
	memman->frees = 0;
	memman->maxfrees = 0;
	memman->lostsize = 0;
	memman->losts = 0;
	return;
}

unsigned int memman_total(struct MEMMAN *memman)
{
	unsigned int i, t = 0;
	for (i = 0; i < memman->frees; ++i) {
		t += memman->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN *memman, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < memman->frees; ++i) {
		if (memman->free[i].size >= size) {
			a = memman->free[i].addr;
			memman->free[i].addr += size;
			memman->free[i].size -= size;
			if (memman->free[i].size == 0) {
				--memman->frees;
				for (; i < memman->frees; ++i) {
					memman->free[i] = memman->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

int memman_free(struct MEMMAN *memman, unsigned int addr, unsigned int size)
{
	int i, j;
	for (i = 0; i < memman->frees; ++i) {
		if (memman->free[i].addr > addr) {
			break;
		}
	}
	if (i > 0) {
		if (memman->free[i - 1].addr + memman->free[i - 1].size == addr) {
			memman->free[i - 1].size += size;
			if (i < memman->frees) {
				if (addr + size == memman->free[i].addr) {
					memman->free[i - 1].size += memman->free[i].size;
					--memman->frees;
					for(; i < memman->frees; ++i) {
						memman->free[i] = memman->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	if (i < memman->frees) {
		if (addr + size == memman->free[i].addr) {
			memman->free[i].addr = addr;
			memman->free[i].size += size;
			return 0;
		}
	}
	if (memman->frees < MEMMAN_FREES) {
		for (j = memman->frees; j > i; --j) {
			memman->free[i] = memman->free[i - 1];
		}
		++memman->frees;
		if (memman->maxfrees < memman->frees) {
			memman->maxfrees = memman->frees;
		}
		memman->free[i].addr = addr;
		memman->free[i].size = size;
		return 0;
	}
	++memman->losts;
	memman->lostsize += size;
	return -1;
}
