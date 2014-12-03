#define _XOPEN_SOURCE
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>
#include "DataExtract.h"
#include "tde.h"



#define FORMAT_DATETIME "%Y-%m-%d %H:%M:%S"
#define FORMAT_DATE     "%Y-%m-%d"

/* 
 * Should be able to query like this ...
"{$project: {_id:0, "hobbies":1}}, {$unwind : "$hobbies"}, {$limit:1}"
 */

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
 * convert the unix epoch string to GMT 
 * */
struct tm* convert_epoch_to_gmt(char * epoch){
    struct tm *time;
    time_t c;
    // todo, implement check that len(epoch) == 14
    c = strtoul(epoch, NULL, 0);
    time = localtime(&c);
    return time;
}


/*
 * Get keys and values and store them in string arrays
 */
void 
parse_keys_values(char **column_names, char **column_values, 
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
                //printf("Object_tokens: %zu\n", object_tokens);
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
                    column_names[cn] = str;
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
                /* date values are given like this { "$date" : 1412200800000 }
                 * hence, we need to have special handling here ...
                 * */
                //if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                //    log_die("PRINT, Invalid response: object values must be strings or primitives.");
                if (t->type == JSMN_OBJECT){
                    rv = check_date(js, t);
                    if (! rv )
                        column_values[cv] = "DATE";
                    state = SKIP;
                    break;
                }
                
                if (t->type == JSMN_PRIMITIVE){
                    str = json_token_tostr(js, t);
                    column_values[cv] = "PRIMITIVE";
                }
                
                if (t->type == JSMN_STRING){
                    str = json_token_tostr(js, t);
                    column_values[cv] = "STRING";
                }
                
                //else {
                //   str = json_token_tostr(js, t);
                //    column_values[cv] = str;
                //    printf("%zu VALUE %s\n", i, column_values[cv]);
                //}
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
//static int verbose_flag;
TAB_HANDLE 
make_table_definition(char *js){

    TAB_HANDLE hExtract = NULL;
    //typedef enum { START, KEY, PRINT, SKIP, STOP } parse_state;
    //parse_state state = START;
    jsmntok_t *tokens = json_tokenise(js);
    printf("js: %s\n", js);
    char **column_names = malloc( tokens[0].size / 2 * sizeof(char*));
    char **column_values = malloc(tokens[0].size / 2 * sizeof(char*));
    //printf("size of : %d\n", tokens[0].size / 2 );
    
    parse_keys_values(column_names, column_values, js, tokens);
    for  (int i = 0 ; i < tokens[0].size / 2; i++){
        printf("KEY %s ", column_names[i]);
        printf("VALUE %s\n", column_values[i]);
    
    }


    return hExtract;

}
/* Define the table's schema */
TAB_HANDLE MakeTableDefinition()
{
    TAB_HANDLE hTableDef;
    TableauWChar sPurchased[10];
    TableauWChar sProduct[8];
    TableauWChar sUProduct[9];
    TableauWChar sPrice[6];
    TableauWChar sQuantity[9];
    TableauWChar sTaxed[6];
    TableauWChar sExpirationDate[16];
    TableauWChar sProdukt[8];

    ToTableauString( L"Purchased", sPurchased );
    ToTableauString( L"Product", sProduct );
    ToTableauString( L"uProduct", sUProduct );
    ToTableauString( L"Price", sPrice );
    ToTableauString( L"Quantity", sQuantity );
    ToTableauString( L"Taxed", sTaxed );
    ToTableauString( L"Expiration Date", sExpirationDate );
    ToTableauString( L"Produkt", sProdukt );

    TryOp( TabTableDefinitionCreate( &hTableDef ) );
    TryOp( TabTableDefinitionSetDefaultCollation( hTableDef, TAB_COLLATION_en_GB ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sPurchased,      TAB_TYPE_DateTime ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sProduct,        TAB_TYPE_CharString ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sUProduct,       TAB_TYPE_UnicodeString ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sPrice,          TAB_TYPE_Double ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sQuantity,       TAB_TYPE_Integer ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sTaxed,          TAB_TYPE_Boolean ) );
    TryOp( TabTableDefinitionAddColumn( hTableDef, sExpirationDate, TAB_TYPE_Date ) );

    /* Override Default collation */
    TryOp( TabTableDefinitionAddColumnWithCollation( hTableDef, sProdukt, TAB_TYPE_CharString, TAB_COLLATION_de ) );

    return hTableDef;
}

/* Print a Table's schema to stderr */
void PrintTableDefinition( TAB_HANDLE hTableDef )
{
    int i, numColumns, len;
    TAB_TYPE type;
    TableauString str;
    wchar_t* wStr;

    TryOp( TabTableDefinitionGetColumnCount( hTableDef, &numColumns ) );
    for ( i = 0; i < numColumns; ++i ) {
        TryOp( TabTableDefinitionGetColumnType( hTableDef, i, &type ) );
        TryOp( TabTableDefinitionGetColumnName( hTableDef, i, &str ) );

        len = TableauStringLength( str );
        wStr = malloc( (len + 1) * sizeof(wchar_t) ); /* make room for the null */
        FromTableauString( str, wStr );

        fprintf(stderr, "Column %d: %ls (%#06x)\n", i, wStr, type);
        free(wStr);
    }
}

/* Insert a few rows of data. */
void InsertData( TAB_HANDLE hTable )
{
    TAB_HANDLE hRow;
    TAB_HANDLE hTableDef;
    int i;

    TableauWChar sUniBeans[9];
    ToTableauString( L"uniBeans", sUniBeans );

    TryOp( TabTableGetTableDefinition( hTable, &hTableDef ) );

   /* Create a row to insert data */
    TryOp( TabRowCreate( &hRow, hTableDef ) );

    TryOp( TabRowSetDateTime(   hRow, 0, 2012, 7, 3, 11, 40, 12, 4550 ) ); /* Purchased */
    TryOp( TabRowSetCharString( hRow, 1, "Beans" )  );                     /* Product */
    TryOp( TabRowSetString(     hRow, 2, sUniBeans ) );                    /* uProduct */
    TryOp( TabRowSetDouble(     hRow, 3, 1.08 ) );                         /* Price */
    TryOp( TabRowSetDate(       hRow, 6, 2029, 1, 1 ) );                   /* Expiration date */
    TryOp( TabRowSetCharString( hRow, 7, "Bohnen" ) )                      /* Produkt */

    /* Insert a few rows */
    for ( i = 0; i < 10; ++i ) {
        TryOp( TabRowSetInteger(  hRow, 4, i * 10) );                      /* Quantity */
        TryOp( TabRowSetBoolean(  hRow, 5, i % 2 ) );                      /* Taxed */
        TryOp( TabTableInsert( hTable, hRow ) );
    }

    TryOp( TabRowClose( hRow ) );
    TryOp( TabTableDefinitionClose( hTableDef ) );
}

