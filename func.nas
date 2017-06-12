[INSTRSET "i486p"]
[FORMAT "WCOFF"]
[BITS 32]

[FILE "func.nas"]

	GLOBAL _io_hlt

[SECTION .text]

_io_hlt:
	HLT
	RET
