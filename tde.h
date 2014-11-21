#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "DataExtract.h"


#define COLS 999 

#define TryOp(x) if (x != TAB_RESULT_Success) { \
    fprintf(stderr, "Error: %ls\n", TabGetLastErrorMessage()); \
    exit( EXIT_FAILURE ); }

