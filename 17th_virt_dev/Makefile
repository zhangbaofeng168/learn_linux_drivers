obj-m := virt_net.o 
KDIR := /work/my_kernel/linux
all:
	make -C $(KDIR) M=$(PWD) modules CROSS_COMPILE=arm-linux- ARCH=arm
	rm -f  *.o *.mod.o *.mod.c *.symvers *.bak *.order
clean:
	rm -f *.ko
