SUMMARY = "Simple dual-LED char driver with ioctl"
DESCRIPTION = "Platform driver on_led_kernel dieu khien 2 LED bang GPIO + ioctl"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = " file://LICENSE;md5=c94f9406457821714facaa0f1b2a6972"

inherit module

SRC_URI = "file://on_led_kernel.c \
           file://Makefile \
           file://LICENSE \
          "

S = "${WORKDIR}"

# Tu dong load module khi boot (khong bat buoc, nhung tien)
KERNEL_MODULE_AUTOLOAD += "on_led_kernel"
