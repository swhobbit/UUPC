/*--------------------------------------------------------------------*/
/*    s c r i p t . h                                                 */
/*                                                                    */
/*    Script processing routines used by UUPC/extended                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: script.h 1.9 1998/03/01 01:42:50 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: script.h $
 *    Revision 1.9  1998/03/01 01:42:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/04/24 01:37:51  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/01 21:24:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1995/01/07 16:48:56  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.5  1994/12/22 00:40:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/01/01 19:24:07  ahd
 *    Annual Copyright Update
 *
 * Revision 1.3  1993/12/24  05:26:43  ahd
 * Add RCS header
 *
 */

int expectstr(char *Search, unsigned int Timeout, char **failure);

KWBoolean sendstr(char  *str, unsigned int timeout, char **failure);

void echoCheck( const unsigned int timeout );
