#include "mongo.h"
/* Wrap all mongodb stuff in a function */

/* get_one never returns _id */
mongoc_cursor_t *
get_one(char *host, char *db, char *collection_name, bson_t *pquery,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p){
    
    bson_t *fields = NULL;
    fields = bson_new ();
    fields = BCON_NEW("_id", BCON_INT32 (0));

    mongoc_cursor_t *cursor;
    *client_p = mongoc_client_new("mongodb://localhost:27017/");
    *collection_p = mongoc_client_get_collection (*client_p, "test", "test");
    cursor = mongoc_collection_find(*collection_p, MONGOC_QUERY_NONE, 
                                    0, -1, 0, pquery, fields, NULL);
    return cursor;

}
