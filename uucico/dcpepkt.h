#ifndef _DCPEPKT_H
#define _DCPEPKT_H

/*--------------------------------------------------------------------*/
/*       d c p t p k t . h                                            */
/*                                                                    */
/*       "t" protocol packet driver for dcp (UUPC/extended data       */
/*       communications)                                              */
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
  *      $Id: dcpepkt.h 1.6 1996/01/01 21:23:30 ahd v1-12r $
  *
  *      $Log: dcpepkt.h $
  *      Revision 1.6  1996/01/01 21:23:30  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.5  1995/01/07 16:45:23  ahd
  *      Change boolean to KWBoolean to avoid VC++ 2.0 conflict
  *
  *      Revision 1.4  1994/12/22 00:39:31  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.3  1994/01/01 19:22:57  ahd
  *      Annual Copyright Update
  *
 * Revision 1.2  1993/12/02  13:50:40  ahd
 * 'e' protocol support
 *
 * Revision 1.1  1993/12/02  04:00:19  dmwatt
 * 'e' protocol support
 *
 * Revision 1.1  1993/09/18  19:47:42  ahd
 * Initial revision
 *
 * Revision 1.1  1993/09/18  19:47:42  ahd
 * Initial revision
 *
  */

short  eopenpk(const KWBoolean master);

short  eclosepk(void);

short  egetpkt(char  *data,short  *len);

short  esendpkt(char  *data,short  len);

short  ewrmsg(char *str);

short  erdmsg(char *str);

short  eeofpkt( void );

short  efilepkt( const KWBoolean master, const unsigned long bytes );

#endif
