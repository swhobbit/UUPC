/*--------------------------------------------------------------------*/
/*       g e t d r i v e . c                                          */
/*                                                                    */
/*       Determine current drive letter efficiently                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.30 1995/01/30 04:05:39 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#include <uupcmoah.h>

#include <direct.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/
/*       g e t D r i v e                                              */
/*                                                                    */
/*       Get the current drive letter                                 */
/*--------------------------------------------------------------------*/

char getDrive( const char *currentDirectory )
{

   if (( currentDirectory != NULL ) &&
       isalpha( currentDirectory[0] ) &&
       (currentDirectory[1] == ':' ))  /* Current dir has drive?     */
      return (char) toupper(*currentDirectory);
                                    /* Yes --> Use it                */

   if (( E_cwd != NULL ) &&
       isalpha( E_cwd[0] ) &&
       (E_cwd[1] == ':' ))          /* Current dir has drive?        */
      return (char) toupper(*E_cwd);/* Yes --> Use it, of course     */

/*--------------------------------------------------------------------*/
/*         Determine the current drive letter from the system         */
/*--------------------------------------------------------------------*/

    return (char) toupper(_getdrive() + 'A' - 1);

} /* getDrive */
