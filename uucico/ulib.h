#ifndef ULIB_H
#define ULIB_H

/*--------------------------------------------------------------------*/
/*    u l i b . h                                                     */
/*                                                                    */
/*    Native mode (serial port) communications functions              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1996 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulib.h 1.10 1995/01/07 16:49:42 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: ulib.h $
 *    Revision 1.10  1995/01/07 16:49:42  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.9  1994/12/22 00:41:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/04/26 03:34:33  ahd
 *    hot (previously opened) handle support
 *
 * Revision 1.7  1994/01/01  19:24:19  ahd
 * Annual Copyright Update
 *
 * Revision 1.6  1993/12/24  05:22:49  ahd
 * Use UUFAR pointer for comm buffers
 *
 * Revision 1.5  1993/11/20  14:50:25  ahd
 * Add prototype for GetComHandle
 *
 * Revision 1.4  1993/09/20  04:53:57  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 * Revision 1.3  1993/05/30  00:11:03  ahd
 * Multiple communications drivers support
 *
 */

extern int nopenline(char *name, BPS baud, const KWBoolean direct);

extern unsigned int nsread(char UUFAR *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int nswrite(const char UUFAR *data, unsigned int len);

void nssendbrk(unsigned int duration);

void ncloseline(void);

void nSIOSpeed(BPS baud);

void nflowcontrol( KWBoolean );

void nhangup( void );

BPS nGetSpeed( void );

KWBoolean nCD( void );

#if defined(BIT32ENV) || defined(FAMILYAPI)

int nGetComHandle( void );

void nSetComHandle( const int );

#endif

#endif
