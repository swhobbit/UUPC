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
 *    $Id: lib.h 1.10 1993/07/22 23:26:19 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "win32ver.h"
#include "timestmp.h"

#ifdef _Windows
#include "winutil.h"
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
         MOV      AX,0x400   // DPMI Get Version call
         INT      0x31       // ax=ver,bx=win286/386...
         AND      BX,3       // win386=1,win286=2
         MOV      wMode,BX   // Put in local
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
         printmsg(0,"Program not supported in unknown Windows environment");
         panic();
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

   _InitEasyWin();

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

         strcpy(argv[0], program);  /* Reset original program name   */
         compilen = argv[0];

/*--------------------------------------------------------------------*/
/*                 Return if input is not the console                 */
/*--------------------------------------------------------------------*/

      if (!isatty(fileno(stdout))) /* Is the console I/O redirected?  */
         return;                 /* Yes --> Run quietly              */

/*--------------------------------------------------------------------*/
/*                       Print the program name                       */
/*--------------------------------------------------------------------*/

         fprintf(stderr,"%s: ",program);
      } /* if */

/*--------------------------------------------------------------------*/
/*    Now print out the version, operating system (MS C only) and     */
/*    timestamp                                                       */
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
   hOurWindow = FindTaskWindow(hOurTask, "BCEasyWin");
   SetWindowText(hOurWindow, dummy);
#endif

} /* banner */
