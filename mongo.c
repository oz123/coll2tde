#include "mongo.h"
/* Wrap all mongodb stuff in a function */

 /* aggregation_json should begin with pipeline and contain array of
 * operations:
 * { "pipeline" : [ { "$project" : { "name" : 1 } }, { "$group" : 
 *                  { "_id" : "$name", "credit" : { "$sum" : 1 } } }
 *                ] 
 *  }
 * */
/* get_one never returns _id */
mongoc_cursor_t *
get_cursor(char *host, char *db, char *collection_name, 
           const char *json_query,
           const char *json_fields, 
           const char *json_aggregation,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p){
    mongoc_init();
    bson_t *query = NULL;
    bson_t *fields = NULL;
    query = bson_new ();
    fields = BCON_NEW("_id", BCON_INT32 (0));
    mongoc_cursor_t *cursor;
    char *host_uri = malloc(10);
    strcpy(host_uri, "mongodb://");
    strcat(host_uri, host);  
    printf("Trying to connect to %s\n", host_uri);
    *client_p = mongoc_client_new(host_uri);
    *collection_p = mongoc_client_get_collection (*client_p, db, collection_name);
    
    if (json_aggregation) {
        bson_error_t error;
        bson_t *pipeline = bson_new_from_json((unsigned char *)json_aggregation, -1, &error);
        cursor = mongoc_collection_aggregate (*collection_p, MONGOC_QUERY_NONE, 
                                              pipeline, NULL, NULL);
    } else {
    
        if (json_fields){
            bson_t *new_fields = parse_json(json_fields);
            bson_concat(fields, new_fields);
            bson_destroy(new_fields);
            }
        if (json_query){
            bson_t *new_query = parse_json(json_query);
            bson_concat(query, new_query);
            bson_destroy(new_query);
            }
        
        cursor = mongoc_collection_find(*collection_p, MONGOC_QUERY_NONE, 
                                        0, 0, 0, query, fields, NULL);
    }
    
    free(host_uri);
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
