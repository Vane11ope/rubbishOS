[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api006.nas"]

	GLOBAL _api_init_malloc

[SECTION .text]

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
