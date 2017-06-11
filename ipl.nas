	CYLS EQU 10
	ORG 0x7c00
	JMP entry
	DB 0x90
	DB "HELLOIPL";
	DW 512;
	DB 1;
	DW 1;
	DB 2;
	DW 224;
	DW 2880;
	DB 0xf0;
	DW 9;
	DW 18;
	DW 2;
	DD 0;
	DD 2880;
	DB 0,0,0x29;
	DD 0xffffffff;
	DB "HELLO-OS";
	DB "FAT12";
	RESB 18;

entry:
	MOV AX,0
	MOV SS,AX
	MOV SP,0x7c00
	MOV DS,AX

	MOV AX,0x0820
	MOV ES,AX
	MOV CH,0
	MOV DH,0
	MOV CL,2

readloop:
	MOV SI,0

retry:
	MOV AH,0x02
	MOV AL,1
	MOV BX,0
	MOV DL,0x00
	INT 0x13
	JNC next
	ADD SI,1
	CMP SI,5
	JAE error
	MOV AH,0x00
	MOV DL,0x00
	INT 0x13
	JMP retry

next:
	MOV AX,ES
	ADD AX,0x0020
	MOV ES,AX
	ADD CL,1
	CMP CL,18
	JBE readloop
	MOV CL,1
	ADD DH,1
	CMP DH,2
	JB readloop
	MOV DH,0
	ADD CH,1
	CMP CH,CYLS
	JB readloop

fin:
	HLT
	JMP fin

error:
	MOV SI,msg

putloop:
	MOV AL,[SI]
	ADD SI,1
	CMP AL,0
	JE fin
	MOV AH,0x0e
	MOV BX,15
	MOV BL,0x22
	INT 0x10
	JMP putloop

msg:
	DB 0x0a,0x0a
	DB "load error, get the fuck out"
	DB 0x0a
	DB 0

RESB 0x7dfe-$;

DB 0x55,0xaa
