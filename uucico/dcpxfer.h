#ifndef _DCPXFER
#define _DCPXFER

/*
      dcpxfer.h

      File transfer functions for dcp (UUPC/extended data
      communications)

      Change History:

      08 Sep 90   -  Create via Microsoft C compiler /Zg          ahd
*/

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

XFER_STATE sdata( void );
XFER_STATE sbreak( void );
XFER_STATE seof( const boolean purge_file );
XFER_STATE newrequest( void );
XFER_STATE ssfile( void );
XFER_STATE srfile( void );
XFER_STATE sinit( void );
XFER_STATE schkdir( const boolean outbound, const char callgrade );
XFER_STATE endp( void );
XFER_STATE rdata( void );
XFER_STATE rheader( void );
XFER_STATE rrfile( void );
XFER_STATE rsfile( void );
XFER_STATE rinit( void );
XFER_STATE reof( void );

#endif
