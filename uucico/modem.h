#ifndef _MODEM_H
#define _MODEM_H

/*--------------------------------------------------------------------*/
/*    m o d e m . h                                                   */
/*                                                                    */
/*    Prototypes for high level modem support routines                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*
 *    $Id: modem.h 1.6 1993/09/20 04:53:57 ahd Exp $
 *
 *    Revision history:
 *    $Log: modem.h $
 * Revision 1.6  1993/09/20  04:53:57  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 * Revision 1.5  1993/04/11  00:36:13  ahd
 * Global edits for year, TEXT, etc.
 *
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

void shutDown( void );

KEWSHORT GetGPacket( KEWSHORT maxvalue, const char protocol );

KEWSHORT GetGWindow( KEWSHORT maxvalue, const char protocol );

/*--------------------------------------------------------------------*/
/*                       Environment variables                        */
/*--------------------------------------------------------------------*/

extern boolean  bmodemflag[MODEM_LAST];
extern char     *device;
extern KEWSHORT M_fPacketSize;
extern KEWSHORT M_fPacketTimeout;
extern KEWSHORT M_gPacketTimeout;
extern KEWSHORT M_MaxErr;
extern KEWSHORT M_PortTimeout;
extern KEWSHORT M_startupTimeout;       /* pre-procotol exchanges        */
extern KEWSHORT M_tPacketTimeout;
extern KEWSHORT M_xfer_bufsize;
#endif
