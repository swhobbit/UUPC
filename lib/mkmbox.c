/*--------------------------------------------------------------------*/
/*    m k m b o x . h                                                 */
/*                                                                    */
/*    Host Support routines for UUPC/extended                         */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of hlib.c                         ahd    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*    m k m a i l b o x                                               */
/*                                                                    */
/*    Build a mailbox name                                            */
/*--------------------------------------------------------------------*/

char *mkmailbox(char *buf, const char *userid)
{
   boolean append = ( E_mailext != NULL );

   if (buf == NULL)           /* Do we need to allocate buffer?         */
   {
      buf = malloc( FILENAME_MAX );
      checkref(buf);
   } /* if */

   if (bflag[F_DIRECT])
   {
      sprintf(buf,"%s%c%s%c%s", E_maildir, SEPCHAR,
                               userid, SEPCHAR,"newmail");
   } /* if (bflag[F_DIRECT]) */
   else {
      char tuser[FILENAME_MAX];
      strcpy( tuser, userid );

      if (strchr(userid, '.') == NULL )
         tuser[8] = '\0';        /* Auto-truncate mbox name          */
      else
         append = FALSE;

      mkfilename(buf, E_maildir, tuser);
   } /* else */

/*--------------------------------------------------------------------*/
/*              If we need a mailbox extension, add one               */
/*--------------------------------------------------------------------*/

   if ( append )
      strcat( strcat(buf,".") , E_mailext );

   return buf;

} /* mkmailbox */
