/*--------------------------------------------------------------------*/
/*    b u g o u t . c                                                 */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "catcher.h"
#include "timestmp.h"

#ifdef _Windows
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*    b u g o u t                                                     */
/*                                                                    */
/*    Perform a panic() exit from UUPC/extended                       */
/*--------------------------------------------------------------------*/

void bugout( const size_t lineno, const char *fname )
{

#ifdef _Windows
   char buf[BUFSIZ];
#endif

  printmsg(0,"%s aborting at line %d in file %s",
              compilen, lineno, fname );

#ifdef _Windows
   sprintf(buf,"%s aborting at line %d in file %s",
              compilen, lineno, fname );

   MessageBox( hOurWindow, buf, compilen, MB_ICONEXCLAMATION );

#else
   fputc('\a',stderr);
#endif

   exit(panic_rc);
} /*bugout */
