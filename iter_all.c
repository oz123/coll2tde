#include <bson.h>
#include <mongoc.h>
#include <stdio.h>


bson_iter_t *
iter_recursively(bson_iter_t *doc){

    bson_iter_t sub_iter;
    const bson_value_t * value;
    printf ("Key %s\n", bson_iter_key (doc));
    value = bson_iter_value(doc);
    switch ((*value).value_type){
        case BSON_TYPE_UTF8:
            printf("String %s\n", (*value).value.v_utf8.str);
            break;

        case BSON_TYPE_DOCUMENT:
             bson_iter_recurse (doc, &sub_iter);
             while (bson_iter_next (&sub_iter)) {
                iter_recursively(&sub_iter);
            }
            break;

        case BSON_TYPE_ARRAY:
            printf("Array!\n");
            while (bson_iter_next (&sub_iter)) {
                    iter_recursively(&sub_iter);
            }
            break;

        case BSON_TYPE_INT32:
            printf("Int %d!\n", bson_iter_int32(doc));
            break;

        case BSON_TYPE_DOUBLE:
            printf("Double %f!\n", bson_iter_double(doc));
            break;

        default:
            printf("Don't know what we found %d!\n", (*value).value_type);
        }
}

int
main (int   argc, char *argv[])
{
    const bson_t *doc;
    const bson_value_t *value;
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;

    bson_t *query;
    char *str;
    /* bson_iter_t iter - is meant to be used on the stack and can be
     * discarded at any time as it contains no external allocation.
     */
    bson_iter_t iter;
    bson_iter_t sub_iter;
    bson_iter_t sub_sub_iter;
    //bson_type_t type; // we can use that, but it's quite redundant.
    mongoc_init ();

    client = mongoc_client_new ("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection (client, "testtde", "test");
    query = bson_new ();
    cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0,
                                     0, 0, query, NULL, NULL);

    while (mongoc_cursor_next (cursor, &doc)) {
        str = bson_as_json (doc, NULL);
        if (bson_iter_init (&iter, doc)) {
            while (bson_iter_next (&iter)) {
                iter_recursively(&iter);
            }
        bson_free(str);
        }
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    mongoc_client_destroy(client);

    return 0;
}
