[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api022.nas"]

	GLOBAL _api_fseek

[SECTION .text]

_api_fseek: ; int api_fseek(int fhandle, int offset, int mode);
	PUSH EBX
	MOV EDX,23
	MOV EAX,[ESP+8]
	MOV ECX,[ESP+16]
	MOV EBX,[ESP+12]
	INT 0x40
	POP EBX
	RET
