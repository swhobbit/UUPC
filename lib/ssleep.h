/*--------------------------------------------------------------------*/
/*       s s l e e p . h                                              */
/*                                                                    */
/*       Time delay function definitions for UUPC/extended            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ssleep.h 1.10 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: ssleep.h $
 *    Revision 1.10  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.9  1998/03/01 01:27:42  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/03/31 07:06:51  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1996/01/01 20:57:03  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1995/05/07 14:40:13  ahd
 *    Add checkForBreak
 *
 *    Revision 1.5  1995/02/22 12:15:00  ahd
 *    Correct 16 bit compiler warning errors
 *
 *    Revision 1.4  1994/12/22 00:13:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/01/01 19:09:51  ahd
 *    Annual Copyright Update
 *
 *     Revision 1.2  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 */

void ssleep(const time_t interval);

void ddelay(const KEWSHORT milliseconds);

void checkForBreak( void );
