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
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ULIB.H 1.3 1993/05/30 00:11:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: ULIB.H $
 */

extern int tactiveopenline(char *name, BPS baud, const boolean direct);

extern int tpassiveopenline(char *name, BPS baud, const boolean direct);

extern unsigned int tsread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int tswrite(const char *data, unsigned int len);

void tssendbrk(unsigned int duration);

void tcloseline(void);

void tSIOSpeed(BPS baud);

void tflowcontrol( boolean );

void thangup( void );

BPS tGetSpeed( void );

boolean tCD( void );

boolean tWaitForNetConnect(int timeout);

#endif
