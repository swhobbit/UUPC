/*--------------------------------------------------------------------*/
/*    p r i n t e r r . c                                             */
/*                                                                    */
/*    Run time library error reporting, includes source line of       */
/*    error via pre-processor generated information.                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: printerr.c 1.15 1996/01/01 20:50:13 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: printerr.c $
 *    Revision 1.15  1996/01/01 20:50:13  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1995/09/04 02:13:41  ahd
 *    Correct DOS compile error
 *
 *    Revision 1.13  1995/05/07 14:38:19  ahd
 *    Drop automatic aborts under DOS, other systems don't provide
 *    the function and calling code now issues desired panic()
 *    directly.
 *
 *    Revision 1.12  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.11  1995/01/13 14:00:40  ahd
 *    Print error number as well as location at higher debug levels
 *
 *    Revision 1.10  1995/01/07 16:13:48  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.9  1994/02/20 19:07:38  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.8  1993/10/12  00:45:27  ahd
 *     Normalize comments
 *
 *     Revision 1.7  1993/10/12  00:03:05  ahd
 *     Perform extended diags under MS C 7.0
 *
 *     Revision 1.6  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 *     Revision 1.5  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.4  1993/07/22  23:19:50  ahd
 *     First pass for Robert Denny's Windows 3.x support changes
 *
 *     Revision 1.3  1993/04/11  00:32:05  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/19  02:57:19  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

#include "uupcmoah.h"

#include <errno.h>

#include <io.h>

#ifndef __32BIT__
#include <dos.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    p r i n t e r r                                                 */
/*                                                                    */
/*    Perform a perror() with logging                                 */
/*--------------------------------------------------------------------*/

void prterror(const size_t lineno, const char *fname, const char *prefix)
{
   char buf[50];
   int myErrno = errno;
   char *s = strerror(errno);
   size_t l = strlen( s );

   KWBoolean redirect;

   if ((logfile != stdout) && !isatty(fileno(stdout)))
      redirect = KWTrue;
   else
      redirect = KWFalse;

/*--------------------------------------------------------------------*/
/*    Drop extra new from error message if we have room in our        */
/*    small buffer                                                    */
/*--------------------------------------------------------------------*/

   if (( s[l-1] == '\n') && (l < sizeof buf ))
   {
      s = strcpy( buf, s);    /* Make buf copy of string we use below*/
      s[l-1] = '\0';          /* Drop extra newline from string       */
   }

/*--------------------------------------------------------------------*/
/*           Display the message with option file location            */
/*--------------------------------------------------------------------*/

   printmsg(2,"Run time library error %d in %s at line %u ...",
               myErrno,
               fname,
               lineno);

   printmsg(0,"%s: %s", prefix, s);

   if ( redirect )
      fprintf(stdout,"%s: %s\n", prefix, s);

#if !defined(_Windows) && !defined(BIT32ENV) && (defined(__TURBOC__) || (_MSC_VER >= 700))

   if (_osmajor >= 3 )
   {

      union REGS regs;
      struct SREGS sregs;
      regs.h.ah = 0x59;       /* Extended error information           */
      regs.x.bx = 0x00;       /* Set up for call                      */
      intdosx(&regs, &regs, &sregs);

      printmsg(1,"Extended DOS Error Information: "
            "Number = %d, Class = %d, Action = %d, Locus = %d",
                  (int) regs.x.ax, (int) regs.h.bh,
                  (int) regs.h.bl, (int) regs.h.ch );

      if ( redirect )
      {
         fprintf(stdout, "Extended DOS Error Information: "
            "Number = %d, Class = %d, Action = %d, Locus = %d",
                  (int) regs.x.ax, (int) regs.h.bh,
                  (int) regs.h.bl, (int) regs.h.ch );
         fputc('\n',stdout);  /* Allows compiler to avoid generating
                                 second almost duplicate literal str  */
      } /* if ( redirect ) */

   } /*  if (_osmajor >= 3 ) */
#endif

} /* printerr */
