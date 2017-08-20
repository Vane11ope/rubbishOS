[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api015.nas"]

	GLOBAL _api_alloc_timer

[SECTION .text]

_api_alloc_timer: ; int api_alloc_timer(void);
	MOV EDX,16
	INT 0x40
	RET
