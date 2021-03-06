#ifndef ULIBIP_H
#define ULIBIP_H

/*--------------------------------------------------------------------*/
/*    u l i b i p . h                                                 */
/*                                                                    */
/*    Native mode (serial port) communications functions              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) David M. Watt 1993, All Right Reserved            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibip.h 1.15 2001/03/12 13:51:16 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: ulibip.h $
 *    Revision 1.15  2001/03/12 13:51:16  ahd
 *    Annual copyright update
 *
 *    Revision 1.14  2000/05/12 12:41:43  ahd
 *    Annual copyright update
 *
 *    Revision 1.13  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.12  1998/03/01 01:43:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1997/06/03 03:26:38  ahd
 *    First compiling SMTP daemon
 *
 *    Revision 1.10  1997/04/24 01:38:19  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 21:24:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/07 16:50:32  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.7  1994/12/22 00:42:01  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/04/26 03:34:33  ahd
 *    hot (previously opened) handle support
 *
 * Revision 1.5  1994/01/24  02:58:06  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1994/01/01  19:24:40  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1993/12/30  02:56:28  dmwatt
 * Correct compile warnings
 *
 * Revision 1.2  1993/12/24  05:22:49  ahd
 * Use UUFAR pointer for comm buffers
 *
 * Revision 1.1  1993/09/20  04:53:57  ahd
 * Initial revision
 *
 */

extern int tactiveopenline(char *name, BPS baud, const KWBoolean direct);

extern int tpassiveopenline(char *name, BPS baud, const KWBoolean direct);

extern unsigned int tsread(char UUFAR *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int tswrite(const char UUFAR *data, unsigned int len);

void tssendbrk(unsigned int duration);

void tcloseline(void);

void tSIOSpeed(BPS baud);

void tflowcontrol( KWBoolean );

void thangup( void );

void tTerminateCommunications( void );

BPS tGetSpeed( void );

KWBoolean tCD( void );

KWBoolean tWaitForNetConnect(const unsigned int timeout);

int tGetComHandle( void );

void tSetComHandle( const int );

#endif
