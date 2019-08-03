#include <binary.h>
#include <debug.h>

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
#include <sys/libkern.h>

int boot_binary(char *path){
	char * boot_path = strcat(BIN_PATH, path);
	// struct proc * p = find_process(1);
	// if(p != 0){
	// 	LOGI("Found process with pid %d\n", p->p_pid);
	// }
	LOGI("Attempting to boot %s\n", boot_path);
	boot_path = 0x00;


	return 1;
}

struct proc * find_process(pid_t pid)
{	
	struct proc *p;	
	p = 0x00;
	LOGI("Attemping to find process with ID %d\n", pid);

	sx_slock(&allproc_lock);
	
	/* Two options. Iterate over pidhashtabl, or allproc_list. 
	*  Better performance over pidhashtabl */
	LIST_FOREACH(p, PIDHASH(pid), p_list){
		LOGI("Checking PID:%d\n", p->pid)
		if(p->p_pid == pid)
		{
			/* A process is either NEW, NORMAL, ZOMBIE 
			*  ( new means in creation , see proc.h ) */
			if(p->p_state == PRS_NEW)
			{ 
				p = NULL;
				break;
			}
			LOGI("Found PID, Removing from p_list, p_hash\n")
			break;
		}
	}

	sx_sunlock(&allproc_lock);
	return p;
}






