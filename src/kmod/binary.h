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

#ifndef BINARY_H
#define BINARY_H

#include "config.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bitstring.h>
#include <sys/elf.h>
#include <sys/eventhandler.h>
#include <sys/exec.h>
#include <sys/jail.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/lock.h>
#include <sys/loginclass.h>
#include <sys/malloc.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/imgact.h>

MALLOC_DECLARE(BINARY_BOOT);
extern struct image_args args;
extern enum uio_seg segflg;


int           boot_binary(char *path);
int           fork_process(struct thread * thread_to_fork, int *pid);
char *        get_binary_path(char * path);
int           execute_binary(struct thread * td);
struct thread * find_first_thread(pid_t pid);
int exec_copyin_args_custom(struct image_args *args, const char *fname, enum uio_seg segflg, char **argv, char **envv);
int exec_args_add_str(struct image_args *args, const char *str, enum uio_seg segflg, int *countp);
int exec_args_add_arg(struct image_args *args, const char *argp, enum uio_seg segflg);
int exec_args_add_env(struct image_args *args, const char *envp, enum uio_seg segflg);
int exec_args_add_fname(struct image_args *args, const char *fname, enum uio_seg segflg);
#endif /* BINARY_H */
