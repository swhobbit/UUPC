/*--------------------------------------------------------------------*/
/*       d c p l i b . h                                              */
/*                                                                    */
/*       System dependent (non-communications related) routines       */
/*       for UUPC/extended.                                           */
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
 *    $Id: dcplib.h 1.11 2001/03/12 13:51:16 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: dcplib.h $
 *    Revision 1.11  2001/03/12 13:51:16  ahd
 *    Annual copyright update
 *
 *    Revision 1.10  2000/05/12 12:41:43  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.8  1998/03/01 01:42:24  ahd
 *    Annual Copyright Update
 *
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
