/*--------------------------------------------------------------------*/
/*    m k t e m p n m . c                                             */
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
#include <limits.h>
#include <time.h>

#ifdef __GNUC__
#include <unistd.h>
#else
#include <io.h>
#endif

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Header$
 *
 *    Revision history:
 *    $Log$
 */

static char rcsid[] = "$Id$";

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
/*    m k t e m p n a m e                                             */
/*                                                                    */
/*    Generate a temporary name with a pre-defined extension          */
/*--------------------------------------------------------------------*/

char *mktempname( char *buf, char *extension)
{
   static size_t file = 0;
   if (buf == NULL)           /* Do we need to allocate buffer?         */
   {
      buf = malloc( FILENAME_MAX );
      checkref(buf);
   } /* if */

   for (file++ ; file < INT_MAX ; file++ )
   {
      sprintf(buf,"%s/uupc%04.4x.%s", E_tempdir, file, extension);
      if ( access( buf, 0 ))  /* Does the host file exist?           */
         break;               /* No  --> Use the name                */
   } /* for */

   printmsg(5,"Generated temporary name: %s",buf);
   return buf;

} /* mktempname */
