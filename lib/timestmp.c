/*--------------------------------------------------------------------*/
/*    timestmp.c                                                      */
/*                                                                    */
/*    Compiler timestamps for display at program start-up             */
/*                                                                    */
/*    History:                                                        */
/*                                                                    */
/*       12/13/89 Add Copyright statements - ahd                      */
/*--------------------------------------------------------------------*/

#include <dos.h>
#include <direct.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __TURBOC__
#include "win32ver.h"
#endif

#include "lib.h"
#include "timestmp.h"

#ifndef UUPCV
#define UUPCV "1.11(experimental)"
#endif

char compiled[] = { __DATE__ } ;
char compilet[] = { __TIME__ } ;
char compilev[] = { UUPCV } ;

char compilep[] = { "UUPC/extended" } ;

char *compilen  = compilep;

void banner (char **argv)
{
      char dummy[FILENAME_MAX];
      char program[FILENAME_MAX];

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
                  "Windows 16 bit",
#elif defined(__TURBOC__)
                  "real",
#elif defined(__OS2__)
                  "OS/2 2.x",
#else
                  (_osmode == DOS_MODE) ? "DOS" : "OS/2 1.x",
#endif

                  &compiled[4],
                  &compiled[0],
                  &compiled[9],
                  compilet);
} /* banner */
