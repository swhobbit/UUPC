/*--------------------------------------------------------------------*/
/*    m k d i r . c                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mkdir.c 1.8 1994/02/18 23:11:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: mkdir.c $
 *     Revision 1.8  1994/02/18  23:11:20  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/01/24  03:08:38  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.6  1994/01/01  19:03:02  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.5  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.4  1993/04/11  00:31:31  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.3  1993/03/24  01:57:30  ahd
 *     Delete unneeded currentfile()
 *
 */

#ifdef __GNUC__

#include "uupcmoah.h"
#include <os2.h>
#else
#include <direct.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    M K D I R                                                       */
/*                                                                    */
/*    Like mkdir() but create intermediate directories as well        */
/*--------------------------------------------------------------------*/

int MKDIR(const char *inpath)
{
   char *cp;
   char *path;

   if (*inpath == '\0')
      return 0;

   cp = path = normalize(inpath );

/*--------------------------------------------------------------------*/
/*        See if we need to make any intermediate directories         */
/*--------------------------------------------------------------------*/

   cp = path ;
   while ((cp = strchr(cp, '/')) != nil(char)) {
      *cp = '\0';

#ifndef __GNUC__
      mkdir(path);
#else
      DosCreateDir( path, 0);
#endif
      *cp = '/';
      cp++;
   }

/*--------------------------------------------------------------------*/
/*                           Make last dir                            */
/*--------------------------------------------------------------------*/

   return mkdir((char *) inpath);

} /*MKDIR*/
