#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
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

void parse_keys_values(wchar_t **column_names, TAB_TYPE *column_types, 
        char *js, jsmntok_t *tokens);

struct tm* convert_epoch_to_gmt(char * epoch);

int check_number(char *str, int * ival, double *fval);


void extract_values(char **column_values, char *js, jsmntok_t *tokens);

void insert_values(char **record_values, TAB_TYPE **column_types, 
        TAB_HANDLE *hTable, TAB_HANDLE *hRow);
