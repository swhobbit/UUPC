/*--------------------------------------------------------------------*/
/*    u l i b f s . h                                                 */
/*                                                                    */
/*    FOSSIL (serial port) communications functions                   */
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

extern int fopenline(char *name, BPS baud, const boolean direct);

extern unsigned int fsread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int fswrite(const char *data, unsigned int len);

void fssendbrk(unsigned int duration);

void fcloseline(void);

void fSIOSpeed(BPS baud);

void fflowcontrol( boolean );

void fhangup( void );

BPS fGetSpeed( void );

boolean fCD( void );
