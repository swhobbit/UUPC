/*
      dcpfpkt.h

      "f" protocol packet driver for dcp (UUPC/extended data
      communications)

      Change History:

      08 Sep 90   -  Create via Microsoft C compiler /Zg          ahd
      21 Aug 91   -  Create from dcpgpkt.c                        ahd
 */

 /*
  *      $Id: DCPFPKT.H 1.2 1992/11/15 20:08:29 ahd Exp $
  *
  *      $Log: DCPFPKT.H $
 * Revision 1.2  1992/11/15  20:08:29  ahd
 * Clean up modem file support for different procotols
 *
  */

short  fopenpk(const boolean master);
short  fclosepk(void);
short  fgetpkt(char  *data,short  *len);
short  fsendpkt(char  *data,short  len);
short  fwrmsg(char *str);
short  frdmsg(char *str);
short  feofpkt( void );
short  ffilepkt( void );
