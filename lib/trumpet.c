/*--------------------------------------------------------------------*/
/*       t r u m p e t . c                                            */
/*                                                                    */
/*       Audio support for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: trumpet.c 1.8 1994/02/19 04:47:22 ahd Exp $
 *
 *    Revision history:
 *    $Log: trumpet.c $
 * Revision 1.8  1994/02/19  04:47:22  ahd
 * Use standard first header
 *
 * Revision 1.7  1994/02/19  04:12:35  ahd
 * Use standard first header
 *
 * Revision 1.6  1994/02/19  03:59:45  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/02/18  23:15:56  ahd
 * Use standard first header
 *
 * Revision 1.4  1994/01/01  19:06:37  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1993/10/12  00:48:44  ahd
 * Normalize comments
 *
 * Revision 1.2  1993/09/20  04:39:51  ahd
 * OS/2 2.x support
 *
 * Revision 1.1  1993/07/31  16:22:16  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*    Use a complex beep upon mail delivery if way to control the     */
/*    speaker is available; if using MS C 6.0 under DOS, we can't     */
/*    so don't try                                                    */
/*--------------------------------------------------------------------*/

#if defined(__TURBOC__) && !defined(_Windows)
#define SMARTBEEP
#endif

#if defined(FAMILYAPI) || defined(WIN32) || defined(__OS2__)
#define SMARTBEEP
#endif

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <io.h>

#include <ctype.h>
#include <process.h>
#include <limits.h>

#ifdef __TURBOC__
#include <dos.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#if defined(FAMILYAPI) || defined(__OS2__)
#include <os2.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#ifdef SMARTBEEP
#include "ssleep.h"
#endif

/*--------------------------------------------------------------------*/
/*    t r u m p e t                                                   */
/*                                                                    */
/*    Trumpet the arrival of remote mail to a local user              */
/*--------------------------------------------------------------------*/

void trumpet( const char *tune)
{
#ifdef SMARTBEEP
   char buf[BUFSIZ];
   char *token = buf;
   size_t tone, duration;
#endif

   if ( ! bflag[F_SUPPRESSBEEP]  || (tune == NULL) )
                              /* Should we announce?                  */
      return;                 /* No --> Return quietly (literally)    */

/*--------------------------------------------------------------------*/
/*             We are to announce the arrival of the mail             */
/*--------------------------------------------------------------------*/

#ifdef SMARTBEEP
   strcpy(buf,tune);          /* Save the data                        */

   while( (token = strtok( token, ",")) != NULL)
   {
      tone = (size_t) atoi(token);
      token = strtok( NULL, ",");
      duration = (token == NULL) ? 500 : (size_t) atoi(token);

#ifdef WIN32
      Beep( tone, duration );

      if (tone == 0)
         ddelay(duration);

#elif defined(FAMILYAPI) || defined(__OS2__)

      DosBeep( tone, duration );

      if (tone == 0)
         ddelay(duration);

#else
      if (tone != 0)
         sound( tone );

      ddelay(duration);

      nosound();

#endif /* __TURBOC__ */

      token = NULL;           /* Look at next part of string   */
   } /* while */

#else /* SMARTBEEP */

/*--------------------------------------------------------------------*/
/*      We cannot play the requested tune; just beep at the user      */
/*--------------------------------------------------------------------*/

   fputc('\a', stdout);
#endif /* SMARTBEEP */

} /* trumpet */
