#ifndef _DCPEPKT_H
#define _DCPEPKT_H

/*--------------------------------------------------------------------*/
/*       d c p t p k t . h                                            */
/*                                                                    */
/*       "t" protocol packet driver for dcp (UUPC/extended data       */
/*       communications)                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

 /*
  *      $Id: dcpepkt.h 1.1 1993/12/02 04:00:19 dmwatt Exp $
  *
  *      $Log: dcpepkt.h $
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

short  eopenpk(const boolean master);

short  eclosepk(void);

short  egetpkt(char  *data,short  *len);

short  esendpkt(char  *data,short  len);

short  ewrmsg(char *str);

short  erdmsg(char *str);

short  eeofpkt( void );

short  efilepkt( const boolean master, const unsigned long bytes );

#endif
