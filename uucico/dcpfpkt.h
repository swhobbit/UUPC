/*
      dcpfpkt.h

      "f" protocol packet driver for dcp (UUPC/extended data
      communications)

      Change History:

      08 Sep 90   -  Create via Microsoft C compiler /Zg          ahd
      21 Aug 91   -  Create from dcpgpkt.c                        ahd
 */

 /*
  *      $Id$
  *
  *      $Log$
  */

int  fopenpk(const boolean master);
int  fclosepk(void);
int  fgetpkt(char  *data,int  *len);
int  fsendpkt(char  *data,int  len);
int  fwrmsg(char *str);
int  frdmsg(char *str);
int  feofpkt( void );
int  ffilepkt( void );
