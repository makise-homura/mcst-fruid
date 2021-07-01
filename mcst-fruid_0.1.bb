SUMMARY = "MCST FRU ID retriever"
DESCRIPTION = "MCST FRU ID retriever"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

inherit systemd
inherit features_check

REQUIRED_DISTRO_FEATURES = "systemd"
SYSTEMD_SERVICE_${PN} = "mcst-fruid.service"

DEPENDS += "i2c-tools"
RDEPENDS_${PN} += "systemd dtc"

SRC_URI = "git://github.com/makise-homura/mcst-fruid.git;protocol=https"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

do_install() {
  install -d ${D}/libexec
  install -d ${D}${systemd_system_unitdir}
  install -m 755 mcst-fruid ${D}/libexec
  install -m 644 ${S}/mcst-fruid.service ${D}${systemd_system_unitdir}
}

FILES_${PN} = "/libexec/mcst-fruid ${systemd_system_unitdir}"
