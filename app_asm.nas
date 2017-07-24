[FORMAT "WCOFF"] ; mode where create object file
[INSTRSET "i486p"]
[BITS 32]
[FILE "app.nas"]

	GLOBAL _api_putchar, _api_putstr, _api_end

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

_api_end:
	MOV EDX,4
	INT 0x40
