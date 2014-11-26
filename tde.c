#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "DataExtract.h"
#include "tde.h"

/* 
 * Should be able to query like this ...
"{$project: {_id:0, "hobbies":1}}, {$unwind : "$hobbies"}, {$limit:1}"
 */

/*
 * Get keys and values and store them in string arrays
 */
void 
parse_keys_values(char **column_names, char **column_values, 
                  char *js, 
                  jsmntok_t *tokens){
    
    typedef enum { START, KEY, PRINT, SKIP, STOP } parse_state;
    parse_state state = START;
    size_t object_tokens = 0;
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
                printf("Object_tokens: %zu\n", object_tokens);
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
                char *str = NULL;
                if (i % 2) {
                    str = json_token_tostr(js, t);
                    column_names[cn] = str;
                    printf("KEY %s\n", column_names[cn]);
                    cn++;
                    state = PRINT;
                }
                break;

            case SKIP:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    log_die("Invalid response: object values must be strings or primitives.");

                object_tokens--;
                state = KEY;

                if (object_tokens == 0)
                    state = STOP;

                break;

            case PRINT:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    log_die("Invalid response: object values must be strings or primitives.");

                str = json_token_tostr(js, t);
                //if (! (i % 2))
                column_values[cv] = str;
                printf("%zu VALUE %s\n", i, column_values[cv]);
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
    printf("size of : %d\n", tokens[0].size);
    
    parse_keys_values(column_names, column_values, js, tokens);
    printf("%s\n", column_names[0]);
    printf("%s\n", column_names[1]);


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

