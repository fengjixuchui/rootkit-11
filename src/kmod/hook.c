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

void
hook_syscall_set(int syscall, void *func)
{
	sysent[syscall].sy_call = (sy_call_t *) func;
}
