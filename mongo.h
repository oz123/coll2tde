#include <mongoc.h>
#include "log.h"

mongoc_cursor_t *
get_one(char *host, char *db, char *collection_name, const char *json_fields,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p);


bson_t * parse_columns(const char * json_fields);
bson_t * parse_aggregation(char * aggregation);
