#include <stdio.h>
#include <getopt.h>
#include "json.h"
#include "mongo.h"
#include "tde.h"

#define JSON_TOKENS 256
#define COLS 999 
#define AGGR 1001 

//static int verbose_flag;

void print_usage() {
    printf("Usage: coll2tde -h HOST -d DATABASE -c COLLECTION [-s "
           "COLUMNS] [-a AGGREGATION] -o TDEFILE\n");
}


int
main (int   argc, char *argv[]){
    int opt = 0;
    char *host = NULL;
    char *database = NULL;
    char *collection_name = NULL;
    char *filename = NULL;
    char *columns = NULL;
    char *aggregation = NULL;
    
    static struct option long_options[] = {
        {"host",      required_argument,       0,  'h' },
        {"database", no_argument,       0,  'd' },
        {"collection",    required_argument, 0,  'c' },
        {"columns",   required_argument, 0,  COLS },
        {"aggregation",   required_argument, 0,  AGGR },
        {"filename",   required_argument, 0,  'f' },
        {0,           0,                 0,  0   }
    };

    int long_index =0;
    while ((opt = getopt_long(argc, argv,"h:d:c:f:", 
                   long_options, &long_index )) != -1) {
        switch (opt) {
             case 'h' : host = optarg;
                 break;
             case 'd' : database = optarg;
                 break;
             case 'c' : collection_name = optarg; 
                 break;
             case 'f' : filename = optarg;
                 break;
             case COLS : columns = optarg;
                 break;
             case AGGR : aggregation = optarg;
                 break;
             default: print_usage(); 
                 exit(EXIT_FAILURE);
        }


    }
    /* if no file name is given the collection_name will be appended tde */
    printf("host: %s\n", host);
    printf("database: %s\n", database);
    printf("collection_name: %s\n", collection_name);

    if ( host == NULL || database == NULL || collection_name == NULL)  {
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    if ( filename == NULL ) {
        filename = strcat(collection_name, ".tde");  
    }
    
    if ( columns != NULL ) {
        printf("columns: %s\n", columns); 
    }
    if ( aggregation != NULL ) {
        printf("aggregation: %s\n", aggregation);
    }

    printf("filename: %s\n", filename);
    
    mongoc_client_t *client_p;
    mongoc_collection_t *collection_p;
    mongoc_cursor_t *cursor = NULL;
    const bson_t *doc;
    TAB_HANDLE hExtract;
    TAB_HANDLE hTable;
    TAB_HANDLE hTableDef;
    wchar_t *fname_w = calloc(strlen(filename) + 1, sizeof(wchar_t));
    mbstowcs(fname_w, filename, strlen(filename)+1);
    TableauWChar *sfname = calloc(strlen(filename) + 1, sizeof(TableauWChar));
    TableauWChar sExtract[8];
    ToTableauString(fname_w, sfname);
    ToTableauString(L"Extract", sExtract);
    TryOp(TabExtractCreate(&hExtract, sfname));
    char *jsstr = NULL;
    /* TODO: add option to parse aggregation, and get this cursor 
     * aggregation should be given as json and converted to BSON with 
     * bson_new_from_json
     * mongoc_collection_aggregate should than be used to get a cursor
     * */
    TAB_TYPE *column_types = NULL;
    int ncol = 0;
    int bHasTable;
    TryOp(TabExtractHasTable(hExtract, sExtract, &bHasTable));

    if (!bHasTable) {
        cursor = get_one(host, database, collection_name, &collection_p, &client_p);
        mongoc_cursor_next (cursor, &doc);
        jsstr = bson_as_json (doc, NULL);
        printf("Creating tde file: %ls\n", fname_w);
        /* Table does not exist; create it. */
        hTableDef = make_table_definition(jsstr, &column_types, &ncol);
        
        //for (int i=0 ; i < ncol ; i++)
        //    printf("type: %d\n", column_types[i]);
        
        TryOp(TabExtractAddTable(hExtract, sExtract, hTableDef, &hTable));
    }
    else {
        printf("Found existing file!\n");
        /* Open an existing table to add more rows. */
        TryOp(TabExtractOpenTable(hExtract, sExtract, &hTable));
        /* Get table definition and column_types */
        TryOp(TabTableGetTableDefinition(hTable, &hTableDef));
        get_columns(&column_types, hTableDef, &ncol);
    }

    
    printf("The length is %d\n", ncol);
    //for (int i=0; i< ncol; i++)
    //    printf("Column %d is type %d\n", i, column_types[i]);
    if ( cursor != NULL )
        mongoc_cursor_destroy(cursor);
    /* revert cursor to begining of query */
    //mongoc_cursor_t *cursor_copy;
    //cursor_copy = mongoc_cursor_clone (cursor);
    cursor = get_one(host, database, collection_name, &collection_p, &client_p);
    /* do all the fun inserting data here ...*/
    while (mongoc_cursor_next (cursor, &doc)) {
        jsstr = bson_as_json (doc, NULL);
        jsmntok_t *tokens = json_tokenise(jsstr);
        wchar_t **column_values = malloc(tokens[0].size / 2 * sizeof(wchar_t*));
        extract_values(column_values, jsstr, tokens, &ncol);
        // insert_values adds a row, column_values is actually row values
        insert_values(column_values, column_types, hTable, ncol);
    }
    
    TryOp(TabExtractClose(hExtract));
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection_p);
    mongoc_client_destroy(client_p);
    mongoc_cleanup();  

    return 0;
}
