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
OBJS_API = api001.obj api002.obj api003.obj api004.obj api005.obj api006.obj api007.obj api008.obj api009.obj api010.obj api011.obj api012.obj api013.obj api014.obj api015.obj api016.obj api017.obj api018.obj api019.obj
GOLIB    = tools/golib00

apilib.lib : Makefile $(OBJS_API)
	$(GOLIB) $(OBJS_API) out:apilib.lib

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

app.bim: app.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:app.bim map:app.map app.obj apilib.lib

app.rub : app.bim Makefile
	$(BIM2RUB) app.bim app.rub 0

eff.bim: eff.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:eff.bim stack:1k map:eff.map eff.obj apilib.lib

eff.rub : eff.bim Makefile
	$(BIM2RUB) eff.bim eff.rub 0

window.bim: open_window.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:window.bim stack:1k map:window.map open_window.obj apilib.lib

window.rub : window.bim Makefile
	$(BIM2RUB) window.bim window.rub 40k

dot.bim: dot.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:dot.bim stack:1k map:dot.map dot.obj apilib.lib

dot.rub : dot.bim Makefile
	$(BIM2RUB) dot.bim dot.rub 47k

dots.bim: dots.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:dots.bim stack:1k map:dots.map dots.obj apilib.lib

dots.rub : dots.bim Makefile
	$(BIM2RUB) dots.bim dots.rub 48k

dots2.bim: dots2.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:dots2.bim stack:1k map:dots2.map dots2.obj apilib.lib

dots2.rub : dots2.bim Makefile
	$(BIM2RUB) dots2.bim dots2.rub 48k

line.bim: line.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:line.bim stack:1k map:line.map line.obj apilib.lib

line.rub : line.bim Makefile
	$(BIM2RUB) line.bim line.rub 48k

walk.bim: walk.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:walk.bim stack:1k map:walk.map walk.obj apilib.lib

walk.rub : walk.bim Makefile
	$(BIM2RUB) walk.bim walk.rub 48k

noodle.bim: noodle.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:noodle.bim stack:1k map:noodle.map noodle.obj apilib.lib

noodle.rub : noodle.bim Makefile
	$(BIM2RUB) noodle.bim noodle.rub 48k

beep.bim: beep.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:beep.bim stack:1k map:beep.map beep.obj apilib.lib

beep.rub : beep.bim Makefile
	$(BIM2RUB) beep.bim beep.rub 48k

color.bim: color.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color.bim stack:1k map:color.map color.obj apilib.lib

color.rub : color.bim Makefile
	$(BIM2RUB) color.bim color.rub 60k

color2.bim: color2.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:color2.bim stack:1k map:color2.map color2.obj apilib.lib

color2.rub : color2.bim Makefile
	$(BIM2RUB) color2.bim color2.rub 60k

effasm.bim : effasm.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:effasm.bim stack:1k map:effasm.map effasm.obj

effasm.rub : effasm.bim Makefile
	$(BIM2RUB) effasm.bim effasm.rub 0

crack.bim : crack.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:crack.bim stack:1k map:crack.map crack.obj

crack.rub : crack.bim Makefile
	$(BIM2RUB) crack.bim crack.rub 0

bug.bim: bug.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bug.bim map:bug.map bug.obj apilib.lib

bug.rub : bug.bim Makefile
	$(BIM2RUB) bug.bim bug.rub 0

bug2.bim: bug2.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bug2.bim map:bug2.map bug2.obj

bug2.rub : bug2.bim Makefile
	$(BIM2RUB) bug2.bim bug2.rub 0

bug3.bim: bug3.obj apilib.lib Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bug3.bim map:bug3.map bug3.obj apilib.lib

bug3.rub : bug3.bim Makefile
	$(BIM2RUB) bug3.bim bug3.rub 0

rubbish.img : ipl10.bin rubbish.sys app.rub eff.rub window.rub dot.rub dots.rub dots2.rub line.rub walk.rub noodle.rub color.rub color2.rub beep.rub effasm.rub crack.rub bug.rub bug2.rub bug3.rub Makefile
	$(EDIMG) imgin:tools/fdimg0at.tek \
		wbinimg src:ipl10.bin len:512 from:0 to:0 \
		copy from:rubbish.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:app.rub to:@: \
		copy from:eff.rub to:@: \
		copy from:window.rub to:@: \
		copy from:dot.rub to:@: \
		copy from:dots.rub to:@: \
		copy from:dots2.rub to:@: \
		copy from:line.rub to:@: \
		copy from:walk.rub to:@: \
		copy from:noodle.rub to:@: \
		copy from:beep.rub to:@: \
		copy from:color.rub to:@: \
		copy from:color2.rub to:@: \
		copy from:effasm.rub to:@: \
		copy from:crack.rub to:@: \
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
