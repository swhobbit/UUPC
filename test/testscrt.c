/*--------------------------------------------------------------------*/
/*    s c r t e s t                                                   */
/*                                                                    */
/*    login Script test driver for UUPC/extended                      */
/*                                                                    */
/*    Copyright (c) Andrew H. Derbyshire                              */
/*                                                                    */
/*    May be distributed freely if original source and                */
/*    documentation files are included.  Please direct all            */
/*    questions on UUPC/extended to help@kew.com.                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    You know it's a REAL bitch if I write a test driver for it.     */
/*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <conio.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended included files                    */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "script.h"

void slowwrite( char *s, int len);

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program for driver                                         */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
{
   int result;
   char **failure = NULL;

   assert( argc > 1 );
   if (argc > 2)
      failure = &argv[2];

   result = expectstr( argv[1], 30, failure );
   printf("Result of operation is %d",result);
   exit(0);

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

/*--------------------------------------------------------------------*/
/*    s r e a d                                                       */
/*                                                                    */
/*    Fake serial port read                                           */
/*--------------------------------------------------------------------*/

extern unsigned int sread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout)
{
   int c;
   int needed = wanted;
   printf("sread(%d,%d):",wanted,timeout);

   while(needed-- > 0)
   {
      c = getch();
      if ( c == '.' )
         return -1;
      else
         *buffer++ = c;
   }

   putchar('\n');

   return wanted;
}

int swrite(char *data, unsigned int len)
{
   int needed = len;
   printf("swrite:");
   while ( needed-- )
      putchar( *data++ );
   putchar('\n');
   return len;
}

void ssendbrk(unsigned int duration)
{
   printf("BREAK %d\n",duration);
}

/*--------------------------------------------------------------------*/
/*    s l o w w r i t e                                               */
/*                                                                    */
/*    Write characters to the serial port at a configurable           */
/*    snail's pace.                                                   */
/*--------------------------------------------------------------------*/

void slowwrite( char *s, int len)
{
   swrite( s , len );
} /* slowwrite */

#pragma argsused

void SIOSpeed(BPS bps)
{
   printf("SIOSPEED\n");

} /*SIOSpeed*/
