#include "bootpack.h"
#define EFLAGS_AC_BIT        0x00040000
#define CR0_CACHE_DISABLE    0x60000000

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
			memman->free[j] = memman->free[j - 1];
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

unsigned int memman_alloc_4k(struct MEMMAN *memman, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(memman, size);
	return a;
}

int memman_free_4k(struct MEMMAN *memman, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(memman, addr, size);
	return i;
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
