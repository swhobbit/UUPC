#ifndef _DCPTPKT_H
#define _DCPTPKT_H

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
  *      $Id: DCPFPKT.H 1.3 1993/04/05 12:27:31 ahd Exp $
  *
  *      $Log: DCPFPKT.H $
  */

short  topenpk(const boolean master);

short  tclosepk(void);

short  tgetpkt(char  *data,short  *len);

short  tsendpkt(char  *data,short  len);

short  twrmsg(char *str);

short  trdmsg(char *str);

short  teofpkt( void );

short  tfilepkt( void );

#endif
