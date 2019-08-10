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

#include "hook.h"
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
#include <sys/malloc.h>

#define PATCH_NBYTES 5

static void
patch(void *func, void *hook)
{
	int diff;
	char *ptr;

	ptr = (char*) func;
	diff = (int) (((char *) hook - ptr) - PATCH_NBYTES);

	*ptr = 0xe9;
	*(int *) (ptr + 1) = diff;
}

char *
hook_fetch(void *func)
{
	char *buf;
	buf = malloc(PATCH_NBYTES, M_TEMP, M_WAITOK);

	memcpy(buf, func, PATCH_NBYTES);
	return(buf);
}

void
hook_set(void *func, char *bytes)
{
	memcpy(func, bytes, PATCH_NBYTES);
}

void
hook(void *func, void *hook)
{
	patch(func, hook);
}
