
KERN_DIR =/work/system/linux-3.4.2

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	rm -f *.o *.mod.o *.mod.c *.symvers *.bak *.order
clean:
	rm -f *.ko 

obj-m	+= cmos_ov9650_dev.o
obj-m	+= cmos_ov9650_drv.o 

