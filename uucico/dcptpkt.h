#ifndef _DCPTPKT_H
#define _DCPTPKT_H

/*--------------------------------------------------------------------*/
/*       d c p t p k t . h                                            */
/*                                                                    */
/*       "t" protocol packet driver for dcp (UUPC/extended data       */
/*       communications)                                              */
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
  *      $Id: dcptpkt.h 1.4 1994/01/01 19:23:24 ahd v1-12k $
  *
  *      $Log: dcptpkt.h $
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

short  topenpk(const boolean master);

short  tclosepk(void);

short  tgetpkt(char  *data,short  *len);

short  tsendpkt(char  *data,short  len);

short  twrmsg(char *str);

short  trdmsg(char *str);

short  teofpkt( void );

short  tfilepkt( const boolean master, const unsigned long bytes );

#endif
