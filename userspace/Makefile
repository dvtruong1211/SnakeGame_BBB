
GCC := /home/truongdv/Works/beagleboneblack/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc
 

SRC_FILES += lcd.c
SRC_FILES += main.c


all: $(SRC_FILES)
	$(GCC) -o main $(SRC_FILES) -I. -lthread

clean: 
	rm -rf ./main
