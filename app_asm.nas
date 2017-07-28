[FORMAT "WCOFF"] ; mode where create object file
[INSTRSET "i486p"]
[BITS 32]
[FILE "app.nas"]

	GLOBAL _api_init_malloc, _api_malloc, _api_free, _api_putchar, _api_putstr, _api_open_window, _api_putstr_on_window, _api_boxfill_on_window, _api_dot, _api_refresh_window, _api_end

[SECTION .text]

_api_putchar: ; void api_putchar(int c);
	MOV EDX,1
	MOV AL,[ESP + 4]
	INT 0x40
	RET

_api_putstr: ; void api_putstr(char *s);
	PUSH EBX
	MOV EDX,2
	MOV EBX,[ESP+8]
	INT 0x40
	POP EBX
	RET

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

_api_putstr_on_window: ; void api_putstr_on_window(int win, int x, int y, int opacity, int len, char *str);
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

_api_boxfill_on_window: ; void api_boxfill_on_window(int win, int x0, int y0, int x1, int y1, int opacity);
	PUSH EDI
	PUSH ESI
	PUSH EBP
	PUSH EBX
	MOV EDX,7
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

_api_init_malloc: ; void api_init_malloc(void);
	PUSH EBX
	MOV EDX,8
	MOV EBX,[CS:0x0020]
	MOV EAX,EBX
	ADD EAX,32*1024
	MOV ECX,[CS:0x0000]
	SUB ECX,EAX
	INT 0x40
	POP EBX
	RET

_api_malloc: ; char *api_malloc(int size);
	PUSH EBX
	MOV EDX,9
	MOV EBX,[CS:0x0020]
	MOV ECX,[ESP+8]
	INT 0x40
	POP EBX
	RET

_api_free: ; void api_free(char *addr, int size);
	PUSH EBX
	MOV EDX,10
	MOV EBX,[CS:0x0020]
	MOV EAX,[ESP+8]
	MOV ECX,[ESP+12]
	INT 0x40
	POP EBX
	RET

_api_dot: ; void api_dot(int window, int x, int y, int color);
	PUSH EDI
	PUSH ESI
	PUSH EBX
	MOV EDX,11
	MOV EBX,[ESP+16]
	MOV ESI,[ESP+20]
	MOV EDI,[ESP+24]
	MOV EAX,[ESP+28]
	INT 0x40
	POP EBX
	POP ESI
	POP EDI
	RET

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

_api_end:
	MOV EDX,4
	INT 0x40
