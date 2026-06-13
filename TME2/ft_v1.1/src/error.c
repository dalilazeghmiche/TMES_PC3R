#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_LEN 24000
static char place[MAX_LEN];

/***************************************/
void internal_error (char *format, ...)
{
   va_list ap;
   va_start(ap,format);
   vsprintf(place,format,ap);
   va_end(ap);
   printf ("\ninternal error: %s\n",place);
   exit (1);
}

/***************************************/
