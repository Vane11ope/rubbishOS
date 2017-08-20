[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api017.nas"]

	GLOBAL _api_set_timer

[SECTION .text]

_api_set_timer: ; void api_set_timer(int timer, int time);
	PUSH EBX
	MOV EDX,18
	MOV EBX,[ESP+8]
	MOV EAX,[ESP+12]
	INT 0x40
	POP EBX
	RET
