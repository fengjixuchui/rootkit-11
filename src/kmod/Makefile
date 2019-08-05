#
# Copyright (c) 2019 Jasper Lowell
# Copyright (c) 2019 Wesley Hamburger
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

KMOD=rootkit
SRCS=rootkit.c privilege.c hook.c hide.c vnode_if.h

load:
	sudo kldload ./$(KMOD).ko

unload:
	sudo kldunload $(KMOD).ko

.include <bsd.kmod.mk>
