/*
      dcpgpkt.h

      "g" protocol packet driver for dcp (UUPC/extended data
      communications)

      Change History:

      08 Sep 90   -  Create via Microsoft C compiler /Zg          ahd
 */

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

 /*
  *      $Id: dcpgpkt.h 1.2 1992/11/15 20:08:46 ahd Exp ahd $
  *
  *      $Log: dcpgpkt.h $
 * Revision 1.2  1992/11/15  20:08:46  ahd
 * Clean up modem file support for different protocols
 *
  */

short  vopenpk(const boolean master);
short  Gopenpk(const boolean master);
short  gopenpk(const boolean master);
short  gclosepk(void);
short  ggetpkt(char  *data,short  *len);
short  gsendpkt(char  *data,short  len);
short  gwrmsg(char *str);
short  grdmsg(register char *str);
short  geofpkt( void );
short  gfilepkt( void );
