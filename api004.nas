[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api004.nas"]

	GLOBAL _api_putstr_on_window

[SECTION .text]

_api_putstr_on_window: ; void api_putstr_on_window(int win, int x, int y, int color, int len, char *str);
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV EDX,6
	MOV EBX,[ESP+20]
	MOV ESI,[ESP+24]
	MOV EDI,[ESP+28]
	MOV EAX,[ESP+32]
	MOV ECX,[ESP+36]
	MOV EBP,[ESP+40]
	INT 0x40
	POP EBX
	POP EBP
	POP ESI
	POP EDI
	RET
