/*--------------------------------------------------------------------*/
/*       p o s 2 e r r  . c                                           */
/*                                                                    */
/*       Report error message from OS/2 error library                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pos2err.c 1.2 1993/09/30 03:06:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: pos2err.c $
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
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <dos.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
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
   static boolean recursion  = FALSE;
   int l;
   static char sysMsgs[] = "oso001.msg";
   boolean redirect = ((logfile != stdout) && !isatty(fileno(stdout)));

#ifdef __OS2__
   ULONG len, xrc;
#else
   USHORT len, xrc;
#endif

   switch( rc )
   {
      case ERROR_TS_WAKEUP:
         strcpy( buf, "Interrupted System Call");
         break;

      case ERROR_GEN_FAILURE:
         strcpy( buf, "Invalid parameter, Port IRQ conflict, or device failure");
         break;

      default:
         xrc = DosGetMessage( (PCHAR) NULL,
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
               recursion = TRUE;
               printOS2error( "DosGetMessage", xrc );
               recursion = FALSE;
            } /* recursion */

            sprintf(buf, "OS/2 API error %d in %s at line %d,"
                         " cannot find message",
                         (int) rc,
                         fname,
                         lineno );

         } /* if ( xrc != 0 ) */
         else
            buf[ len ] = '\0';
         break;

   } /* switch */

/*--------------------------------------------------------------------*/
/*    Drop extra new from error message if we have room in our        */
/*    small buffer                                                    */
/*--------------------------------------------------------------------*/

   l = strlen( buf );

   if (( buf[l-1] == '\n') & (l < sizeof buf ))
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
