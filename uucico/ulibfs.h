/*--------------------------------------------------------------------*/
/*    u l i b f s . h                                                 */
/*                                                                    */
/*    FOSSIL (serial port) communications functions                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibfs.h 1.3 1994/01/01 19:24:33 ahd Exp $
 *
 *    Revision history:
 *    $Log: ulibfs.h $
 * Revision 1.3  1994/01/01  19:24:33  ahd
 * Annual Copyright Update
 *
 * Revision 1.2  1993/12/24  05:22:49  ahd
 * Use UUFAR pointer for comm buffers
 *
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
