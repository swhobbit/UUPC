/*--------------------------------------------------------------------*/
/*    s c r i p t . h                                                 */
/*                                                                    */
/*    Script processing routines used by UUPC/extended                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: script.h 1.12 2001/03/12 13:51:16 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: script.h $
 *    Revision 1.12  2001/03/12 13:51:16  ahd
 *    Annual copyright update
 *
 *    Revision 1.11  2000/05/12 12:41:43  ahd
 *    Annual copyright update
 *
 *    Revision 1.10  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
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
