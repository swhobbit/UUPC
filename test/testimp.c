/*--------------------------------------------------------------------*/
/*       Program:    testimp     06/09/91                             */
/*       Author:     Andrew H. Derbyshire                             */
/*       Function:   Test UUPC/extended filename mapping              */
/*                   functions                                        */
/*                                                                    */
/*       Copyright (C) 1991, Andrew H. Derbyshire                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "import.h"
#include "export.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

char *E_nodename = "kendra";
char *panic_string = "Program aborting at line %d in file %s";
char *E_charset = DOSCHARS;
char *E_spooldir = "c:\uupc\spool";

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program to test importpath()                               */
/*--------------------------------------------------------------------*/

void main( int argc , char **argv )
{
   char canon[FILENAME_MAX];
   char host[FILENAME_MAX];
   size_t count;

   banner( argv );            /* Out of habit, I guess               */
   assert( argc > 2 );

   for( count = 2; count < argc; count++)
   {
      printf("main: Processing arg[%d]=\"%s\"\n",
               count, argv[count] );
      importpath( canon, argv[count], argv[1] );

      printf("import remote(%s)\thost(%s)\t yields canon(%s)\n",
               argv[1],
               argv[count] ,
               canon);

      fflush( stdout );

      if ( strchr( argv[count], '/' ) == NULL )
      {
         exportpath( host, canon,  argv[ 1 ] );

         printf("export remote(%s)\tcanon(%s)\t yields host(%s)\n",
               argv[1],
               canon,
               host );
      } /* if */

   } /* for */

} /* main */

/*--------------------------------------------------------------------*/
/*           Replacement for UUPC/extended logging function           */
/*--------------------------------------------------------------------*/

#pragma argsused

void printmsg(int level, char *fmt, ...)
{
   va_list arg_ptr;

   va_start(arg_ptr,fmt);
   vfprintf(stdout, fmt, arg_ptr);
   putchar('\n');
}


void bugout( const size_t lineno, const char *fname )
{
   printmsg(0,"Program aborting at line %d in file %s",
              lineno, fname );
   fcloseall();
   exit(69);
} /*bugout */
