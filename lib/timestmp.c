/*--------------------------------------------------------------------*/
/*    t i m e s t m p . c                                             */
/*                                                                    */
/*    Compiler timestamps for display at program start-up             */
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
 *    $Id: timestmp.c 1.8 1993/08/11 02:31:12 ahd Exp $
 *
 *    Revision history:
 *    $Log: timestmp.c $
 *     Revision 1.8  1993/08/11  02:31:12  ahd
 *     Always display the banner
 *
 *     Revision 1.7  1993/08/03  03:11:49  ahd
 *     Drop _EasyWinInit
 *
 *     Revision 1.6  1993/08/02  03:24:59  ahd
 *     Further changes in support of Robert Denny's Windows 3.x support
 *
 *     Revision 1.5  1993/07/31  16:22:16  ahd
 *     Changes in support of Robert Denny's Windows 3.x support
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <dos.h>
#include <direct.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _Windows
#include <windows.h>
#include <time.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "win32ver.h"
#include "timestmp.h"

#ifdef _Windows
#include "winutil.h"
#include "ssleep.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#ifdef _Windows
currentfile();
#endif

char compiled[] = { __DATE__ } ;
char compilet[] = { __TIME__ } ;
char compilev[] = { UUPCV } ;

char compilep[] = { "UUPC/extended" } ;
char *compilen  = compilep;

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

      fprintf(stderr,"%s %s (%s mode, %2.2s%3.3s%2.2s %5.5s)\n",
                  compilep,
                  compilev,

#ifdef WIN32
                  "Windows 32 bit",
#elif defined(_Windows)
                  /* "Windows 16 bit", */ compilew,
#elif defined(__OS2__)
                  "OS/2 2.x",
#elif defined(__TURBOC__)
                  "DOS",
#else
                  (_osmode == DOS_MODE) ? "DOS" : "OS/2 1.x",
#endif

                  &compiled[4],
                  &compiled[0],
                  &compiled[9],
                  compilet);

#ifdef _Windows
   sprintf( dummy, "%s  (%s %s)", program, compilep, compilev);

   hOurTask = GetCurrentTask();
   if ( hOurTask == NULL )
      panic();
   hOurWindow = FindTaskWindow(hOurTask, "BCEasyWin");
   SetWindowText(hOurWindow, dummy);

   ddelay(0);           /* To insure we yield control at least once;  */
                        /* see comments in winutil.c.                 */
#endif

} /* banner */
