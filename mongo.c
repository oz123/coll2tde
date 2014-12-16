#include "mongo.h"
/* Wrap all mongodb stuff in a function */

/* get_one never returns _id */
mongoc_cursor_t *
get_one(char *host, char *db, char *collection_name, const char *json_fields, 
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p){
    mongoc_init();
    bson_t *query = NULL;
    bson_t *fields = NULL;
    query = bson_new ();
    fields = bson_new ();
    fields = BCON_NEW("_id", BCON_INT32 (0));

    if (json_fields){
        bson_concat(fields, parse_columns(json_fields));
        }
    
    mongoc_cursor_t *cursor;
    *client_p = mongoc_client_new("mongodb://localhost:27017/");
    *collection_p = mongoc_client_get_collection (*client_p, "test", "test");
    cursor = mongoc_collection_find(*collection_p, MONGOC_QUERY_NONE, 
                                    0, 0, 0, query, fields, NULL);
    bson_destroy(query);
    bson_destroy(fields);
    return cursor;

}

bson_t * parse_json(const char * json_fields){

    bson_error_t error;
    bson_t *n_fields = bson_new_from_json((unsigned char *)json_fields, -1, &error);
    if (!n_fields) {
        log_die("Error: %s\n", error.message);
    } else {
      return n_fields;
    }
    log_die("Error: something bad happend in parse_columns");
    return n_fields; // this should never be reached ...
}

bson_t * parse_aggregation(char * aggregation){

    bson_t *aggregation_bson = NULL;
    aggregation_bson = bson_new ();
    
    return aggregation_bson;
}
