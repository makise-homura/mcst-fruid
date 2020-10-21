SUMMARY = "MCST FRU ID retriever"
DESCRIPTION = "MCST FRU ID retriever"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

DEPENDS += "zlib i2c-tools"
RDEPENDS_${PN} += "dtc"

SRC_URI = "file://mcst-fruid.c \
           file://Makefile \
           file://LICENSE \
          "

S = "${WORKDIR}"

do_install() {
  install -d ${D}/usr/sbin
  install -m 755 mcst-fruid ${D}/usr/sbin
}

FILES_${PN} = "/usr/sbin"
