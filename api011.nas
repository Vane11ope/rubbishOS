[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api011.nas"]

	GLOBAL _api_drawline

[SECTION .text]

_api_drawline: ; void api_drawline(int window, int x0, int y0, int x1, int y1, int color);
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV EDX,13
	MOV EBX,[ESP+20]
	MOV EAX,[ESP+24]
	MOV ECX,[ESP+28]
	MOV ESI,[ESP+32]
	MOV EDI,[ESP+36]
	MOV EBP,[ESP+40]
	INT 0x40
	POP EBX
	POP EBP
	POP ESI
	POP EDI
	RET
