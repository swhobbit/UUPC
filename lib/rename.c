#include "uupcmoah.h"

RCSID("$Id$");

/*--------------------------------------------------------------------*/
/*    r e n a m e . c                                                 */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    R E N A M E                                                     */
/*                                                                    */
/*    Rename a file, creating the target directory if needed          */
/*--------------------------------------------------------------------*/

int RENAME(const char *oldname, const char *newname )
{

   char *last;

/*--------------------------------------------------------------------*/
/*                     Attempt to rename the file                     */
/*--------------------------------------------------------------------*/

   if (!rename( oldname, newname )) /* Success?                       */
      return 0;                     /* Yes --> Return to caller       */

/*--------------------------------------------------------------------*/
/*      Try rebuilding the directory and THEN renaming the file       */
/*--------------------------------------------------------------------*/

   if ((last = strrchr(newname, '/')) != nil(char))
   {
      *last = '\0';
      MKDIR(newname);
      *last = '/';
   }

   return rename( oldname, newname );
} /* RENAME */
