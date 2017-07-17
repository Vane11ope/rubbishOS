[BITS 32]
	MOV AL,'A'
	CALL 2*8:0xdc7
fin:
	HLT
	JMP fin
