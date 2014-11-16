#include <bson.h>
#include <mongoc.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include "DataExtract.h"


#define COLS 999 

static int verbose_flag;

void print_usage() {
    printf("Usage: coll2tde [v] -h HOST -d DATABASE -c COLLECTION -s "
           "COLUMNS -o TDEFILE\n");
}

#define TryOp(x) if (x != TAB_RESULT_Success) { \
    fprintf(stderr, "Error: %ls\n", TabGetLastErrorMessage()); \
    exit( EXIT_FAILURE ); }

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

/* Wrap all mongodb stuff in a function */
mongoc_cursor_t *
get_cursor(char *host, char *db, char *collection_name, bson_t *pquery,
           mongoc_collection_t **collection_p,
           mongoc_client_t **client_p){
    
    //mongoc_client_t *client;
    //mongoc_collection_t *collection;
    mongoc_cursor_t *cursor;
    //const bson_t *doc;
    //char *str;

    *client_p = mongoc_client_new("mongodb://localhost:27017/");
    *collection_p = mongoc_client_get_collection (*client_p, "test", "test");
    cursor = mongoc_collection_find(*collection_p, MONGOC_QUERY_NONE, 
                                    0, 0, 0, pquery, NULL, NULL);
    //*client_p = client;
    //*collection_p = collection;
    return cursor;
}

int
main (int   argc, char *argv[]){
    int opt = 0;
    char *host = NULL;
    char *database = NULL;
    char *collection_name = NULL;
    char *filename = NULL;
    char *columns = NULL;
    
    static struct option long_options[] = {
        {"host",      required_argument,       0,  'h' },
        {"database", no_argument,       0,  'd' },
        {"collection",    required_argument, 0,  'c' },
        {"columns",   required_argument, 0,  COLS },
        {"filename",   required_argument, 0,  'f' },
        {0,           0,                 0,  0   }
    };

    int long_index =0;
    while ((opt = getopt_long(argc, argv,"h:d:c:f:", 
                   long_options, &long_index )) != -1) {
        switch (opt) {
             case 'h' : host = optarg;
                 break;
             case 'd' : database = optarg;
                 break;
             case 'c' : collection_name = optarg; 
                 break;
             case 'f' : filename = optarg;
                 break;
             case COLS : columns = optarg;
                 break;
             default: print_usage(); 
                 exit(EXIT_FAILURE);
        }


    }
    /* if no file name is given the collection_name will be appended tde */
    printf("host: %s\n", host);
    printf("database: %s\n", database);
    printf("collection_name: %s\n", collection_name);

    if ( host == NULL || database == NULL || collection_name == NULL)  {
        print_usage();
        exit(EXIT_FAILURE);
    }
    
    if ( filename == NULL ) {
        filename = strcat(collection_name, ".tde");  
    }
    
    if ( columns != NULL ) {
        printf("columns: %s\n", columns); 
    }
   

    printf("filename: %s\n", filename);
    TAB_HANDLE hExtract;
    TAB_HANDLE hTableDef;
    TAB_HANDLE hTable;
    int i;
    int bHasTable;

    TableauWChar sOrderTde[12];
    TableauWChar sExtract[8];
    ToTableauString( L"order-c.tde", sOrderTde );
    ToTableauString( L"Extract", sExtract );
    
    TryOp( TabExtractCreate( &hExtract, sOrderTde ) );
    TryOp( TabExtractHasTable( hExtract, sExtract, &bHasTable ) );

    if ( !bHasTable ) {
        /* Table does not exist; create it. */
        hTableDef = MakeTableDefinition();
        TryOp( TabExtractAddTable( hExtract, sExtract, hTableDef, &hTable ) );
        TryOp( TabTableDefinitionClose( hTableDef ) );
    }
    else {
        /* Open an existing table to add more rows. */
        TryOp( TabExtractOpenTable( hExtract, sExtract, &hTable ) );
    }

    TryOp( TabTableGetTableDefinition( hTable, &hTableDef ) );
    PrintTableDefinition( hTableDef );
 
    InsertData( hTable );

    /* Clean up */
    TryOp( TabTableDefinitionClose( hTableDef ) );
    TryOp( TabExtractClose( hExtract ) );

    printf ("done with tableu kram\n");
    mongoc_client_t *client_p;
    mongoc_collection_t *collection_p;
    mongoc_cursor_t *cursor;
    const bson_t *doc;
    bson_t *query = NULL;
    char *str;

    mongoc_init();

    //client = mongoc_client_new("mongodb://localhost:27017/");
    //collection = mongoc_client_get_collection (client, "test", "test");
    query = bson_new ();
    //cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 
    //                               0, 0, 0, query, NULL, NULL);
    cursor = get_cursor(host, database, collection_name, query, //&collection,
             &collection_p, &client_p);
    while (mongoc_cursor_next (cursor, &doc)) {
        str = bson_as_json (doc, NULL);
        printf ("%s\n", str);
        bson_free (str);
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(query);
    mongoc_collection_destroy(collection_p);
    mongoc_client_destroy(client_p);
    mongoc_cleanup();
    return 0;
}
