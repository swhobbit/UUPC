/*--------------------------------------------------------------------*/
/*    p r i n t m s g . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

/*
 *    $Id$
 *
 *    $Log$
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __CORE__
#define __HEAPCHECK__
#endif

#ifdef __HEAPCHECK__
#include <alloc.h>
#else
#ifdef __CORELEFT__
#include <alloc.h>
#endif
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dater.h"
#include "logger.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#ifdef __HEAPCHECK__
currentfile();
#endif

int debuglevel = 1;
FILE *logfile = stdout;

#ifdef __CORE__
long  *lowcore = NULL;
char  *copyright = (char *) 4;
char  *copywrong = NULL;
#endif

/*--------------------------------------------------------------------*/
/*    As this routine is called from everywhere, we turn on stack     */
/*    checking here to handle the off-chance we screwed up and        */
/*    blew the stack.  This may catch it late, but it will catch      */
/*    it.                                                             */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma -N
#else
#pragma check_stack( on )
#endif

char *full_log_file_name = "UUPC log file";

/*--------------------------------------------------------------------*/
/*   p r i n t m s g                                                  */
/*                                                                    */
/*   Print an error message if its severity level is high enough.     */
/*   Print message on standard output if not in remote mode           */
/*   (call-in).  Always log the error message into the log file.      */
/*                                                                    */
/*   Modified by ahd 10/01/89 to check for Turbo C NULL pointers      */
/*   being de-referenced anywhere in program.  Fixed 12/14/89         */
/*                                                                    */
/*   Modified by ahd 04/18/91 to use true variable parameter list,    */
/*   supplied by Harald Boegeholz                                     */
/*--------------------------------------------------------------------*/

void printmsg(int level, char *fmt, ...)
{
   va_list arg_ptr;

#ifdef __CORELEFT__
   static unsigned freecore = 63 * 1024;
   unsigned nowfree;
#endif

#ifdef __HEAPCHECK__
      static boolean recurse = FALSE;
      int heapstatus;

      heapstatus = heapcheck();
      if (heapstatus == _HEAPCORRUPT)
         printf("\a*** HEAP IS CORRUPTED ***\a\n");

#endif

#ifdef __CORE__
   if (*lowcore != 0L)
   {
      putchar('\a');
      debuglevel = level;  /* Force this last message to print ahd   */
   }
#endif


#ifdef __CORELEFT__
   nowfree = coreleft();
   if (nowfree < freecore)
   {
      freecore = (nowfree / 10) * 9;
      printmsg(0,"Free memory = %u bytes", nowfree);
   }
#endif

   if (level <= debuglevel)
   {

      FILE *stream = (logfile == NULL) ? stdout : logfile;

      va_start(arg_ptr,fmt);

      if (stream != stdout)
      {
         vfprintf(stderr, fmt, arg_ptr);
         fputc('\n',stderr);

         if ( debuglevel > 1 )
            fprintf(stream, "(%d) ", level);
         else
            fprintf(stream, "%s ", dater( time( NULL ), NULL));

      } /* if (stream != stdout) */

      if (!ferror(stream))
         vfprintf(stream, fmt, arg_ptr);

      if (!ferror(stream))
         fputc('\n',stream);

      if (ferror(stream))
      {
         perror(full_log_file_name);
         abort();
      } /* if */

#ifdef __HEAPCHECK__
      if ( !recurse )
      {
         recurse = TRUE;
#ifdef __CORE__
         if (*lowcore != 0L)
            panic();
    /*     if (!equal(copyright,copywrong))
            panic();                         */
#endif
         if (heapstatus == _HEAPCORRUPT)
            panic();
         recurse = FALSE;
      }
#endif


/*--------------------------------------------------------------------*/
/*                        Massive debug mode?                         */
/*--------------------------------------------------------------------*/

   if ((debuglevel > 10) &&  ((level+2) < debuglevel))
      fflush( logfile );

   } /* if (level <= debuglevel) */

} /*printmsg*/
