#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "jsmn/jsmn.h"

/* JSON parsers and helpers */
jsmntok_t * json_tokenise(char *js);
bool json_token_streq(char *js, jsmntok_t *t, char *s);
char * json_token_tostr(char *js, jsmntok_t *t);

