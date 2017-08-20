[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api012.nas"]

	GLOBAL _api_close_window

[SECTION .text]

_api_close_window: ; void api_close_window(int win);
	PUSH EBX
	MOV EDX,14
	MOV EBX,[ESP+8]
	INT 0x40
	POP EBX
	RET
