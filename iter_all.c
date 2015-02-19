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
                printf ("Found element key: \"%s\"\n", bson_iter_key (&iter));
                value = bson_iter_value(&iter);
                switch ((*value).value_type){
                case BSON_TYPE_UTF8:
                    printf("(UTF-8)\nThe value is %s\n", (*value).value.v_utf8.str);
                    break;
                case BSON_TYPE_DOCUMENT:
                     printf("Yes! We found a document embedded key [%s]!\n", bson_iter_key(&iter));
                     bson_iter_recurse (&iter, &sub_iter);
                     while (bson_iter_next (&sub_iter)) {
                        printf ("Found YYY key \"%s\" in sub document.\n",
                        bson_iter_key (&sub_iter));
                    }
                    break;
                case BSON_TYPE_ARRAY:
                    printf("Yes! We found an array embedded!\n");
                    bson_iter_recurse (&iter, &sub_iter);
                    while (bson_iter_next (&sub_iter)) {
                        printf ("Found XXX key \"%s\" in sub document.\n",
                        bson_iter_key (&sub_iter));
                    }
                    break;
                case BSON_TYPE_INT32:
                    printf("We found an int %d!\n", bson_iter_int32(&iter));
                    break;
                case BSON_TYPE_DOUBLE:
                    printf("We found a double %f!\n", bson_iter_double(&iter));

                    break;
                default:
                    printf("Don't now what we found %d!\n", (*value).value_type);
                }
            }
        bson_free(str);
        }
    }
    bson_destroy (query);
    mongoc_cursor_destroy (cursor);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);

    return 0;
}
