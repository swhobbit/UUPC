/*--------------------------------------------------------------------*/
/*       s s l e e p . h                                              */
/*                                                                    */
/*       Time delay function definitions for UUPC/extended            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ssleep.h 1.2 1993/09/20 04:51:31 ahd Exp $
 *
 *    Revision history:
 *    $Log: ssleep.h $
 *     Revision 1.2  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 */

void ssleep(time_t interval);

void ddelay(KEWSHORT milliseconds);
