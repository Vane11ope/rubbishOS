[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "crack.nas"]

	GLOBAL _RubbMain

[SECTION .text]
_RubbMain:
	MOV AX,1005*8
	MOV DS,AX
	CMP DWORD [DS:0x0004],'main'
	JNE fin
	MOV ECX,[DS:0x0000]
	MOV AX,2005*8
	MOV DS,AX

crackloop:
	ADD ECX,-1
	MOV BYTE [DS:ECX],123
	CMP ECX,0
	JNE crackloop

fin:
	MOV EDX,4
	INT 0x40
