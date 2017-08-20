[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "eff_asm.nas"]

	GLOBAL _RubbMain

[SECTION .text]

_RubbMain:
	MOV EDX,2
	MOV EBX,msg
	INT 0x40
	MOV EDX,4
	INT 0x40

[SECTION .data]
msg:
	DB "fuck you, world", 0x0a, 0
