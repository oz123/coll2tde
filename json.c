#include "json.h"

#define JSON_TOKENS 256


/* JSON parsers and helpers */
jsmntok_t * json_tokenise(char *js)
{
    jsmn_parser parser;
    jsmn_init(&parser);

    unsigned int n = JSON_TOKENS;
    jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * n);
    
    int ret = jsmn_parse(&parser, js, strlen(js), tokens, JSON_TOKENS);
    
    while (ret == JSMN_ERROR_NOMEM)
    {
        n = n * 2 + 1;
        tokens = realloc(tokens, sizeof(jsmntok_t) * n);
        //log_null(tokens);
        ret = jsmn_parse(&parser, js, strlen(js), tokens, n);
    }

    // if (ret == JSMN_ERROR_INVAL)
    //if (ret == JSMN_ERROR_PART)

    return tokens;
}

