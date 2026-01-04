SUMMARY = "Auto bring-up CAN interface at boot"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://can-up.sh \
           file://can-up.service \
"

S = "${WORKDIR}"

inherit systemd

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/can-up.sh ${D}${sbindir}/can-up.sh

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/can-up.service ${D}${systemd_system_unitdir}/can-up.service
}

SYSTEMD_SERVICE:${PN} = "can-up.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

