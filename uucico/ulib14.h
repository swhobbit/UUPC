/*--------------------------------------------------------------------*/
/*    u l i b 1 4 . h                                                 */
/*                                                                    */
/*    INT14 (serial port) communications functions                    */
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
 *    $Id: ULIBFS.H 1.1 1993/05/30 00:19:06 ahd Exp $
 *
 *    Revision history:
 *    $Log: ULIBFS.H $
 * Revision 1.1  1993/05/30  00:19:06  ahd
 * Initial revision
 *
 */

extern int iopenline(char *name, BPS baud, const boolean direct);

extern unsigned int isread(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int iswrite(const char *data, unsigned int len);

void issendbrk(unsigned int duration);

void icloseline(void);

void iSIOSpeed(BPS baud);

void iflowcontrol( boolean );

void ihangup( void );

BPS iGetSpeed( void );

boolean iCD( void );
