/*
      dcpgpkt.h

      "g" protocol packet driver for dcp (UUPC/extended data
      communications)

      Change History:

      08 Sep 90   -  Create via Microsoft C compiler /Zg          ahd
 */

 /*
  *      $Id$
  *
  *      $Log$
  */

int  vopenpk(const boolean master);
int  Gopenpk(const boolean master);
int  gopenpk(const boolean master);
int  gclosepk(void);
int  ggetpkt(char  *data,int  *len);
int  gsendpkt(char  *data,int  len);
int  gwrmsg(char *str);
int  grdmsg(register char *str);
int  geofpkt( void );
int  gfilepkt( void );
