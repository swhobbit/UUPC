/*--------------------------------------------------------------------*/
/*       t i t l e . h                                                */
/*                                                                    */
/*       Change title name/session list info for a program            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: title.h 1.3 1996/01/01 20:58:08 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: title.h $
 *    Revision 1.3  1996/01/01 20:58:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/12/22 00:14:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1994/04/24 20:17:29  dmwatt
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*       s e t T i t l e                                              */
/*                                                                    */
/*       Set task list entry for program under OS/2                   */
/*--------------------------------------------------------------------*/

void setTitle( const char *format, ... );
