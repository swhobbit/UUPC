/*--------------------------------------------------------------------*/
/*       p n t e r r . c                                              */
/*                                                                    */
/*       Report error message from NT error library                   */
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
 *    $Id: pnterr.c 1.12 1998/03/01 01:24:41 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: pnterr.c $
 *    Revision 1.12  1998/03/01 01:24:41  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1997/03/31 07:05:23  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1996/01/01 20:49:54  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1995/01/07 16:13:36  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.8  1994/12/22 00:10:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/02/19 04:44:57  ahd
 *    Use standard first header
 *
 * Revision 1.6  1994/02/19  04:10:06  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/02/19  03:55:45  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/02/18  23:13:01  ahd
 * Use standard first header
 *
 * Revision 1.3  1994/01/01  19:03:58  ahd
 * Annual Copyright Update
 *
 * Revision 1.2  1993/10/12  00:46:16  ahd
 * Normalize comments
 *
 * Revision 1.1  1993/09/25  03:02:37  ahd
 * Initial revision
 *
 * Revision 1.1  1993/09/24  21:43:27  dmwatt
 * Initial revision
 *
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <windows.h>

#include <io.h>
#include <errno.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "pnterr.h"

currentfile();

/*--------------------------------------------------------------------*/
/*    p N T e r r                                                     */
/*                                                                    */
/*    Perform a perror() with logging                                 */
/*--------------------------------------------------------------------*/

void pNTErr(const size_t lineno,
             const char *fname,
             const char *prefix,
             DWORD rc)
{
   char buf[BUFSIZ];
   static KWBoolean recursion  = KWFalse;
   int l;
   KWBoolean redirect = ((logfile != stdout) && !isatty(fileno(stdout)));

   DWORD xrc;
   xrc = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, rc, LANG_USER_DEFAULT, buf, BUFSIZ, NULL);

   if ( xrc == 0 )
   {

      if ( ! recursion )
      {
         recursion = KWTrue;
         printNTerror( "FormatMessage", xrc );
         recursion = KWFalse;
      } /* recursion */

      sprintf(buf, "NT API error %u in %s at line %d, cannot find message",
                   (unsigned int) rc,
                   fname,
                   lineno );

   } /* if ( xrc == 0 ) */

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

   printmsg(2,"NT API error %d in %s at line %d ...",
            (int) rc, fname, lineno );

   printmsg(0,"%s: %s", prefix, buf);

   if ( redirect )
      fprintf(stdout,"%s: %s\n", prefix, buf);

} /* pNTErr */
