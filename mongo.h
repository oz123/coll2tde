#include <mongoc.h>

mongoc_cursor_t *
get_one(char *host, char *db, char *collection_name, 
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p);


bson_t * parse_columns(char * columns);
bson_t * parse_aggregation(char * aggregation);
