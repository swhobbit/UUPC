#ifndef _DCPFPKT_H
#define _DCPFPKT_H

/*--------------------------------------------------------------------*/
/*       d c p f p k t . h                                            */
/*                                                                    */
/*       "f" protocol packet driver for dcp (UUPC/extended data       */
/*       communications)                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

 /*
  *      $Id: dcpfpkt.h 1.12 1998/03/01 01:42:18 ahd v1-13f ahd $
  *
  *      $Log: dcpfpkt.h $
  *      Revision 1.12  1998/03/01 01:42:18  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.11  1997/04/24 01:36:54  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.10  1996/01/01 21:24:25  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.9  1995/01/07 16:45:38  ahd
  *      Change boolean to KWBoolean to avoid VC++ 2.0 conflict
  *
  *      Revision 1.8  1994/12/22 00:39:41  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.7  1994/01/01 19:23:03  ahd
  *      Annual Copyright Update
  *
 * Revision 1.6  1993/12/02  13:50:40  ahd
 * 'e' protocol support
 *
 * Revision 1.5  1993/12/02  04:00:19  dmwatt
 * 'e' protocol support
 *
 * Revision 1.4  1993/09/20  04:53:57  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 * Revision 1.3  1993/04/05  12:27:31  ahd
 * Correct protypes to match gpkt
 *
 * Revision 1.2  1992/11/15  20:08:29  ahd
 * Clean up modem file support for different procotols
 *
 *    08 Sep 90   -  Create via Microsoft C compiler /Zg          ahd
 *    21 Aug 91   -  Create from dcpgpkt.c                        ahd
 *
  */

short  fopenpk(const KWBoolean master);

short  fclosepk(void);

short  fgetpkt(char  *data,short  *len);

short  fsendpkt(char  *data,short  len);

short  fwrmsg(char *str);

short  frdmsg(char *str);

short  feofpkt( void );

short  ffilepkt( const KWBoolean master, const unsigned long bytes );

#endif
