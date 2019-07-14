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

#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_FATAL 3
#define LOG_LEVEL_OMIT  4

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define LOGF(...) printf(__VA_ARGS__)
#else
#define LOGF
#endif /* LOG_LEVEL <= LOG_LEVEL_FATAL */

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOGE(...) printf(__VA_ARGS__)
#else
#define LOGE
#endif /* LOG_LEVEL <= LOG_LEVEL_ERROR */

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOGI(...) printf(__VA_ARGS__)
#else
#define LOGI
#endif /* LOG_LEVEL <= LOG_LEVEL_INFO */

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOGD(...) printf(__VA_ARGS__)
#else
#define LOGD
#endif /* LOG_LEVEL <= LOG_LEVEL_DEBUG */

#endif /* DEBUG_H */
