/*--------------------------------------------------------------------*/
/*       t r u m p e t . c                                            */
/*                                                                    */
/*       Audio support for UUPC/extended                              */
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
 *    $Id: lib.h 1.10 1993/07/22 23:26:19 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

/*--------------------------------------------------------------------*/
/*    Use a complex beep upon mail delivery if way to control the     */
/*    speaker is available; if using MS C 6.0 under DOS, we can't     */
/*    so don't try                                                    */
/*--------------------------------------------------------------------*/

#if defined(__TURBOC__) && !defined(_Windows)
#define SMARTBEEP
#endif

#if defined(FAMILYAPI) || defined(WIN32)
#define SMARTBEEP
#endif

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
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

#include "lib.h"

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

   if (tune == NULL)          /* Should we announce?  */
      return;                 /* No --> Return quietly (literally)   */

/*--------------------------------------------------------------------*/
/*             We are to announce the arrival of the mail             */
/*--------------------------------------------------------------------*/

#ifdef SMARTBEEP
   strcpy(buf,tune);          /* Save the data                       */

   while( (token = strtok( token, ",")) != NULL)
   {
      tone = (size_t) atoi(token);
      token = strtok( NULL, ",");
      duration = (token == NULL) ? 500 : (size_t) atoi(token);

#ifdef __TURBOC__
      if (tone == 0)
         nosound();
      else
         sound( tone );
      ddelay( duration );
#else
      if (tone == 0)
         ddelay(duration);
      else {
#ifdef WIN32
         Beep( tone, duration );
#else
         DosBeep( tone, duration );
#endif
      }
#endif /* __TURBOC__ */

      token = NULL;           /* Look at next part of string   */
   } /* while */

#ifdef __TURBOC__
   nosound();
#endif
#else /* SMARTBEEP */

/*--------------------------------------------------------------------*/
/*      We cannot play the requested tune; just beep at the user      */
/*--------------------------------------------------------------------*/

   fputc('\a', stdout);
#endif /* SMARTBEEP */

} /* trumpet */
