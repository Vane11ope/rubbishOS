[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api010.nas"]

	GLOBAL _api_refresh_window

[SECTION .text]

_api_refresh_window: ; void api_refresh_window(int win, int x0, int y0, int x1, int y1);
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX,12
	MOV EBX,[ESP+16]
	MOV EAX,[ESP+20]
	MOV ECX,[ESP+24]
	MOV ESI,[ESP+28]
	MOV EDI,[ESP+32]
	INT 0x40
	POP EBX
	POP ESI
	POP EDI
	RET
