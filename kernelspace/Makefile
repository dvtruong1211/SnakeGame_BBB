obj-m 			+= button_module.o
obj-m 			+= lcd_driver.o
lcd_driver-objs := lcd_5110_module.o lcd_5110.o

PWD := $(shell pwd)
CROSS=/home/truongdv/Works/beagleboneblack/gcc-11.3.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi-
KER_DIR=/home/truongdv/Works/beagleboneblack/kernelbuildscripts/KERNEL

all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KER_DIR) M=$(PWD) modules

clean:
	make -C $(KER_DIR) M=$(PWD) clean