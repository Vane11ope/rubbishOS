RUBPATH  = tools/rubbish/

MAKE     = make -r
EDIMG    = tools/edimg
IMGTOL   = tools/imgtol
DEL      = rm -f

default :
	$(MAKE) rubbish.img

rubbish.img : rubbish/ipl20.nas rubbish/rubbish.sys Makefile \
		a/a.rub beep/beep.rub color/color.rub color2/color2.rub \
		dot/dot.rub dots/dots.rub dots2/dots2.rub eff/eff.rub line/line.rub \
		noodle/noodle.rub open_window/window.rub walk/walk.rub sosu/sosu.rub \
		typeipl/typeipl.rub cat/cat.rub iroha/iroha.rub
	$(EDIMG) imgin:tools/fdimg0at.tek \
		wbinimg src:rubbish/ipl20.bin len:512 from:0 to:0 \
		copy from:rubbish/rubbish.sys to:@: \
		copy from:rubbish/ipl20.nas to:@: \
		copy from:sample.txt to:@: \
		copy from:euc.txt to:@: \
		copy from:a/a.rub to:@: \
		copy from:eff/eff.rub to:@: \
		copy from:open_window/window.rub to:@: \
		copy from:dot/dot.rub to:@: \
		copy from:dots/dots.rub to:@: \
		copy from:dots2/dots2.rub to:@: \
		copy from:line/line.rub to:@: \
		copy from:walk/walk.rub to:@: \
		copy from:noodle/noodle.rub to:@: \
		copy from:beep/beep.rub to:@: \
		copy from:color/color.rub to:@: \
		copy from:color2/color2.rub to:@: \
		copy from:sosu/sosu.rub to:@: \
		copy from:typeipl/typeipl.rub to:@: \
		copy from:cat/cat.rub to:@: \
		copy from:iroha/iroha.rub to:@: \
		copy from:nihongo/nihongo.fnt to:@: \
		imgout:rubbish.img

run :
	$(MAKE) rubbish.img
	cp rubbish.img tools/qemu/fdimage0.bin
	$(MAKE) -C tools/qemu

full :
	$(MAKE) -C rubbish
	$(MAKE) -C apilib
	$(MAKE) -C a
	$(MAKE) -C eff
	$(MAKE) -C open_window
	$(MAKE) -C dot
	$(MAKE) -C dots
	$(MAKE) -C dots2
	$(MAKE) -C line
	$(MAKE) -C walk
	$(MAKE) -C noodle
	$(MAKE) -C beep
	$(MAKE) -C color
	$(MAKE) -C color2
	$(MAKE) -C sosu
	$(MAKE) -C typeipl
	$(MAKE) -C cat
	$(MAKE) -C iroha
	$(MAKE) rubbish.img

run_full :
	$(MAKE) full
	cp rubbish.img tools/qemu/fdimage0.bin
	$(MAKE) -C tools/qemu

run_os :
	$(MAKE) -C rubbish
	$(MAKE) run

src_only :
	$(MAKE) clean
	$(DEL) rubbish.img

clean :

clean_full :
	$(MAKE) -C rubbish     clean
	$(MAKE) -C apilib      clean
	$(MAKE) -C a           clean
	$(MAKE) -C eff         clean
	$(MAKE) -C open_window clean
	$(MAKE) -C dot         clean
	$(MAKE) -C dots        clean
	$(MAKE) -C dots2       clean
	$(MAKE) -C line        clean
	$(MAKE) -C walk        clean
	$(MAKE) -C noodle      clean
	$(MAKE) -C beep        clean
	$(MAKE) -C color       clean
	$(MAKE) -C color2      clean
	$(MAKE) -C sosu        clean
	$(MAKE) -C typeipl     clean
	$(MAKE) -C cat         clean
	$(MAKE) -C iroha       clean

src_only_full :
	$(MAKE) -C rubbish     src_only
	$(MAKE) -C apilib      src_only
	$(MAKE) -C a           src_only
	$(MAKE) -C eff         src_only
	$(MAKE) -C open_window src_only
	$(MAKE) -C dot         src_only
	$(MAKE) -C dots        src_only
	$(MAKE) -C dots2       src_only
	$(MAKE) -C line        src_only
	$(MAKE) -C walk        src_only
	$(MAKE) -C noodle      src_only
	$(MAKE) -C beep        src_only
	$(MAKE) -C color       src_only
	$(MAKE) -C color2      src_only
	$(MAKE) -C sosu        src_only
	$(MAKE) -C typeipl     src_only
	$(MAKE) -C cat         src_only
	$(MAKE) -C iroha       src_only
	$(DEL) rubbish.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	$(DEL) rubbish.img
