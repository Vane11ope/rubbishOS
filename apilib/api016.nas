[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api016.nas"]

	GLOBAL _api_init_timer

[SECTION .text]

_api_init_timer: ; void api_init_timer(int timer, int data);
	PUSH EBX
	MOV EDX,17
	MOV EBX,[ESP+8]
	MOV EAX,[ESP+12]
	INT 0x40
	POP EBX
	RET
