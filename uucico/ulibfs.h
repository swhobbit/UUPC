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
 *    $Id: ULIBFS.H 1.1 1993/05/30 00:19:06 ahd Exp $
 *
 *    Revision history:
 *    $Log: ULIBFS.H $
 * Revision 1.1  1993/05/30  00:19:06  ahd
 * Initial revision
 *
 */

extern int fopenline(char *name, BPS baud, const boolean direct);

extern unsigned int fsread(char UUFAR *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int fswrite(const char UUFAR *data, unsigned int len);

void fssendbrk(unsigned int duration);

void fcloseline(void);

void fSIOSpeed(BPS baud);

void fflowcontrol( boolean );

void fhangup( void );

BPS fGetSpeed( void );

boolean fCD( void );
