SUMMARY = "MCST FRU ID retriever"
DESCRIPTION = "MCST FRU ID retriever"
SECTION = "base"
PR = "r1"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b234ee4d69f5fce4486a80fdaf4a4263"

DEPENDS += "zlib i2c-tools"
RDEPENDS_${PN} += "dtc"

SRC_URI = "https://github.com/makise-homura/mcst-fruid"
SRC_URI[sha256sum] = "5a0c00ec6ac94bbefaf027315477dab0251183f1adc5313b3b2003416178ce70"

S = "${WORKDIR}"

FILES_${PN} = "/usr/libexec/mcst-fruid"
