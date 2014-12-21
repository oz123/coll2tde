#define _XOPEN_SOURCE
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>
#include <wchar.h>
#include "DataExtract.h"
#include "tde.h"



#define FORMAT_DATETIME "%Y-%m-%d %H:%M:%S"
#define FORMAT_DATE     "%Y-%m-%d"

/* 
 * Should be able to query like this ...
"{$project: {_id:0, "hobbies":1}}, {$unwind : "$hobbies"}, {$limit:1}"
 */

wchar_t * char_to_wchar(char *str){

    wchar_t *str_w = calloc(strlen(str) + 1, sizeof(wchar_t));
    mbstowcs(str_w, str, strlen(str)+1);
    return str_w;
}


/* check if a string is an integer or a float */
int check_number(char *str, int * ival, double *fval){
    
    typedef enum { ERR, INT, DOUBLE } parse_num;
    
    char* to_convert = str;
    char* p = NULL;
    errno = 0;
    int val = strtol(str, &p, 0);

    if (errno != 0)
        return ERR;// conversion failed (EINVAL, ERANGE)

    if (to_convert == p){
        // conversion to int failed (no characters consumed)
        double val2 = strtod(p, &p);
        if (*p){
           return ERR;
           }
        *fval = val2;
        return DOUBLE;
        }

    if (*p != 0){
        // conversion to int failed (trailing data)
        double val2 = strtod(str, &p);
        if (*p){
           return ERR;
           }
        *fval = val2;
        return DOUBLE; 
        }
    
    *ival = val;
    return INT;
}


/* check if tocken is a date object */
int check_date(char *js, jsmntok_t *t){

    regex_t regex;
    int reti;
    char msgbuf[100];
    char *buff;
    buff = json_token_tostr(js, t);
    /* Compile regular expression */
    reti = regcomp(&regex, "{ \"$date\" : [[:digit:]]\\{13\\} }", 0);
    /* Execute regular expression */
    reti = regexec(&regex, buff, 0, NULL, 0);
    if( !reti ){
        regfree(&regex);
        return 0;
        }
    else if( reti == REG_NOMATCH ){
        regfree(&regex);
        return 1;
    } else {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        /* Free compiled regular expression if you want to use the regex_t again */
        regfree(&regex);
        exit(1);
    }
}


/*
 * convert the unix epoch string to localtime
 * */
struct tm* convert_epoch_to_localtime(char * epoch){
    struct tm *time;
    time_t c;
    c = strtoul(epoch, NULL, 0);
    time = localtime(&c);
    return time;
}

/*
 * convert the unix epoch string to gmt
 * */
struct tm* convert_epoch_to_gmt(char * epoch){
    struct tm *time;
    time_t c;
    c = strtoul(epoch, NULL, 0);
    time = gmtime(&c);
    return time;
}

/*
 * Get valeus and store them in string array
 */
void 
extract_values(wchar_t **column_values, 
               char *js, 
               jsmntok_t *tokens, 
               int *ncol){
    
    int rv = -999;
    typedef enum { START, KEY, VALUE, SKIP, STOP, INCR } parse_state;
    parse_state state = START;
    size_t object_tokens = 0;
    char *str = NULL;
    size_t cv = 0;
    for (size_t i = 0, j = 1 ; j > 0; i++, j--)
    {
        jsmntok_t *t = &tokens[i];

        // Should never reach uninitialized tokens
        log_assert(t->start != -1 && t->end != -1);

        if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT)
            j += t->size;
        switch (state)
        {
            case START:
                if (t->type != JSMN_OBJECT)
                    log_die("Invalid response: root element must be an object.");

                state = KEY;
                object_tokens = t->size;
                if (object_tokens == 0)
                    state = STOP;

                if (object_tokens % 2 != 0)
                    log_die("Invalid response: object must have even number of children.");

                break;

            case KEY:
                object_tokens--;

                if (t->type != JSMN_STRING)
                    log_die("Invalid response: object keys must be strings.");
                    state = SKIP;
                
                if (i % 2) {
                    state = VALUE;
                }
                break;

            case SKIP:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    log_die("SKIP, Invalid response: object values must be strings or primitives.");

                object_tokens--;
                state = INCR;

                if (object_tokens == 0)
                    state = STOP;

                break;

            case VALUE:
                if (t->type == JSMN_OBJECT){
                    rv = check_date(js, t);
                    if (! rv )
                        str = json_token_tostr(js, t);
                        column_values[cv] = char_to_wchar(str);
                    state = SKIP;
                    break;
                }
                
                if (t->type == JSMN_PRIMITIVE){
                    str = json_token_tostr(js, t);
                    column_values[cv] = char_to_wchar(str);
                }
                
                if (t->type == JSMN_STRING){
                    str = json_token_tostr(js, t);
                    column_values[cv] = char_to_wchar(str);
                }
                
                state = INCR;
            
            case INCR:
                cv++;
                object_tokens--;
                state = KEY;
                if (object_tokens == 0)
                    state = STOP;
                break;

            case STOP:
                // Just consume the tokens 
                break;

            default:
                log_die("Invalid state %u", state);
        }
    }

    *ncol = (int) cv;
}

