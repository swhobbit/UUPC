#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*    m k m b o x . c                                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mkmbox.c 1.14 1997/03/31 07:05:00 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: mkmbox.c $
 *    Revision 1.14  1997/03/31 07:05:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1996/01/01 20:54:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.11  1995/01/07 16:13:16  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.10  1994/12/22 00:09:34  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/02/19 04:43:55  ahd
 *    Use standard first header
 *
 *     Revision 1.8  1994/02/19  04:08:53  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/19  03:53:55  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/02/18  23:11:39  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/01/24  03:08:53  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.4  1994/01/01  19:03:14  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.3  1993/10/12  00:46:16  ahd
 *     Normalize comments
 *
 *     Revision 1.2  1993/07/13  01:13:32  ahd
 *     Limit directory names for users to eight characters
 *
 *
 *       21Nov1991 Break out of hlib.c                         ahd
 */

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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
   KWBoolean append = (KWBoolean) (( E_mailext != NULL ) ? KWTrue : KWFalse);

   if (buf == NULL)           /* Do we need to allocate buffer?       */
   {
      buf = malloc( FILENAME_MAX );
      checkref(buf);
   } /* if */

   if (bflag[F_DIRECT])
   {
      sprintf(buf,"%s%c%.8s%c%s", E_maildir, SEPCHAR,
                               userid, SEPCHAR,"newmail");
   } /* if (bflag[F_DIRECT]) */
   else {
      char tuser[FILENAME_MAX];
      strcpy( tuser, userid );

      if (strchr(userid, '.') == NULL )
         tuser[8] = '\0';        /* Auto-truncate mbox name           */
      else
         append = KWFalse;

      mkfilename(buf, E_maildir, tuser);
   } /* else */

/*--------------------------------------------------------------------*/
/*              If we need a mailbox extension, add one               */
/*--------------------------------------------------------------------*/

   if ( append )
      strcat( strcat(buf,".") , E_mailext );

   return buf;

} /* mkmailbox */
