/*--------------------------------------------------------------------*/
/*       t r u m p e t . c                                            */
/*                                                                    */
/*       Audio support for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: trumpet.c 1.16 1998/03/08 23:07:12 ahd Exp $
 *
 *    Revision history:
 *    $Log: trumpet.c $
 *    Revision 1.16  1998/03/08 23:07:12  ahd
 *    Add support for WAV files under Windows 32 bit
 *
 *    Revision 1.15  1998/03/01 01:25:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1997/03/31 07:07:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1996/01/01 20:53:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/03/12 16:42:24  ahd
 *    Suppress compiler warnings
 *
 *    Revision 1.11  1995/02/20 17:28:43  ahd
 *    16 bit compiler warning message clean up
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
#include <time.h>

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

void trumpet( const char *tune, KWBoolean daemon, time_t last)
{
   static time_t now = 0;

#ifdef SMARTBEEP
   char buf[BUFSIZ];
   char *token = buf;
   KEWSHORT tone;
   KEWSHORT duration;
#endif

   if (bflag[F_SUPPRESSBEEP] || (tune == NULL))
                              /* Should we announce?                  */
      return;                 /* No --> Return quietly (literally)    */

/*--------------------------------------------------------------------*/
/*       We make a distinct check to suppress the announcement;       */
/*       the first is if the system mailbox exists (no pop3 client    */
/*       has asked about it) and we updated the mailbox within 30     */
/*       seconds, we don't beep.                                      */
/*                                                                    */
/*       This could result in a string of mailbox updates 29          */
/*       seconds apart keeping the announcement quiet for a long      */
/*       time, but that's a feature.                                  */
/*--------------------------------------------------------------------*/

   time(&now);

   if ((last + 30) > now)
      return;

/*--------------------------------------------------------------------*/
/*             We are to announce the arrival of the mail             */
/*--------------------------------------------------------------------*/

#ifdef SMARTBEEP

   strcpy(buf,tune);          /* Save the data                        */

   while( (token = strtok( token, ",")) != NULL)
   {

/*--------------------------------------------------------------------*/
/*                     Handle playing a WAV file                      */
/*--------------------------------------------------------------------*/

      if (strchr(token,'.'))
      {
#ifdef WIN32
         denormalize(token);
#ifdef UDEBUG
         printmsg(4,"trumpet: Announcing mail with %s", token );
#endif
         /* Daemons exist long enough we don't have to wait for sound */
         PlaySound(token, NULL, ( (daemon ? SND_ASYNC : 0) |
                                SND_FILENAME |
                                0 /* SND_NODEFAULT */ ));
         break;
#else
         /* Ignore it under environments which do not support WAV files */
         token = NULL;              /* Step to next part of string */
         continue;
#endif
      }

      tone = (KEWSHORT) atoi(token);
      token = strtok( NULL, ",");

      if ( token == NULL )
         duration = 500;
      else
         duration = (KEWSHORT) atoi(token);

#ifdef WIN32
      Beep( tone, duration );

      if (tone == 0)
         ddelay(duration);

#elif defined(FAMILYAPI) || defined(__OS2__)

      DosBeep( tone, duration );

      if (tone == 0)
         ddelay((KEWSHORT) duration);

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
