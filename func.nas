[INSTRSET "i486p"]
[FORMAT "WCOFF"]
[BITS 32]

[FILE "func.nas"]

	GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt
	GLOBAL _io_in8, _io_in16, _io_in32
	GLOBAL _io_out8, _io_out16, _io_out32
	GLOBAL _io_load_eflags, _io_store_eflags
	GLOBAL _load_gdtr, _load_idtr
	GLOBAL _load_cr0, _store_cr0
	GLOBAL _load_tr
	GLOBAL _farjmp
	GLOBAL _farcall
	GLOBAL _asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler0d, _asm_rub_api
	GLOBAL _start_app

	EXTERN _inthandler20, _inthandler21, _inthandler27, _inthandler2c, _inthandler0d, _rub_api

[SECTION .text]

_io_hlt:
	HLT
	RET

_io_cli:
	CLI
	RET

_io_sti:
	STI
	RET

_io_stihlt:
	STI
	HLT
	RET

_io_in8: ; int io_in8(int port);
	MOV EDX,[ESP+4]
	MOV EAX,0
	IN  AL,DX
	RET

_io_in16: ; int io_in16(int port);
	MOV EDX,[ESP+4]
	MOV EAX,0
	IN  AX,DX
	RET

_io_in32: ; int io_in32(int port);
	MOV EDX,[ESP+4]
	IN  EAX,DX
	RET

_io_out8: ; void io_out8(int port, int data);
	MOV EDX,[ESP+4]
	MOV AL,[ESP+8]
	OUT DX,AL
	RET

_io_out16: ; void io_out16(int port, int data);
	MOV EDX,[ESP+4]
	MOV EAX,[ESP+8]
	OUT DX,AX
	RET

_io_out32: ; void io_out32(int port, int data);
	MOV EDX,[ESP+4]
	MOV EAX,[ESP+8]
	OUT DX,EAX
	RET

_io_load_eflags: ; int io_load_eflags(void);
	PUSHFD       ; PUSH EFLAGS
	POP EAX
	RET

_io_store_eflags: ; void io_store_eflags(int eflags);
	MOV EAX,[ESP+4]
	PUSH EAX
	POPFD ; POP EFLAGS
	RET

_load_gdtr: ; void load_gdtr(int limit, int addr);
	MOV  AX,[ESP+4]
	MOV  [ESP+6],AX
	LGDT [ESP+6]
	RET

_load_idtr: ; void load_idtr(int limit, int addr);
	MOV  AX,[ESP+4]
	MOV  [ESP+6],AX
	LIDT [ESP+6]
	RET

_load_cr0: ; int load_cr0(void);
	MOV  EAX,CR0
	RET

_load_tr: ; void load_tr(int tr);
	LTR [ESP + 4]
	RET

_store_cr0: ; void store_cr0(int cr0);
	MOV  EAX,[ESP+4]
	MOV  CR0,EAX
	RET

_farjmp: ; void farjmp(int eip, int cs);
	JMP FAR [ESP + 4]
	RET

_farcall: ; void farcall(int eip, int cs);
	CALL FAR [ESP + 4]
	RET

_asm_inthandler20:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX,SS
	CMP AX,1*8
	JNE .from_app
	MOV EAX,ESP
	PUSH SS
	PUSH EAX
	MOV AX,SS
	MOV DS,AX
	MOV ES,AX
	CALL _inthandler20
	ADD ESP,8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	MOV EAX,1*8
	MOV DS,AX
	MOV ECX,[0xfe4]
	ADD ECX,-8
	MOV [ECX+4],SS
	MOV [ECX],ESP
	MOV SS,AX
	MOV ES,AX
	MOV ESP,ECX
	CALL _inthandler20
	POP ECX
	POP EAX
	MOV SS,AX
	MOV ESP,ECX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler21:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX,SS
	CMP AX,1*8
	JNE .from_app
	MOV EAX,ESP
	PUSH SS
	PUSH EAX
	MOV AX,SS
	MOV DS,AX
	MOV ES,AX
	CALL _inthandler21
	ADD ESP,8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	MOV EAX,1*8
	MOV DS,AX
	MOV ECX,[0xfe4]
	ADD ECX,-8
	MOV [ECX+4],SS
	MOV [ECX],ESP
	MOV SS,AX
	MOV ES,AX
	MOV ESP,ECX
	CALL _inthandler21
	POP ECX
	POP EAX
	MOV SS,AX
	MOV ESP,ECX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler27:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX,SS
	CMP AX,1*8
	JNE .from_app
	MOV EAX,ESP
	PUSH SS
	PUSH EAX
	MOV AX,SS
	MOV DS,AX
	MOV ES,AX
	CALL _inthandler27
	ADD ESP,8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	MOV EAX,1*8
	MOV DS,AX
	MOV ECX,[0xfe4]
	ADD ECX,-8
	MOV [ECX+4],SS
	MOV [ECX],ESP
	MOV SS,AX
	MOV ES,AX
	MOV ESP,ECX
	CALL _inthandler27
	POP ECX
	POP EAX
	MOV SS,AX
	MOV ESP,ECX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler2c:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX,SS
	CMP AX,1*8
	JNE .from_app
	MOV EAX,ESP
	PUSH SS
	PUSH EAX
	MOV AX,SS
	MOV DS,AX
	MOV ES,AX
	CALL _inthandler2c
	ADD ESP,8
	POPAD
	POP DS
	POP ES
	IRETD
