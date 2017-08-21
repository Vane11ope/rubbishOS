[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api023.nas"]

	GLOBAL _api_fsize

[SECTION .text]

_api_fsize: ; int api_fsize(int fhandle, int mode);
	MOV EDX,24
	MOV EAX,[ESP+4]
	MOV ECX,[ESP+8]
	INT 0x40
	RET
