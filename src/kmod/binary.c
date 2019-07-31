#include <utils.h>
#include <binary.h>
#include <debug.h> 
#include <sys/proc.h>

void boot_binary(char *path){	
	char * boot_path = strcat(BINARY_PATH, path);
	struct proc * p = _pfind(1, 0);

	LOGI("Attempting to boot %s\n", boot_path);
	return;
}

struct proc * _pfind(pid_t pid, bool zombie)
{
	LOGI("Attemping to find process with ID %d\n", pid);

	struct proc *p;

	p = curproc;
	if (p->p_pid == pid) {
		PROC_LOCK(p);
		return (p);
	}

	sx_slock(PIDHASHLOCK(pid));
	LIST_FOREACH(p, PIDHASH(pid), p_hash) {
		if (p->p_pid == pid) {
			PROC_LOCK(p);
			if (p->p_state == PRS_NEW ||
			    (!zombie && p->p_state == PRS_ZOMBIE)) {
				PROC_UNLOCK(p);
				p = NULL;
			}
			LOGI("Found process returning ..\n");
			break;
		}
	}
	sx_sunlock(PIDHASHLOCK(pid));
	return (p);
}






