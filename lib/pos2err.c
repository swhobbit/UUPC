/*--------------------------------------------------------------------*/
/*       p o s 2 e r r  . c                                           */
/*                                                                    */
/*       Report error message from OS/2 error library                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pos2err.c 1.15 1998/03/01 01:24:44 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: pos2err.c $
 *    Revision 1.15  1998/03/01 01:24:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1997/03/31 07:05:29  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1996/01/01 20:54:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/07/21 13:23:19  ahd
 *    Clean up OS/2 compiler warnings
 *
 *    Revision 1.11  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.10  1995/01/07 16:13:42  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.9  1994/12/22 00:10:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/03/09 04:53:53  ahd
 *    Add missing <ctype.h> header
 *
 * Revision 1.7  1994/03/09  04:17:41  ahd
 * Suppress all control characters in OS/2 message text
 *
 * Revision 1.6  1994/02/20  19:05:02  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.5  1994/01/01  19:04:03  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/12/23  03:11:17  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.3  1993/10/12  00:47:04  ahd
 * Normalize comments
 *
 * Revision 1.2  1993/09/30  03:06:28  ahd
 * Handle selected errors with special messages
 *
 * Revision 1.1  1993/09/24  03:43:27  ahd
 * Initial revision
 *
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#define INCL_DOSMISC
#define INCL_ERRORS

#include "uupcmoah.h"

#include <os2.h>
#include <errno.h>
#include <io.h>
#include <ctype.h>

#ifndef ERROR_TIMEOUT
#define ERROR_TIMEOUT   640     /* MSG%none */
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "pos2err.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    p O S 2 e r r                                                   */
/*                                                                    */
/*    Perform a perror() with logging                                 */
/*--------------------------------------------------------------------*/

void pOS2Err(const size_t lineno,
             const char *fname,
             const char *prefix,
             unsigned int rc)
{
   char buf[BUFSIZ];
   static KWBoolean recursion  = KWFalse;
   size_t l;
   static char sysMsgs[] = "oso001.msg";

   KWBoolean redirect;

#ifdef __OS2__
   ULONG len, xrc;
#else
   USHORT len, xrc;
#endif

/*--------------------------------------------------------------------*/
/*       Determine if we need to echo the error an extra time to      */
/*       the console.                                                 */
/*--------------------------------------------------------------------*/

   if ((logfile != stdout) && !isatty(fileno(stdout)))
      redirect = KWTrue;
   else
      redirect = KWFalse;

/*--------------------------------------------------------------------*/
/*             Override the text for selected error numbers           */
/*--------------------------------------------------------------------*/

   switch( rc )
   {
      case ERROR_TS_WAKEUP:
         strcpy( buf, "Interrupted System Call");
         break;

      case ERROR_GEN_FAILURE:
         strcpy( buf, "Invalid parameter, Port IRQ conflict, or device failure");
         break;

      case ERROR_TIMEOUT:
         strcpy( buf, "Error timeout");   /* DosWaitEventSem reports
                                             this                    */
         break;

      default:
         xrc = DosGetMessage( NULL,
                              0,
                              (PCHAR) buf,
                              sizeof buf,
                              rc,
                              (PSZ) sysMsgs,
                              &len );

         if ( xrc != 0 )
         {

            if ( ! recursion )
            {
               recursion = KWTrue;
               printOS2error( "DosGetMessage", xrc );
               recursion = KWFalse;
            } /* recursion */

            sprintf(buf, "OS/2 API error %d in %s at line %d,"
                         " cannot find message",
                         (int) rc,
                         fname,
                         lineno );

         } /* if ( xrc != 0 ) */
         else {
            size_t column = 0;

            for ( column = 0; column < len; column++ )
            {
               if (iscntrl( buf[ column ] ))
                  buf[column] = ' ';      /* Zap control chars    */
            }

            buf[ len ] = '\0';
         }
         break;

   } /* switch */

/*--------------------------------------------------------------------*/
/*    Drop extra new from error message if we have room in our        */
/*    small buffer                                                    */
/*--------------------------------------------------------------------*/

   l = strlen( buf );

   if (( buf[l-1] == '\n') && (l < sizeof buf ))
      buf[l-1] = '\0';          /* Drop extra newline from string     */

/*--------------------------------------------------------------------*/
/*           Display the message with option file location            */
/*--------------------------------------------------------------------*/

   printmsg(2,"OS/2 API error %d in %s at line %d ...",
            (int) rc, fname, lineno );

   printmsg(0,"%s: %s", prefix, buf);

   if ( redirect )
      fprintf(stdout,"%s: %s\n", prefix, buf);

} /* pOS2Err */
