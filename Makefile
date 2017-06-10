default :
	make img

ipl.bin : ipl.nas Makefile
	z_tools/nask ipl.nas ipl.bin ipl.lst

hello.img : ipl.bin Makefile
	z_tools/edimg imgin:z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 imgout:hello.img

asm :
	make -r ipl.bin

img :
	make -r hello.img

run :
	make img
	cp hello.img z_tools/qemu/fdimage0.bin
	make -C z_tools/qemu

clean :
	rm -f ipl.bin  # -f option is for the case where the file does not even exist
	rm -f ipl.lst

src_only :
	make clean
	rm -f hello.img
