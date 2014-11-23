#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "DataExtract.h"
#include "tde.h"

//static int verbose_flag;


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

