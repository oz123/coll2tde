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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <time.h>
#include <regex.h>
#include <bson.h>
#include "DataExtract.h"
#include "log.h"
#include "jsmn/jsmn.h"
#include "json.h"


#define COLS 999

#define TryOp(x) if (x != TAB_RESULT_Success) { \
    fprintf(stderr, "Error: %ls\n", TabGetLastErrorMessage()); \
    exit( EXIT_FAILURE ); }

TAB_HANDLE make_table_definition(char *js, TAB_TYPE **column_types, int *ncols);

void PrintTableDefinition( TAB_HANDLE hTableDef );

void InsertData( TAB_HANDLE hTable );

/*
 * Do the same as parse keys values but instead of using json,
 * we work directly with libbson's functions */
void get_keys_values(wchar_t **column_names, TAB_TYPE *column_types,
                     const bson_t *doc);

void parse_keys_values(wchar_t **column_names, TAB_TYPE *column_types,
                       char *js, jsmntok_t *tokens);

struct tm* convert_epoch_to_localtime(char * epoch);

int check_number(char *str, int * ival, double *fval);


void extract_values(wchar_t **column_values, char *js, jsmntok_t *tokens,
                    int *ncols);

void insert_values(wchar_t **record_values, TAB_TYPE *column_types,
                   TAB_HANDLE *hTable, int rec_size);

void get_columns(TAB_TYPE **column_types, TAB_HANDLE hTableDef, int *ncols);
