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
	MOV ES,AX
	MOV SI,msg

putloop:
	MOV AL,[SI]
	ADD SI,1
	CMP AL,0
	JE fin
	MOV AH,0x0e
	MOV BX,15
	INT 0x10
	JMP putloop

fin:
	HLT
	JMP fin

msg:
	DB 0x0a, 0x0a
	DB "fuck you, world"
	DB 0x0a
	DB 0

RESB 0x7dfe-$;

DB 0x55, 0xaa

DB 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB 4600
DB 0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB 1469432
