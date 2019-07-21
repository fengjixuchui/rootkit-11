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

#ifndef DETECT_H
#define DETECT_H
// Linker Files Setup
extern struct sx kld_sx;
extern linker_file_list_t linker_files;
extern int next_file_id;

//Modules Setup
struct module {
	TAILQ_ENTRY(module)	link;	/* chain together all modules */
	TAILQ_ENTRY(module)	flink;	/* all modules in a file */
	struct linker_file	*file;	/* file which contains this module */
	int			refs;	/* reference count */
	int 			id;	/* unique id number */
	char 			*name;	/* module name */
	modeventhand_t 		handler;	/* event handler */
	void 			*arg;	/* argument for handler */
	modspecific_t 		data;	/* module specific data */
};
extern struct sx modules_sx;
typedef TAILQ_HEAD(,module) modulelist_t;
extern modulelist_t modules;
extern int nextid;


//Modules Methods
int remove_module_from_kernel(char *name);
//Linker Files Methods
int remove_linker_file(char * name);
void decrement_kernel_image_ref_count(void);

#endif /* DETECT_H */

