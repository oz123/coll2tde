#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "DataExtract.h"
#include "log.h"
#include "jsmn/jsmn.h"
#include "json.h"

#define COLS 999 

#define TryOp(x) if (x != TAB_RESULT_Success) { \
    fprintf(stderr, "Error: %ls\n", TabGetLastErrorMessage()); \
    exit( EXIT_FAILURE ); }

TAB_HANDLE MakeTableDefinition();
TAB_HANDLE make_table_definition();
void PrintTableDefinition( TAB_HANDLE hTableDef );
void InsertData( TAB_HANDLE hTable );
void parse_keys_values(char **column_names, char **column_values, char *js, 
                       jsmntok_t *tokens);