/*
 * Get keys and types and store them in string arrays
 */
void 
parse_keys_values(wchar_t **column_names,  
                  TAB_TYPE *column_types,
                  char *js, 
                  jsmntok_t *tokens){
    
    int rv = -999;
    typedef enum { START, KEY, VALUE, SKIP, STOP, INCR } parse_state;
    parse_state state = START;
    size_t object_tokens = 0;
    char *str = NULL;
    for (size_t i = 0, j = 1, cn = 0, cv = 0 ; j > 0; i++, j--)
    {
        jsmntok_t *t = &tokens[i];

        // Should never reach uninitialized tokens
        log_assert(t->start != -1 && t->end != -1);

        if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT)
            j += t->size;
        switch (state)
        {
            case START:
                if (t->type != JSMN_OBJECT)
                    log_die("Invalid response: root element must be an object.");

                state = KEY;
                object_tokens = t->size;
                if (object_tokens == 0)
                    state = STOP;

                if (object_tokens % 2 != 0)
                    log_die("Invalid response: object must have even number of children.");

                break;

            case KEY:
                object_tokens--;

                if (t->type != JSMN_STRING)
                    log_die("Invalid response: object keys must be strings.");
                    state = SKIP;
                
                if (i % 2) {
                    str = json_token_tostr(js, t);
                    column_names[cn] = char_to_wchar(str);
                    cn++;
                    state = VALUE;
                }
                break;

            case SKIP:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    log_die("SKIP, Invalid response: object values must be strings or primitives.");

                object_tokens--;
                state = INCR;

                if (object_tokens == 0)
                    state = STOP;

                break;

            case VALUE:
                
                if (t->type == JSMN_OBJECT){
                    rv = check_date(js, t);
                    if (! rv )
                        column_types[cv] = TAB_TYPE_DateTime;
                    state = SKIP;
                    break;
                }
                
                if (t->type == JSMN_PRIMITIVE){
                    str = json_token_tostr(js, t);
                    int ival = 0;
                    double dval = 0.0;
                    int rv = check_number(str, &ival, &dval);
                    if (rv == 1){
                        column_types[cv] = TAB_TYPE_Integer;
                    } else if (rv == 2) {
                        column_types[cv] = TAB_TYPE_Double;
                    } else if ((! strcmp("true", str)) || (! strcmp("false", str))) {
                        column_types[cv] = TAB_TYPE_Boolean;
                    } else if (!( strcmp("null", str) )) {
                        log_die("Found null in key [%ls], can't understand which type to create...", 
                        column_names[cv]);
                    }
                }
                
                if (t->type == JSMN_STRING){
                    str = json_token_tostr(js, t);
                    column_types[cv] = TAB_TYPE_UnicodeString;
                }
                
                state = INCR;
            
            case INCR:
                cv++;
                object_tokens--;
                state = KEY;
                if (object_tokens == 0)
                    state = STOP;
                break;

            case STOP:
                // Just consume the tokens 
                break;

            default:
                log_die("Invalid state %u", state);
        }
    }
}


/*
 * Detect type from string 
 */

int string_to_type(char *str){
   
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    char *rv = NULL;
    char *DATETIME_FORMAT = getenv("DATETIME_FORMAT");
    char *DATE_FORMAT = getenv("DATE_FORMAT");
    if (DATETIME_FORMAT == NULL)
        DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S";
    if (DATE_FORMAT == NULL)
        DATE_FORMAT = "%Y-%m-%d";

    rv = strptime(str, DATETIME_FORMAT, &tm);
    /* if rv is NULL then  
     * it failed in parsing the time */
    if (rv != NULL) {
        return TAB_TYPE_DateTime;
    }
    rv = strptime(str, getenv("DATE_FORMAT"), &tm);
    if (rv != NULL) {
        return TAB_TYPE_Date;
    }
    return -1;
}

/* Define the table's schema */
TAB_HANDLE 
make_table_definition(char *js, TAB_TYPE **column_types_p, int *ncol){

    jsmntok_t *tokens = json_tokenise(js);
    printf("js: %s\n", js);
    wchar_t **column_names = malloc( tokens[0].size / 2 * sizeof(wchar_t*));
    TAB_TYPE *column_types = malloc(tokens[0].size /2 * sizeof(TAB_TYPE));
    parse_keys_values(column_names, column_types, js, tokens);
    
    TAB_HANDLE hTableDef;
    TryOp(TabTableDefinitionCreate(&hTableDef));
    TryOp(TabTableDefinitionSetDefaultCollation(hTableDef, 
                TAB_COLLATION_en_US));
    for (; *ncol < tokens[0].size / 2 ; *ncol = *ncol + 1){
        TableauWChar *colname = malloc(wcslen(column_names[*ncol])+1);
        ToTableauString(column_names[*ncol], colname);
        printf("Trying to create column [%ls] type [%d]\n", column_names[*ncol],
                column_types[*ncol]);
        TryOp(TabTableDefinitionAddColumn(hTableDef, colname, 
                    column_types[*ncol]));
        printf("Successfully added column [%ls] with type [%d]\n", 
                column_names[*ncol], 
                column_types[*ncol]);
    }
    
    free(column_names);
    free(tokens);
    /* populate values which are passed by reference */
    *column_types_p = column_types;
    return hTableDef;
}


