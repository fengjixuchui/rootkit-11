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

#include "config.h"
#include "debug.h"

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>

static int
load(struct module *module, int cmd, void *arg)
{
        switch(cmd) {
        case MOD_LOAD:
                LOGI("[rootkit:load] Rootkit loaded.\n");
                break;
        case MOD_UNLOAD:
                LOGI("[rootkit:load] Rootkit unloaded.\n");
                break;
        default:
                LOGE("[rootkit:load] Unsupported event: {%d}.\n", cmd);
                break;
        }

        return(0);
}

static moduledata_t rootkit_mod = {
        MODULE_NAME,
        load,
        NULL
};

DECLARE_MODULE(rootkit, rootkit_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
