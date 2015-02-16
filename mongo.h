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

#include <mongoc.h>
#include <bson.h>
#include "log.h"

mongoc_cursor_t *
get_cursor(char *host, char *db, char *collection_name, 
           const char *json_query, 
           const char *json_fields,
           const char *json_aggregation,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p);


bson_t * parse_json(const char * json_fields);
bson_t * parse_aggregation(char * aggregation);
