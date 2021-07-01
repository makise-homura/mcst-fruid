SUMMARY = "MCST FRU ID retriever"
DESCRIPTION = "MCST FRU ID retriever"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

DEPENDS += "i2c-tools"
RDEPENDS_${PN} += "dtc"

SRC_URI = "git://github.com/makise-homura/mcst-fruid.git;protocol=https"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git"

do_install() {
  install -d ${D}/usr/libexec
  install -m 755 mcst-fruid ${D}/usr/libexec
}

FILES_${PN} = "/usr/libexec/mcst-fruid"
