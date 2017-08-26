RUBPATH  = ../tools/rubbish/
MAKE     = make -r
NASK     = ../tools/nask
CC1      = ../tools/gocc1 -I$(RUBPATH) -Os -Wall -quiet
GAS2NASK = ../tools/gas2nask -a
OBJ2BIM  = ../tools/obj2bim
BIM2RUB  = ../tools/bim2rub
BIN2OBJ  = ../tools/bin2obj
RULEFILE = ../tools/rubbish/rubbish.rul
EDIMG    = ../tools/edimg
IMGTOL   = ../tools/imgtol
DEL      = rm -f
MAKEFONT = ../tools/makefont
OBJS_BOOTPACK = bootpack.obj func.obj hankaku.obj graphic.obj dsctbl.obj timer.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj mtask.obj console.obj file.obj tek.obj
GOLIB    = ../tools/golib00

default :
	$(MAKE) ipl20.bin
	$(MAKE) rubbish.sys

ipl20.bin : ipl20.nas Makefile
	$(NASK) ipl20.nas ipl20.bin ipl20.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		$(OBJS_BOOTPACK)

bootpack.rub : bootpack.bim Makefile
	$(BIM2RUB) bootpack.bim bootpack.rub 0

rubbish.sys : asmhead.bin bootpack.rub Makefile
	cat asmhead.bin  bootpack.rub > rubbish.sys

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

clean :
	$(DEL) *.lst
	$(DEL) *.obj
	$(DEL) *.map
	$(DEL) *.bim
	$(DEL) *.rub
	$(DEL) asmhead.bin
	$(DEL) hankaku.bin

src_only :
	$(MAKE) clean
	$(DEL) ipl20.bin
	$(DEL) rubbish.sys
