/*--------------------------------------------------------------------*/
/*    p r i n t e r r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1989, 1992 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: printerr.c 1.10 1995/01/07 16:13:48 ahd Exp $
 *
 *    Revision history:
 *    $Log: printerr.c $
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
   int l = strlen( s );

   KWBoolean redirect = ((logfile != stdout) && !isatty(fileno(stdout))) ?
                           KWTrue : KWFalse;

/*--------------------------------------------------------------------*/
/*    Drop extra new from error message if we have room in our        */
/*    small buffer                                                    */
/*--------------------------------------------------------------------*/

   if (( s[l-1] == '\n') & (l < sizeof buf ))
   {
      s = strcpy( buf, s);    /* Make buf copy of string we use below*/
      s[l-1] = '\0';          /* Drop extra newline from string       */
   }

/*--------------------------------------------------------------------*/
/*           Display the message with option file location            */
/*--------------------------------------------------------------------*/

   printmsg(2,"Run time library error %d in %s at line %d ...",
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

/*--------------------------------------------------------------------*/
/*               Abort if that is the suggested action                */
/*--------------------------------------------------------------------*/

      switch( regs.h.bl )
      {
         case 0x04:
         case 0x05:
               bugout( lineno, fname);

         default:
               break;
      } /* switch */
   } /* (_osmajor >= 3 ) */
#endif

} /* printerr */
