#include <stdio.h>
#include <getopt.h>
#include "json.h"
#include "mongo.h"
#include "tde.h"

#define FIELDS 999 
#define HELP 998 


void print_usage() {
    printf("Usage: coll2tde -h HOST -d DATABASE -c COLLECTION [-q QUERY][--fields "
           "FIELDS] [-a AGGREGATION] -f TDEFILE\n");
}

void get_options(int argc, char *argv[], char **host, char **database,
    char **collection_name, char **filename, char **fields, char **query, 
    char **aggregation) {

    int opt = 0;
    int long_index = 0;
    
    static struct option long_options[] = {
        {"host",      required_argument,       0,  'h' },
        {"database", required_argument,       0,  'd' },
        {"collection",    required_argument, 0,  'c' },
        {"fields",   required_argument, 0,  COLS },
        {"query",   required_argument, 0,  'q' },
        {"aggregation",   required_argument, 0,  'a' },
        {"filename",   required_argument, 0,  'o' },
        {"help",   no_argument, 0,  HELP },
        {0,           0,                 0,  0   }
    };

    while ((opt = getopt_long(argc, argv,"h:d:c:f:q:a:", 
                   long_options, &long_index )) != -1) {
        switch (opt) {
             
            case 'h' : *host = optarg;
                 break;
             case 'd' : *database = optarg;
                 break;
             case 'c' : *collection_name = optarg; 
                 break;
             case 'f' : *filename = optarg;
                 break;
             case 'q' : *query = optarg;
                 break;
             case FIELDS: *fields = optarg;
                 break;
             case 'a' : *aggregation = optarg;
                 break;
             default: print_usage(); 
                 exit(EXIT_FAILURE);
        }
    }
    
    if ( *host == NULL || *database == NULL || *collection_name == NULL)  {
        print_usage();
        exit(EXIT_FAILURE);
        }

    /* if no file name is given the collection_name will be appended tde */
    if (*filename == NULL) {
        *filename = malloc(strlen(*collection_name));
        strcpy(*filename, *collection_name);
        strcat(*filename, ".tde");  
    }
    
    if ( *query && *aggregation ) {
        printf("Aggregation and query are mutually exclusive...\n");
        exit(EXIT_FAILURE);
    }

    
}

int
main (int argc, char *argv[]){
    

    char *host = NULL;
    char *database = NULL;
    char *collection_name = NULL;
    char *filename = NULL;
    char *fields = NULL;
    char *query = NULL;
    char *aggregation = NULL;
    
    get_options(argc, argv, &host, &database, &collection_name, &filename, 
            &fields, &query, &aggregation);
    
    mongoc_client_t *client_p;
    mongoc_collection_t *collection_p;
    mongoc_cursor_t *cursor = NULL;
    const bson_t *doc;
    TAB_HANDLE hExtract;
    TAB_HANDLE hTable;
    TAB_HANDLE hTableDef;
    wchar_t *fname_w = calloc(strlen(filename) + 1, sizeof(wchar_t));
    char *jsstr = NULL;
    TAB_TYPE *column_types = NULL;
    int ncol = 0;
    int r = 0;
    int bHasTable;
    
    mbstowcs(fname_w, filename, strlen(filename)+1);
    TableauWChar *sfname = calloc(strlen(filename) + 1, sizeof(TableauWChar));
    TableauWChar sExtract[8];
    ToTableauString(fname_w, sfname);
    ToTableauString(L"Extract", sExtract);
    TryOp(TabExtractCreate(&hExtract, sfname));
    free(fname_w);
    free(sfname);
    free(filename);
     /* aggregation_json should begin with pipeline and contain array of
     * operations:
     * { "pipeline" : [ { "$project" : { "name" : 1 } }, { "$group" : 
     *                  { "_id" : "$name", "credit" : { "$sum" : 1 } } }
     *                ] 
     *  }
     * */
    TryOp(TabExtractHasTable(hExtract, sExtract, &bHasTable));
    cursor = get_cursor(host, database, collection_name, query, fields,  
                        aggregation, &collection_p, &client_p);
    mongoc_cursor_next (cursor, &doc);
    if (!bHasTable) {
        jsstr = bson_as_json (doc, NULL);
        printf("Creating tde file: %ls\n", fname_w);
        /* Table does not exist; create it. */
        hTableDef = make_table_definition(jsstr, &column_types, &ncol);
        TryOp(TabExtractAddTable(hExtract, sExtract, hTableDef, &hTable));
    } else {
        printf("Found existing file!\n");
        /* Open an existing table to add more rows. */
        TryOp(TabExtractOpenTable(hExtract, sExtract, &hTable));
        /* Get table definition and column_types */
        TryOp(TabTableGetTableDefinition(hTable, &hTableDef));
        get_columns(&column_types, hTableDef, &ncol);
    }
    
    /* insert the first row */
    jsstr = bson_as_json (doc, NULL);
    jsmntok_t *tokens = json_tokenise(jsstr);
    wchar_t **column_values = malloc(tokens[0].size / 2 * sizeof(wchar_t*));
    extract_values(column_values, jsstr, tokens, &ncol);
    insert_values(column_values, column_types, hTable, ncol);
    r++;
    
    /* insert the remaining rows */
    while (mongoc_cursor_next (cursor, &doc)) {
        jsstr = bson_as_json (doc, NULL);
        jsmntok_t *tokens = json_tokenise(jsstr);
        extract_values(column_values, jsstr, tokens, &ncol);
        // insert_values adds a row, column_values is actually row values
        insert_values(column_values, column_types, hTable, ncol);
        r++;
    }
    
    printf("Successfully inserted %d rows\n", r);
    TryOp(TabExtractClose(hExtract));
    free(tokens);
    free(column_values);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection_p);
    mongoc_client_destroy(client_p);
    mongoc_cleanup();  

    return 0;
}
