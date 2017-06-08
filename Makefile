run :
	./z_tools/nask hello.nas hello.img
	cp hello.img ./z_tools/qemu/fdimage0.bin
	make -C ./z_tools/qemu
