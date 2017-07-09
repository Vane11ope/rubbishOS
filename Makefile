RUBPATH  = tools/rubbish/

MAKE     = make -r
NASK     = tools/nask
CC1      = tools/gocc1 -I$(RUBPATH) -Os -Wall -quiet
GAS2NASK = tools/gas2nask -a
OBJ2BIM  = tools/obj2bim
BIM2RUB  = tools/bim2rub
BIN2OBJ  = tools/bin2obj
RULEFILE = tools/rubbish/rubbish.rul
EDIMG    = tools/edimg
IMGTOL   = tools/imgtol
DEL      = rm -f
MAKEFONT = tools/makefont

default :
	$(MAKE) img

hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj : hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

bootpack.bim : bootpack.obj func.obj hankaku.obj graphic.obj dsctbl.obj timer.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj mtask.obj console.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj func.obj hankaku.obj graphic.obj dsctbl.obj timer.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj mtask.obj console.obj

bootpack.rub : bootpack.bim Makefile
	$(BIM2RUB) bootpack.bim bootpack.rub 0

rubbish.sys : asmhead.bin bootpack.rub Makefile
	cat asmhead.bin  bootpack.rub > rubbish.sys

rubbish.img : ipl10.bin rubbish.sys Makefile
	$(EDIMG) imgin:tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:rubbish.sys to:@: \
		imgout:rubbish.img

%.gas : %.c Makefile
	$(CC1) -o $*.gas $*.c

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.bin : %.nas Makefile
	$(NASK) $*.nas $*.bin $*.lst

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

img :
	$(MAKE) rubbish.img

run :
	$(MAKE) img
	cp rubbish.img tools/qemu/fdimage0.bin
	$(MAKE) -C tools/qemu

clean :
	$(DEL) *.bin  # -f option is for the case where the file does not even exist
	$(DEL) *.lst
	$(DEL) *.gas
	$(DEL) *.obj
	$(DEL) bootpack.nas
	$(DEL) bootpack.map
	$(DEL) bootpack.bim
	$(DEL) bootpack.rub
	$(DEL) rubbish.sys

src_only :
	$(MAKE) clean
	$(DEL) rubbish.img
