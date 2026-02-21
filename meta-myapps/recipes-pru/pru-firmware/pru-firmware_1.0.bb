SUMMARY = "AM335x PRU0 firmware (encoder + PWM)"
LICENSE = "CLOSED"

inherit allarch

SRC_URI = "file://pru0_enc_pwm.out"
S = "${WORKDIR}"

do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware
    install -m 0644 ${WORKDIR}/pru0_enc_pwm.out \
        ${D}${nonarch_base_libdir}/firmware/am335x-pru0-fw
}

FILES:${PN} += "${nonarch_base_libdir}/firmware/am335x-pru0-fw"

# PRU firmware là ELF của PRU, không phải ARM -> skip QA arch check
INSANE_SKIP:${PN} += "arch"

# (khuyến nghị) không strip/debug split firmware
INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"