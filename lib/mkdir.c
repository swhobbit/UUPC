/*--------------------------------------------------------------------*/
/*    m k d i r . c                                                   */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mkdir.c 1.17 1999/01/08 02:20:48 ahd Exp $
 *
 *    Revision history:
 *    $Log: mkdir.c $
 *    Revision 1.17  1999/01/08 02:20:48  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.16  1999/01/04 03:52:55  ahd
 *    Annual copyright change
 *
 *    Revision 1.15  1998/03/01 01:24:16  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1997/03/31 07:04:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1996/01/01 20:53:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1994/12/22 00:09:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1994/02/20 19:07:38  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.10  1994/02/19  04:43:39  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:53:30  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/02/19  03:53:30  ahd
 *     Use standard first header
 *
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

#include "uupcmoah.h"

#include <direct.h>

RCSID("$Id: mkdir.c 1.17 1999/01/08 02:20:48 ahd Exp $");

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

      mkdir(path);
      *cp = '/';
      cp++;
   }

/*--------------------------------------------------------------------*/
/*                           Make last dir                            */
/*--------------------------------------------------------------------*/

   return mkdir((char *) inpath);

} /*MKDIR*/
