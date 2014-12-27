/* 
 * This file is part of coll2tde.
 *
 * coll2tde is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 3
 * as published by the Free Software Foundation;
 *
 * coll2tde is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with coll2tde; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ============================================================================
 * Copyright (C) 2014 Oz Nahum Tiram <nahumoz@gmail.com>
 * ============================================================================
 */

#pragma once

#include <string.h>
#include <errno.h>

#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define LINESTR STRINGIFY(__LINE__)

#define log_assert(x) if (!(x)) log_die("%s: assertion failed: " #x \
                                        " (" __FILE__ ", line " LINESTR \
                                        ")", __func__)

#define log_null(x) log_assert(x != NULL)

#define log_debug(msg, ...) log_info("%s: " msg, __func__, ##__VA_ARGS__)

#define log_syserr(msg) log_die("%s: %s: %s", __func__, msg, strerror(errno))

void log_die(char *msg, ...);
void log_info(char *msg, ...);
