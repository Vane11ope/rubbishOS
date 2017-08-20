[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api014.nas"]

	GLOBAL _api_end

[SECTION .text]

_api_end:
	MOV EDX,4
	INT 0x40
