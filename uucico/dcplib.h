/*--------------------------------------------------------------------*/
/*       d c p l i b . h                                              */
/*                                                                    */
/*       System dependent (non-communications related) routines       */
/*       for UUPC/extended.                                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcplib.h 1.7 1997/04/24 01:37:05 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: dcplib.h $
 *    Revision 1.7  1997/04/24 01:37:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1996/01/01 21:23:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1995/01/07 16:46:21  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.4  1994/12/22 00:40:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/05/24 03:45:09  ahd
 *    Clean up header comments
 *
 *        Revision 1.2  1994/05/06  03:58:32  ahd
 *        Add copyright
 *
 */

KWBoolean login(void);

KWBoolean loginbypass(const char *user);
