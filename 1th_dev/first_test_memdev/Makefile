obj-m := memdev.o
KDIR := /home/S5-driver/lesson7/linux-tiny6410/
all:
	make -C $(KDIR) M=$(PWD) modules CROSS_COMPILE=arm-linux- ARCH=arm
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers *.bak *.order

