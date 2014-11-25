#include <mongoc.h>

mongoc_cursor_t *
get_one(char *host, char *db, char *collection_name, 
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p);

