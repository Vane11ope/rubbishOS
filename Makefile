RUBPATH  = tools/rubbish/

MAKE     = make -r
NASK     = tools/nask
CC1      = tools/gocc1 -I$(RUBPATH) -Os -Wall -quiet
GAS2NASK = tools/gas2nask -a
OBJ2BIM  = tools/obj2bim
BIM2RUB  = tools/bim2rub
RULEFILE = tools/rubbish/rubbish.rul
EDIMG    = tools/edimg
IMGTOL   = tools/imgtol
DEL      = rm -f

default :
	$(MAKE) img

ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas ipl10.bin ipl10.lst

asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

bootpack.gas : bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas : bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj : bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

bootpack.bim : bootpack.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj

bootpack.rub : bootpack.bim Makefile
	$(BIM2RUB) bootpack.bim bootpack.rub 0

rubbish.sys : asmhead.bin bootpack.rub Makefile
	cat asmhead.bin  bootpack.rub > rubbish.sys

rubbish.img : ipl10.bin rubbish.sys Makefile
	$(EDIMG) imgin:tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:rubbish.sys to:@: \
		imgout:rubbish.img

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
