[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "api018.nas"]

	GLOBAL _api_free_timer

[SECTION .text]

_api_free_timer: ; void api_free_timer(int timer);
	PUSH EBX
	MOV EDX,19
	MOV EBX,[ESP+8]
	INT 0x40
	POP EBX
	RET
