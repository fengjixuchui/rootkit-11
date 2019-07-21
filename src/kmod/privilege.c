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

#include "privilege.h"
#include "config.h"

#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/proc.h>

void privilege_set(struct thread *td, int id)
{
	td->td_ucred->cr_uid = id;
	td->td_ucred->cr_ruid = id;
	td->td_ucred->cr_svuid = id;
	td->td_ucred->cr_rgid = id;
	td->td_ucred->cr_svgid = id;
}
