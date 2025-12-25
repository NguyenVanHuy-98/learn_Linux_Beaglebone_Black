FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI:append = " \
    file://beagleboneblack-defconfig \
    file://pru.cfg \
"

KERNEL_DEFCONFIG:am335x-evm = "beagleboneblack-defconfig"

do_configure:append() {
    echo ">>> Copy kernel config fragments"
    cp ${WORKDIR}/pru.cfg ${B}/

    echo ">>> Using beagleboneblack-defconfig from meta-myapps"
    cp ${WORKDIR}/beagleboneblack-defconfig ${B}/.config

    oe_runmake olddefconfig
}
