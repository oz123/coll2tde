#include <mongoc.h>
#include "log.h"

mongoc_cursor_t *
get_cursor(char *host, char *db, char *collection_name, 
           const char *json_query, 
           const char *json_fields,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p);


bson_t * parse_json(const char * json_fields);
bson_t * parse_aggregation(char * aggregation);
