#ifndef ULIB_H
#define ULIB_H

/*--------------------------------------------------------------------*/
/*    u l i b . h                                                     */
/*                                                                    */
/*    Native mode (serial port) communications functions              */
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
 *    $Id: lib.h 1.5 1993/04/04 21:51:00 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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
