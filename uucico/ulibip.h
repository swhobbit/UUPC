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
/*    Changes Copyright (c) 1990-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibip.h 1.3 1993/12/30 02:56:28 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: ulibip.h $
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

extern int tactiveopenline(char *name, BPS baud, const boolean direct);

extern int tpassiveopenline(char *name, BPS baud, const boolean direct);

extern unsigned int tsread(char UUFAR *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int tswrite(const char UUFAR *data, unsigned int len);

void tssendbrk(unsigned int duration);

void tcloseline(void);

void tSIOSpeed(BPS baud);

void tflowcontrol( boolean );

void thangup( void );

BPS tGetSpeed( void );

boolean tCD( void );

boolean tWaitForNetConnect(const unsigned int timeout);

#endif
