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

TAB_HANDLE make_table_definition();

void PrintTableDefinition( TAB_HANDLE hTableDef );

void InsertData( TAB_HANDLE hTable );

void parse_keys_values(wchar_t **column_names, char **column_values, 
        TAB_TYPE *column_types, char *js, jsmntok_t *tokens);

struct tm* convert_epoch_to_gmt(char * epoch);

int check_number(char *str, int * ival, double *fval);
