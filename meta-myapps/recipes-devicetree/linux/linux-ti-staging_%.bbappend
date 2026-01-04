FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Thêm file defconfig vào SRC_URI để Yocto copy vào WORKDIR
SRC_URI:append=" file://am335x-boneblack.dts"

# Sau khi TI chạy do_configure xong, mình chép đè am335x-boneblack.dts bằng file của mình
KERNEL_DEVICETREE:append:am335x-evm = " am335x-boneblack.dtb"

# Sau khi kernel unpack/configure, copy DTS vào đúng chỗ trong source tree
do_configure:append:am335x-evm () {
    install -m 0644 ${WORKDIR}/am335x-boneblack.dts ${S}/arch/arm/boot/dts/
}

