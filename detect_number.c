#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{

char* to_convert = argv[1];
char* p = NULL;
errno = 0;
long val = strtol(argv[1], &p, 0);

if (errno != 0)
    return 1;// conversion failed (EINVAL, ERANGE)

if (to_convert == p){
    // conversion to int failed (no characters consumed)
    double val2 = strtod(p, &p);
    if (*p){
       printf("Not a number!\n");
       return 1;
       }
    printf("Double %lf\n" , val2);
    return 0;
    }

if (*p != 0){
    // conversion to int failed (trailing data)
    double val2 = strtod(argv[1], &p);
    if (*p){
       printf("Not a number!\n");
       return 1;
       }
    printf("Double %lf\n" , val2);
    return 0; 
    }

printf("Int %ld\n" , val);
return 0;
}
