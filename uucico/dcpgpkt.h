/*--------------------------------------------------------------------*/
/*       d c p g p k t . h                                            */
/*                                                                    */
/*       "g" protocol packet driver for dcp (UUPC/extended data       */
/*       communications)                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

 /*
  *      $Id: dcpgpkt.h 1.10 1996/01/01 21:23:18 ahd v1-12r $
  *
  *      $Log: dcpgpkt.h $
  *      Revision 1.10  1996/01/01 21:23:18  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.9  1995/01/07 16:45:55  ahd
  *      Change boolean to KWBoolean to avoid VC++ 2.0 conflict
  *
  *      Revision 1.8  1994/12/22 00:39:51  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.7  1994/01/24 03:06:04  ahd
  *      Annual Copyright Update
  *
 * Revision 1.6  1994/01/01  19:23:09  ahd
 * Annual Copyright Update
 *
 * Revision 1.5  1993/12/02  13:50:40  ahd
 * 'e' protocol support
 *
 * Revision 1.4  1993/12/02  04:00:19  dmwatt
 * 'e' protocol support
 *
 * Revision 1.3  1993/04/05  12:27:31  ahd
 * Correct protypes to match gpkt
 *
 * Revision 1.2  1992/11/15  20:08:46  ahd
 * Clean up modem file support for different protocols
 *
  */

short  vopenpk(const KWBoolean master);
short  Gopenpk(const KWBoolean master);
short  gopenpk(const KWBoolean master);
short  gclosepk(void);
short  ggetpkt(char  *data,short  *len);
short  gsendpkt(char  *data,short  len);
short  gwrmsg(char *str);
short  grdmsg(register char *str);
short  geofpkt( void );
short  gfilepkt( const KWBoolean master, const unsigned long bytes );
