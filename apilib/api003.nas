[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api003.nas"]

	GLOBAL _api_open_window

[SECTION .text]

_api_open_window: ; int api_open_window(char *buf, int xsize, int ysize, int opacity, char *title);
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX,5
	MOV EBX,[ESP+16] ; buf
	MOV ESI,[ESP+20] ; xsize
	MOV EDI,[ESP+24] ; ysize
	MOV EAX,[ESP+28] ; opacity
	MOV ECX,[ESP+32] ; title
	INT 0x40
	POP EBX
	POP ESI
	POP EDI
	RET
