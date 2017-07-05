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
	GLOBAL _taskswitch4
	GLOBAL _asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c

	EXTERN _inthandler20, _inthandler21, _inthandler27, _inthandler2c

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

_taskswitch4 ; void taskswitch4(void);
	JMP 4*8:0
	RET

_asm_inthandler20:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV  EAX,ESP
	PUSH EAX
	MOV  AX,SS
	MOV  DS,AX
	MOV  ES,AX
	CALL _inthandler20
	POP  EAX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler21:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV  EAX,ESP
	PUSH EAX
	MOV  AX,SS
	MOV  DS,AX
	MOV  ES,AX
	CALL _inthandler21
	POP  EAX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler27:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV  EAX,ESP
	PUSH EAX
	MOV  AX,SS
	MOV  DS,AX
	MOV  ES,AX
	CALL _inthandler27
	POP  EAX
	POPAD
	POP DS
	POP ES
	IRETD

_asm_inthandler2c:
	PUSH ES
	PUSH DS
	PUSHAD
	MOV  EAX,ESP
	PUSH EAX
	MOV  AX,SS
	MOV  DS,AX
	MOV  ES,AX
	CALL _inthandler2c
	POP  EAX
	POPAD
	POP DS
	POP ES
	IRETD
