/*--------------------------------------------------------------------*/
/*    m o d e m . h                                                   */
/*                                                                    */
/*    Prototypes for high level modem support routines                */
/*--------------------------------------------------------------------*/

/*
 *    $Header: E:\src\uupc\uucico\RCS\MODEM.C 1.2 1992/11/15 20:12:17 ahd Exp $
 *
 *    Revision history:
 *    $Log: MODEM.C $
 */

/*--------------------------------------------------------------------*/
/*                           Defined types                            */
/*--------------------------------------------------------------------*/

typedef enum {
   MODEM_FIXEDSPEED,
   MODEM_VARIABLEPACKET,
   MODEM_LARGEPACKET,
   MODEM_DIRECT,
   MODEM_CD,
   MODEM_LAST
   } MODEM_FLAGS;

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

CONN_STATE callup(void );

CONN_STATE callin( time_t exit_time );

CONN_STATE callhot( const BPS speed );

void slowwrite( char *s, int len);

void shutdown( void );

INTEGER  GetGPacket( INTEGER maxvalue, const char protocol );

INTEGER  GetGWindow( INTEGER maxvalue, const char protocol );


/*--------------------------------------------------------------------*/
/*                       Environment variables                        */
/*--------------------------------------------------------------------*/

extern INTEGER M_gPacketTimeout;
extern INTEGER M_fPacketTimeout;
extern INTEGER M_fPacketSize;
extern INTEGER M_PortTimeout;
extern INTEGER M_MaxErr;
extern INTEGER M_xfer_bufsize;
extern boolean bmodemflag[MODEM_LAST];
extern char *device;
