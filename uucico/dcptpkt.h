#ifndef _DCPTPKT_H
#define _DCPTPKT_H

/*--------------------------------------------------------------------*/
/*       d c p t p k t . h                                            */
/*                                                                    */
/*       "t" protocol packet driver for dcp (UUPC/extended data       */
/*       communications)                                              */
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
  *      $Id: dcptpkt.h 1.8 1997/04/24 01:37:22 ahd v1-12u $
  *
  *      $Log: dcptpkt.h $
  *      Revision 1.8  1997/04/24 01:37:22  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.7  1996/01/01 21:25:02  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.6  1995/01/07 16:47:13  ahd
  *      Change boolean to KWBoolean to avoid VC++ 2.0 conflict
  *
  *      Revision 1.5  1994/12/22 00:40:15  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.4  1994/01/01 19:23:24  ahd
  *      Annual Copyright Update
  *
 * Revision 1.3  1993/12/02  13:50:40  ahd
 * 'e' protocol support
 *
 * Revision 1.2  1993/12/02  04:00:19  dmwatt
 * 'e' protocol support
 *
 * Revision 1.1  1993/09/18  19:47:42  ahd
 * Initial revision
 *
 * Revision 1.1  1993/09/18  19:47:42  ahd
 * Initial revision
 *
  */

short  topenpk(const KWBoolean master);

short  tclosepk(void);

short  tgetpkt(char  *data,short  *len);

short  tsendpkt(char  *data,short  len);

short  twrmsg(char *str);

short  trdmsg(char *str);

short  teofpkt( void );

short  tfilepkt( const KWBoolean master, const unsigned long bytes );

#endif
