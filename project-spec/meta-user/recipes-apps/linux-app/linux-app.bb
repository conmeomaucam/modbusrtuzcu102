#
# This file is the linux-app recipe.
#

SUMMARY = "Linux A53 Embedded POSIX C Application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://core/msq_queue.c \
           file://core/msq_queue.h \
           file://core/data_types.h \
           file://drivers/rpmsg_driver.c \
           file://drivers/rpmsg_driver.h \
           file://Threads/rpmsg.c \
           file://Threads/rpmsg.h \
           file://Threads/GUI.c \
           file://Threads/GUI.h \
           file://Makefile \
          "

S = "${WORKDIR}"

do_compile() {
	oe_runmake LDFLAGS="${LDFLAGS}"
}


do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/build/linux_app ${D}${bindir}/linux_app
}

