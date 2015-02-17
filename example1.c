#include <bson.h>
#include <mongoc.h>
#include <stdio.h>


int
main (int   argc,
              char *argv[])
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
    //bson_type_t type; // we can use that, but it's quite redundant.
    mongoc_init ();

    client = mongoc_client_new ("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection (client, "test", "test");
    query = bson_new ();
    cursor = mongoc_collection_find (collection, MONGOC_QUERY_NONE, 0,
                                     0, 0, query, NULL, NULL);

    while (mongoc_cursor_next (cursor, &doc)) {
        str = bson_as_json (doc, NULL);
        if (bson_iter_init (&iter, doc)) {
            while (bson_iter_next (&iter)) {
                printf ("Found element key: \"%s\"\n", bson_iter_key (&iter));
                value = bson_iter_value(&iter);
                /* change this to switch ... case ...
                 * I've be doing python too ... long ...*/
                if ((*value).value_type == BSON_TYPE_UTF8) {
                    printf("(UTF-8)\nThe value is %s\n", (*value).value.v_utf8.str);
                } else if ((*value).value_type == BSON_TYPE_DOCUMENT ||
                            (*value).value_type == BSON_TYPE_ARRAY) {
                    /* we can properly detect array and documents too ...*/
                    printf("buah! exploding with loud crash, a table can't tollerate document or array as a value!\n");
                } else if ((*value).value_type == BSON_TYPE_OID) {
                /* ignore objectid */
                printf("dying silently on oid...\n");
                }
            }
        }
        bson_free(str);
    }

    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);

    return 0;
}
