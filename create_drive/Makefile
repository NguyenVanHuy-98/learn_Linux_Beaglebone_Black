obj-m := hello-kernel.o 
KERNELDIR=/home/huy/worlk/ti_sdk_beaglebone/board-support/ti-linux-kernel-6.1.119+gitAUTOINC+c490f4c0fe-ti
CURRENTDIR=$(shell pwd)
CROSS=/home/huy/worlk/ti_sdk_beaglebone/external-toolchain-dir/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

all:
	make -C $(KERNELDIR) M=$(CURRENTDIR) ARCH=arm CROSS_COMPILE=$(CROSS) modules
install:
	scp hello-kernel.ko huy@192.168.9.9:/home/root
clean:
	make -C $(KERNELDIR) M=$(CURRENTDIR) ARCH=arm CROSS_COMPILE=$(CROSS) clean
help: 
	make -C $(KERNELDIR) help