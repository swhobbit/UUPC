/*--------------------------------------------------------------------*/
/*    t i m e s t m p . c                                             */
/*                                                                    */
/*    Compiler timestamps for display at program start-up             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: timestmp.c 1.25 2001/03/12 13:54:04 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: timestmp.c $
 *    Revision 1.25  2001/03/12 13:54:04  ahd
 *    Annual Copyright update
 *
 *    Revision 1.24  2000/05/12 12:30:48  ahd
 *    Annual copyright update
 *
 *    Revision 1.23  1999/01/08 02:20:52  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.22  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.21  1998/03/01 01:25:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1997/12/13 18:29:47  ahd
 *    Add RCSID to our master timestamp module
 *
 *    Revision 1.19  1997/12/13 18:05:06  ahd
 *    Shorten Windows ID string
 *
 *    Revision 1.18  1997/03/31 07:07:06  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1996/01/01 20:52:35  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1995/08/27 23:30:21  ahd
 *    Add debugging info to hello message
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <direct.h>
#include <io.h>

#ifdef _Windows
#include <windows.h>
#elif !defined(__32BIT__)
#include <dos.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"

#ifndef UUPCV
#include "win32ver.h"
#endif

#ifdef _Windows
#include "winutil.h"
#include "ssleep.h"
#endif

#include "title.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: timestmp.c 1.25 2001/03/12 13:54:04 ahd v1-13k $");

const char *compiled = __DATE__;
const char *compilet = __TIME__;
const char *compilev = UUPCV;

const char *compilep = "UUPC/extended";
char       *compilen = "?";

#ifdef _Windows
char *compilew = NULL;
#endif

/*--------------------------------------------------------------------*/
/*       b a n n e r                                                  */
/*                                                                    */
/*       Report program name and version at startup                   */
/*--------------------------------------------------------------------*/

void banner (char **argv)
{
      char dummy[FILENAME_MAX];
      char program[FILENAME_MAX];

#if defined(_Windows)
   WORD wVersion;
   WORD wMode;
   char *szMode;

/*--------------------------------------------------------------------*/
/*                     Determine Windows version                      */
/*--------------------------------------------------------------------*/

   wVersion = LOWORD(GetVersion());
   asm {
         MOV      AX,0x400   /* DPMI Get Version call                 */
         INT      0x31       /* ax=ver,bx=win286/386...               */
         AND      BX,3       /* win386=1,win286=2                     */
         MOV      wMode,BX   /* Put in local                          */
   }

   switch(wMode)
   {
      case 1:
         szMode = "386 Enhanced";
         break;

      case 2:
         szMode = "Standard (286)";
         break;

      default:
         szMode = "Unknown";
         break;
   }

   if ( wVersion < 0x301 )
   {
      printmsg(0,"This program requires Windows 3.1, running %u.%u",
                  (WORD)(LOBYTE(wVersion)),
                  (WORD)(HIBYTE(wVersion)));
      panic();
   }

   sprintf(dummy, "Windows %u.%u %s",
                  (WORD)(LOBYTE(wVersion)),
                  (WORD)(HIBYTE(wVersion)),
                  szMode );
   compilew = newstr( dummy );

#endif

/*--------------------------------------------------------------------*/
/*                     Deterine the program name                      */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
      if (  fnsplit(argv[0],dummy,dummy, program,dummy) && FILENAME )
      {
#else
      if (!equal(argv[0],"C"))    /* Microsoft C for no prog name? */
      {
         _splitpath( argv[0], dummy , dummy , program , dummy );
#endif /* __TURBOC__ */

         strcpy(argv[0], program);  /* Reset original program name    */
         compilen = argv[0];

/*--------------------------------------------------------------------*/
/*                 Return if input is not the console                 */
/*--------------------------------------------------------------------*/

#ifndef _Windows
      if (!isatty(fileno(stdout))) /* Is the console I/O redirected?  */
         return;                 /* Yes --> Run quietly               */
#endif

/*--------------------------------------------------------------------*/
/*                       Print the program name                       */
/*--------------------------------------------------------------------*/

         fprintf(stderr,"%s: ",program);
      } /* if */

/*--------------------------------------------------------------------*/
/*    Now print out the version, operating system and time stamp      */
/*--------------------------------------------------------------------*/

      fprintf(stderr,"%s %s (%s mode, %2.2s%3.3s%2.2s %5.5s)"
#ifdef UDEBUG
                  " [debug enabled]"
#endif /* UDEBUG */

                  "\n",
                  compilep,
                  compilev,

#ifdef WIN32
                  "Win32",
#elif defined(_Windows)
                  /* "Windows 16 bit", */ compilew,
#elif defined(__OS2__)
                  "OS/2 32 bit",
#elif defined(__TURBOC__)
                  "DOS",
#else
                  (_osmode == DOS_MODE) ? "DOS" : "OS/2 16 bit",
#endif

                  &compiled[4],
                  &compiled[0],
                  &compiled[9],
                  compilet);

#ifdef _Windows
   hOurTask = GetCurrentTask();
   if ( hOurTask == NULL )
      panic();
   hOurWindow = FindTaskWindow(hOurTask, "BCEasyWin");

   ddelay(0);           /* To insure we yield control at least once;  */
                        /* see comments in winutil.c.                 */
#endif

   setTitle("%s %s", compilep, compilev );

} /* banner */
