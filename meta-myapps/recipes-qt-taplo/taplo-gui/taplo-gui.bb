SUMMARY = "QT App to simulate tapto in car"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=2e7f3427ab08fda49476f7eec09fe84c"

SRC_URI = "git://github.com/NguyenVanHuy-98/QT_app_taplo.git;protocol=https;branch=main"
SRCREV  = "a7a346829ff9e8a3c9c4a284f8051568f74d7960"
S = "${WORKDIR}/git"

inherit qmake5
DEPENDS += "qtbase"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/taplo-gui ${D}${bindir}/taplo-gui
}
