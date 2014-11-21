#include <bson.h>
#include <mongoc.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include "DataExtract.h"
#include "jsmn/jsmn.h"


#define COLS 999 

static int verbose_flag;

void print_usage() {
    printf("Usage: coll2tde [v] -h HOST -d DATABASE -c COLLECTION -s "
           "COLUMNS -o TDEFILE\n");
}

/* Wrap all mongodb stuff in a function */
mongoc_cursor_t *
get_cursor(char *host, char *db, char *collection_name, bson_t *pquery,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p){
    
    mongoc_cursor_t *cursor;
    *client_p = mongoc_client_new("mongodb://localhost:27017/");
    *collection_p = mongoc_client_get_collection (*client_p, "test", "test");
    cursor = mongoc_collection_find(*collection_p, MONGOC_QUERY_NONE, 
                                    0, 0, 0, pquery, NULL, NULL);
    return cursor;
}


/* Wrap all mongodb stuff in a function */
mongoc_cursor_t *
get_one(char *host, char *db, char *collection_name, bson_t *pquery,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p){
    
    mongoc_cursor_t *cursor;
    *client_p = mongoc_client_new("mongodb://localhost:27017/");
    *collection_p = mongoc_client_get_collection (*client_p, "test", "test");
    cursor = mongoc_collection_find(*collection_p, MONGOC_QUERY_NONE, 
                                    0, -1, 0, pquery, NULL, NULL);
    return cursor;
}

int
main (int   argc, char *argv[]){
    int opt = 0;
    char *host = NULL;
    char *database = NULL;
    char *collection_name = NULL;
    char *filename = NULL;
    char *columns = NULL;
    
    static struct option long_options[] = {
        {"host",      required_argument,       0,  'h' },
        {"database", no_argument,       0,  'd' },
        {"collection",    required_argument, 0,  'c' },
        {"columns",   required_argument, 0,  COLS },
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
   

    printf("filename: %s\n", filename);
    
    mongoc_client_t *client_p;
    mongoc_collection_t *collection_p;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query = NULL;
    char *str;
    int t, r;
    jsmntok_t tokens[13];
    jsmn_parser parser;
    jsmn_init(&parser);
    mongoc_init();
    query = bson_new ();
    cursor = get_one(host, database, collection_name, query, 
             &collection_p, &client_p);
    /*do all the tde configuration of tde here */
    while (mongoc_cursor_next (cursor, &doc)) {
        str = bson_as_json (doc, NULL);
        printf ("%s\n", str);
	    r = jsmn_parse(&parser, str, strlen(str), tokens, 13);
        printf("tokens: %d\n", r - 1);
        int skip;
        for (t=0; t<r; t++){
            printf("token %d type %d\n", t, tokens[t].type);
            if (tokens[t].type == JSMN_STRING){
                int size_of_token = tokens[t].end-tokens[t].start;
                char *item = (char *)malloc((size_of_token+1)*sizeof(char));
                strncpy(item, &str[tokens[t].start], size_of_token);
                item[size_of_token] = '\0'; 
                if (! strcmp(item, "_id") || ! strcmp(item, "$oid")){
                    skip = 1;
                    continue;
                }
                if (skip) {
                    skip = 0;
                    continue;
                }
                if (t % 2) {
                    printf("%d key! %s\n", t, item);
                }else{  
                printf("%d value %s\n",t, item);
                } 
                free(item);
                printf("freed item...\n");
            } else {
              printf("Not a key, probably a value..." 
                     "let's try creating a column type\n");
            }
        }
    }
    /* do all the fun inserting data here ...*/
    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    mongoc_collection_destroy(collection_p);
    mongoc_client_destroy(client_p);
    mongoc_cleanup();
    return 0;
}