void insert_values(wchar_t **record_values, TAB_TYPE *column_types, 
        TAB_HANDLE *hTable, int rec_size){
    
    TAB_HANDLE hRow;
    TAB_HANDLE hTableDef;
    TryOp(TabTableGetTableDefinition(hTable, &hTableDef));
    TryOp(TabRowCreate(&hRow, hTableDef));
    
    for (int i = 0; i < rec_size; i++) {
        
        if (! wcscmp(L"null", record_values[i])){
            TryOp(TabRowSetNull(hRow, i));
            continue;
            }
        int coltype = column_types[i];
        char epoch[11];
        struct tm *gtime;
        char *ts; 
        TableauWChar *value;

        switch (coltype)
        {
        
           case 7:  // TAB_TYPE_Integer
                /* printf("Will insert integer %ls!\n", record_values[i]);*/
                errno = 0;
                char *bs = (char*)malloc(wcslen(record_values[i]));
                wcstombs(bs, record_values[i], strlen(bs));
                char* p = NULL;
                long val = strtol(bs, &p, 0);
                if (errno != 0)
                    log_die("Failed to convert %ls", record_values[i]);// conversion failed (EINVAL, ERANGE)
                
                TryOp(TabRowSetInteger(hRow, i, val));
                printf("Successfully inserted value %ld\n", val);
                free(bs);
                break;

           case 13: // TAB_TYPE_DateTime
                printf("Will insert datetime %ls!\n", record_values[i]); 
                ts = malloc(wcslen(record_values[i]));
                wcstombs(ts, record_values[i], wcslen(record_values[i]));
                memset(epoch, '\0', sizeof(epoch));
                strncpy(epoch, ts+12, 10*sizeof(char));
                gtime = convert_epoch_to_gmt(epoch);
                TryOp(TabRowSetDateTime(hRow, i, 1900 + gtime->tm_year, 
                                        gtime->tm_mon + 1 , gtime->tm_mday, 
                                        gtime->tm_hour, gtime->tm_min, 
                                        gtime->tm_sec, 0));
                /* Unfortunately, ctime is only accurate at the second level */
                free(ts);
                break;
            
           case 16: // TAB_TYPE_UnicodeString
                printf("Will insert double  %ls!\n", record_values[i]);
                value = malloc(wcslen(record_values[i])+1);
                ToTableauString(record_values[i], value);
                TryOp(TabRowSetString(hRow, i, value));
                free(value);
                break;

           case 10: // TAB_TYPE_Double 
                printf("Will insert double  %ls!\n", record_values[i]);
                wchar_t *stopwcs;
                double valf = wcstold(record_values[i], &stopwcs);
                TryOp(TabRowSetDouble(hRow, i, valf));                   
                break;
           
           case 11: // TAB_TYPE_Boolean
                printf("Will insert boolean  %ls!\n", record_values[i]);
                if (! wcscmp(record_values[i], L"true")) {
                    TryOp(TabRowSetBoolean(hRow, i, 1))
                } else if (!wcscmp(record_values[i], L"false")) {
                    TryOp(TabRowSetBoolean(hRow, i, 0))
                } else {
                log_die("Unknown boolean value ... %ls\n", record_values[i]);
                }
                break;
           
           default:
                log_die("Unknown tabeleau type...");
        }


    }
    TryOp( TabTableInsert( hTable, hRow ) );
    TryOp(TabRowClose(hRow));
    TryOp(TabTableDefinitionClose(hTableDef));
}


void get_columns(TAB_TYPE **column_types_p, TAB_HANDLE hTableDef, int *ncols){

    int i, numColumns;
    TAB_TYPE type;
    TryOp(TabTableDefinitionGetColumnCount(hTableDef, &numColumns));
    printf("Got table definition\n");
    TAB_TYPE *column_types = malloc(numColumns * sizeof(TAB_TYPE));
    printf("Allocated memory ... \n");
    for ( i = 0; i < numColumns; ++i ) {
        TryOp(TabTableDefinitionGetColumnType(hTableDef, i, &type));
        printf("Got column %d type %d\n", i, type);
        column_types[i] = type;
    }
    
    *column_types_p = column_types;
    *ncols = numColumns;
}
