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

bootpack.bim : bootpack.obj func.obj hankaku.obj graphic.obj dsctbl.obj timer.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj mtask.obj console.obj file.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj func.obj hankaku.obj graphic.obj dsctbl.obj timer.obj int.obj fifo.obj mouse.obj keyboard.obj memory.obj sheet.obj mtask.obj console.obj file.obj

bootpack.rub : bootpack.bim Makefile
	$(BIM2RUB) bootpack.bim bootpack.rub 0

rubbish.sys : asmhead.bin bootpack.rub Makefile
	cat asmhead.bin  bootpack.rub > rubbish.sys

app.bim: app.obj app_asm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:app.bim map:app.map app.obj app_asm.obj

app.rub : app.bim Makefile
	$(BIM2RUB) app.bim app.rub 0

eff.bim: eff.obj app_asm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:eff.bim stack:1k map:eff.map eff.obj app_asm.obj

eff.rub : eff.bim Makefile
	$(BIM2RUB) eff.bim eff.rub 0

window.bim: open_window.obj app_asm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:window.bim stack:10k map:window.map open_window.obj app_asm.obj

window.rub : window.bim Makefile
	$(BIM2RUB) window.bim window.rub 40k

dot.bim: dot.obj app_asm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:dot.bim stack:10k map:dot.map dot.obj app_asm.obj

dot.rub : dot.bim Makefile
	$(BIM2RUB) dot.bim dot.rub 47k

effasm.bim : effasm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:effasm.bim stack:1k map:effasm.map effasm.obj

effasm.rub : effasm.bim Makefile
	$(BIM2RUB) effasm.bim effasm.rub 0

bug.bim: bug.obj app_asm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bug.bim map:bug.map bug.obj app_asm.obj

bug.rub : bug.bim Makefile
	$(BIM2RUB) bug.bim bug.rub 0

bug2.bim: bug2.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bug2.bim map:bug2.map bug2.obj

bug2.rub : bug2.bim Makefile
	$(BIM2RUB) bug2.bim bug2.rub 0

bug3.bim: bug3.obj app_asm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bug3.bim map:bug3.map bug3.obj app_asm.obj

bug3.rub : bug3.bim Makefile
	$(BIM2RUB) bug3.bim bug3.rub 0

rubbish.img : ipl10.bin rubbish.sys app.rub eff.rub window.rub dot.rub effasm.rub bug.rub bug2.rub bug3.rub Makefile
	$(EDIMG) imgin:tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:rubbish.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:app.rub to:@: \
		copy from:eff.rub to:@: \
		copy from:window.rub to:@: \
		copy from:dot.rub to:@: \
		copy from:effasm.rub to:@: \
		copy from:bug.rub to:@: \
		copy from:bug2.rub to:@: \
		copy from:bug3.rub to:@: \
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
	$(DEL) *.map
	$(DEL) *.bim
	$(DEL) *.rub
	$(DEL) bootpack.nas
	$(DEL) rubbish.sys

src_only :
	$(MAKE) clean
	$(DEL) rubbish.img
