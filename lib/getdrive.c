/*--------------------------------------------------------------------*/
/*       g e t d r i v e . c                                          */
/*                                                                    */
/*       Determine current drive letter efficiently                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: getdrive.c 1.1 1995/02/12 23:37:04 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: getdrive.c $
 *    Revision 1.1  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
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
