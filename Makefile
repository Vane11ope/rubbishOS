MAKE = make -r
NASK = z_tools/nask
EDIMG = z_tools/edimg

default :
	make img

ipl.bin : ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst

rubbish.sys : rubbish.nas Makefile
	$(NASK) rubbish.nas rubbish.sys rubbish.lst

rubbish.img : ipl.bin rubbish.sys Makefile
	$(EDIMG) imgin:z_tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:rubbish.sys to:@: \
		imgout:rubbish.img

img :
	$(MAKE) rubbish.img

run :
	make img
	cp rubbish.img z_tools/qemu/fdimage0.bin
	make -C z_tools/qemu

clean :
	rm -f ipl.bin  # -f option is for the case where the file does not even exist
	rm -f ipl.lst
	rm -f rubbish.sys
	rm -f rubbish.lst

src_only :
	make clean
	rm -f rubbish.img
