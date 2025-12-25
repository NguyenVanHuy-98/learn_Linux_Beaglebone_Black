SUMMARY = "QT App to simulate tapto in car"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2e7f3427ab08fda49476f7eec09fe84c"

SRC_URI = "git://github.com/NguyenVanHuy-98/QT_app_taplo.git;protocol=https;branch=main"
SRCREV = "8441625aa50251528a62cc62be3fe4bcae241fed"
S = "${WORKDIR}/git"

inherit qmake5
DEPENDS += "qtbase"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/taplo-gui ${D}${bindir}/taplo-gui
}
