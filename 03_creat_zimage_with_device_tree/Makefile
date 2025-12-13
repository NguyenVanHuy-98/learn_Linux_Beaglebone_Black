obj-m := on_led.o 
KERNELDIR=/home/huy/work_space/ti_sdk_board/board-support/ti-linux-kernel-6.1.119+gitAUTOINC+c490f4c0fe-ti
CURRENTDIR=$(shell pwd)
CROSS=/home/huy/work_space/ti_sdk_board/external-toolchain-dir/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

all:
	make -C $(KERNELDIR) M=$(CURRENTDIR) ARCH=arm CROSS_COMPILE=$(CROSS) modules
clean:
	make -C $(KERNELDIR) M=$(CURRENTDIR) ARCH=arm CROSS_COMPILE=$(CROSS) clean
help: 
	make -C $(KERNELDIR) help
