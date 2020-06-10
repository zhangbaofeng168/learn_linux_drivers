obj-m := nand_dev.o 
KDIR := /work/my_kernel/linux
all:
	make -C $(KDIR) M=$(PWD) modules CROSS_COMPILE=arm-linux- ARCH=arm
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.bak *.order

