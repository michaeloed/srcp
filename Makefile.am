# Makefile.am for srcpd
# version: $Revision: 1773 $
# last update: $Date: 2023-03-06 15:53:47 +0100 (Mo, 06. Mär 2023) $

SUBDIRS = src man

EXTRA_DIST = AUTHORS COPYING TODO NEWS ChangeLog INSTALL DESIGN \
	README README.freebsd README.loconet README.selectrix \
	PROGRAMMING-HOWTO srcpd.lsm srcpd.spec srcpd.redhat.spec \
        init.d.suse init.d.redhat init.d.lsb srcpd.service

dist_sysconf_DATA = srcpd.conf

udevdir = $(sysconfdir)/udev/rules.d
dist_udev_DATA = 10-liusb.rules
