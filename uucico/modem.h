/*--------------------------------------------------------------------*/
/*    m o d e m . h                                                   */
/*                                                                    */
/*    Prototypes for high level modem support routines                */
/*--------------------------------------------------------------------*/

/*
 *    $Id: E:\src\uupc\UUCICO\RCS\MODEM.H 1.4 1992/11/28 19:53:22 ahd Exp $
 *
 *    Revision history:
 *    $Log: MODEM.H $
 * Revision 1.4  1992/11/28  19:53:22  ahd
 * Make callin time parameter const
 *
 * Revision 1.3  1992/11/18  03:50:17  ahd
 * Move check of call window to avoid premature lock file overhead
 *
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

CONN_STATE callin( const time_t exit_time );

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
