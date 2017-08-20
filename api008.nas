[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api008.nas"]

	GLOBAL _api_free

[SECTION .text]

_api_free: ; void api_free(char *addr, int size);
	PUSH EBX
	MOV EDX,10
	MOV EBX,[CS:0x0020]
	MOV EAX,[ESP+8]
	MOV ECX,[ESP+12]
	INT 0x40
	POP EBX
	RET
