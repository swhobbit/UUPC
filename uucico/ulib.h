#ifndef ULIB_H
#define ULIB_H

/*--------------------------------------------------------------------*/
/*    u l i b . h                                                     */
/*                                                                    */
/*    Native mode (serial port) communications functions              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
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
 * Revision 1.3  1993/05/30  00:11:03  ahd
 * Multiple communications drivers support
 *
 */

extern int nopenline(char *name, BPS baud, const boolean direct);

extern unsigned int nsread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int nswrite(const char *data, unsigned int len);

void nssendbrk(unsigned int duration);

void ncloseline(void);

void nSIOSpeed(BPS baud);

void nflowcontrol( boolean );

void nhangup( void );

BPS nGetSpeed( void );

boolean nCD( void );

#endif