.from_app:
	MOV EAX,1*8
	MOV DS,AX
	MOV ECX,[0xfe4]
	ADD ECX,-8
	MOV [ECX+4],SS
	MOV [ECX],ESP
	MOV SS,AX
	MOV ES,AX
	MOV ESP,ECX
	CALL _inthandler2c
	POP ECX
	POP EAX
	MOV SS,AX
	MOV ESP,ECX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler0d:
	STI
	PUSH ES
	PUSH DS
	PUSHAD
	MOV AX,SS
	CMP AX,1*8
	JNE .from_app
	MOV EAX,ESP
	PUSH SS
	PUSH EAX
	MOV AX,SS
	MOV DS,AX
	MOV ES,AX
	CALL _inthandler0d
	ADD ESP,8
	POPAD
	POP DS
	POP ES
	ADD ESP,4
	IRETD
.from_app:
	CLI
	MOV EAX,1*8
	MOV DS,AX
	MOV ECX,[0xfe4]
	ADD ECX,-8
	MOV [ECX+4],SS
	MOV [ECX],ESP
	MOV SS,AX
	MOV ES,AX
	MOV ESP,ECX
	STI
	CALL _inthandler0d
	CLI
	CMP EAX,0
	JNE .kill
	POP ECX
	POP EAX
	MOV SS,AX
	MOV ESP,ECX
	POPAD
	POP DS
	POP ES
	ADD ESP,4
	IRETD
.kill:
	MOV EAX,1*8
	MOV ES,AX
	MOV SS,AX
	MOV DS,AX
	MOV FS,AX
	MOV GS,AX
	MOV ESP,[0xfe4]
	STI
	POPAD
	RET

_asm_rub_api:
	PUSH DS
	PUSH ES
	PUSHAD
	MOV EAX,1*8
	MOV DS,AX
	MOV ECX,[0xfe4]
	ADD ECX,-40
	MOV [ECX+32],ESP
	MOV [ECX+36],SS
	MOV EDX,[ESP]
	MOV EBX,[ESP+4]
	MOV [ECX],EDX
	MOV [ECX+4],EBX
	MOV EDX,[ESP+8]
	MOV EBX,[ESP+12]
	MOV [ECX+8],EDX
	MOV [ECX+12],EBX
	MOV EDX,[ESP+16]
	MOV EBX,[ESP+20]
	MOV [ECX+16],EDX
	MOV [ECX+20],EBX
	MOV EDX,[ESP+24]
	MOV EBX,[ESP+28]
	MOV [ECX+24],EDX
	MOV [ECX+28],EBX
	MOV ES,AX
	MOV SS,AX
	MOV ESP,ECX
	STI
	CALL _rub_api
	MOV ECX,[ESP+32]
	MOV EAX,[ESP+36]
	CLI
	MOV SS,AX
	MOV ESP,ECX
	POPAD
	POP ES
	POP DS
	IRETD

_start_app: ; void start_app(int eip, int cs, int esp, int ds);
	PUSHAD
	MOV EAX,[ESP+36]
	MOV ECX,[ESP+40]
	MOV EDX,[ESP+44]
	MOV EBX,[ESP+48]
	MOV [0xfe4],ESP
	CLI
	MOV ES,BX
	MOV SS,BX
	MOV DS,BX
	MOV FS,BX
	MOV GS,BX
	MOV ESP,EDX
	STI
	PUSH ECX
	PUSH EAX
	CALL FAR [ESP]

	MOV EAX,1*8
	CLI
	MOV ES,AX
	MOV SS,AX
	MOV DS,AX
	MOV FS,AX
	MOV GS,AX
	MOV ESP,[0xfe4]
	STI
	POPAD
	RET
