/*--------------------------------------------------------------------*/
/*    p r i n t m s g . c                                             */
/*                                                                    */
/*    Logging routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: printmsg.c 1.16 1995/01/07 16:13:55 ahd v1-12q $
 *
 *    $Log: printmsg.c $
 *    Revision 1.16  1995/01/07 16:13:55  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.15  1994/12/22 00:10:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1994/02/25 02:23:42  ahd
 *    Suppress compile warning with IBM ICC compiler
 *
 *     Revision 1.13  1994/02/20  19:05:02  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.12  1994/02/19  04:45:23  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/19  04:10:21  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1994/02/19  03:56:05  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/18  23:13:14  ahd
 *     Use standard first header
 *
 *     Revision 1.8  1994/01/01  19:04:11  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.7  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.7  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.6  1993/12/06  01:59:07  ahd
 *     Add code to compare environment changes
 *     ./
 *
 *     Revision 1.5  1993/10/12  00:47:04  ahd
 *     Normalize comments
 *
 *     Revision 1.4  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.3  1993/04/10  21:26:04  ahd
 *     Use unique buffer for printmsg() time stamp
 *
 * Revision 1.2  1992/11/20  12:39:37  ahd
 * Move heapcheck to check heap *EVERY* call
 *
 */

#include "uupcmoah.h"

#include <stdarg.h>

#ifdef __CORE__
#define __HEAPCHECK__
#elif defined(__HEAPCHECK__)
#define __CORELEFT__
#define __ENVCHECK__
#endif

#if defined(__HEAPCHECK__) || defined(__CORELEFT__) || defined(__ENVCHECK__)

#include <alloc.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dater.h"
#include "logger.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#if defined(__HEAPCHECK__)
currentfile();
#endif

int debuglevel = 1;

#ifdef __IBMC__
FILE *logfile;
#else
FILE *logfile = stdout;
#endif

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
#elif !defined(__IBMC__)
#pragma check_stack( on )
#endif

char *full_log_file_name = "UUPC log file";

/*--------------------------------------------------------------------*/
/*             Internal debugging for environment failure             */
/*--------------------------------------------------------------------*/

#ifdef __ENVCHECK__

static char UUFAR *myEnv[200];
static char UUFAR *saveEnv[200];

/*--------------------------------------------------------------------*/
/*       p r i n t e n v                                              */
/*                                                                    */
/*       Print saved and current environment strings                  */
/*--------------------------------------------------------------------*/

static void printEnv( void )
{
   int subscript1 = 0;
   int subscript2 = 0;
   static KWBoolean changed = KWFalse;

   static char UUFAR envBuf[32768];
   static size_t offset = 0;

   if ( offset > sizeof envBuf / 2 )
      offset = 0;

   printmsg(0,"Dumping environments at %Fp and %p ...", myEnv, environ);

   while( (myEnv[subscript1] && _fstrlen(myEnv[subscript1])) ||
         strlen(environ[subscript2]) )
   {
      int diff = (myEnv[subscript1] == 0) ||
                 _fstrcmp( myEnv[subscript1], environ[subscript2] );

      if ( diff && changed )
         printmsg(0,
            "Saved: %Fp: %-22.22Fs Current: %p: %.22s",
            saveEnv[subscript1],
            (char far *) (myEnv[subscript1]  ? myEnv[subscript1] : "*n/a*"),
            environ[subscript2],
            environ[subscript2]  ? environ[subscript2] : "*n/a*" );

/*--------------------------------------------------------------------*/
/*              Copy the new variable value as required               */
/*--------------------------------------------------------------------*/

      saveEnv[subscript2] = environ[subscript2];
      if ( environ[subscript2] )
      {
         if ( diff || ( myEnv[subscript1] > ( envBuf + offset )))
         {
            myEnv[subscript1] = _fstrcpy( envBuf + offset,
                                          environ[subscript2] );
            offset += strlen(environ[subscript2]) + 1;
         }
         subscript2++;
      }
      else
         myEnv[subscript1] = 0;
      subscript1++;

   } /* while */

   myEnv[subscript1] = 0;

   printmsg(0,"%d entries in local copy, %d entries in live copy",
               subscript1, subscript2 );

   changed = KWTrue;              /* After first pass, always report  */

} /* printEnv */

/*--------------------------------------------------------------------*/
/*       c h e c k e n v                                              */
/*                                                                    */
/*       Detect changed environment                                   */
/*--------------------------------------------------------------------*/

static void checkEnv( void )
{
   int subscript = 0;
   static KWBoolean recurse = KWFalse;

   if ( recurse )
      return;

   recurse = KWTrue;

   while( myEnv[subscript] && environ[subscript] )
   {
      if ( !_fstrcmp( myEnv[subscript], environ[subscript] ))
         subscript++;
      else
         break;

   } /* while */

/*--------------------------------------------------------------------*/
/*              If environments different length, report              */
/*--------------------------------------------------------------------*/

   if ( myEnv[subscript] || environ[subscript] )
      printEnv();

   recurse = KWFalse;

}  /* checkEnv */

#endif

/*--------------------------------------------------------------------*/
/*   p r i n t m s g                                                  */
/*                                                                    */
/*   Print an error message if its severity level is high enough.     */
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
      static KWBoolean recurse = KWFalse;
      int heapstatus;

      heapstatus = heapcheck();
      if (heapstatus == _HEAPCORRUPT)
         printf("\a*** HEAP IS CORRUPTED ***\a\n");

#endif

#ifdef __CORE__
   if (*lowcore != 0L)
   {
      putchar('\a');
      debuglevel = level;  /* Force this last message to print ahd    */
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

      FILE *stream = (logfile == NULL) ? stderr : logfile;

      va_start(arg_ptr,fmt);

      if ((stream != stdout) && (stream != stderr))
      {
         char now[DATEBUF];
         vfprintf(stderr, fmt, arg_ptr);
         fputc('\n',stderr);

         if ( debuglevel > 1 )
            fprintf(stream, "(%d) ", level);
         else
            fprintf(stream, "%s ", dater( time( NULL ), now));

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
         recurse = KWTrue;
#ifdef __CORE__
         if (*lowcore != 0L)
            panic();
    /*     if (!equal(copyright,copywrong))
            panic();                         */
#endif
         if (heapstatus == _HEAPCORRUPT)
            panic();
         recurse = KWFalse;
      }
#endif

#ifdef __ENVCHECK__
      checkEnv();
#endif

/*--------------------------------------------------------------------*/
/*                        Massive debug mode?                         */
/*--------------------------------------------------------------------*/

   if ((debuglevel > 10) &&  ((level+2) < debuglevel))
      fflush( logfile );

   } /* if (level <= debuglevel) */

} /*printmsg*/
