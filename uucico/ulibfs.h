/*--------------------------------------------------------------------*/
/*    u l i b f s . h                                                 */
/*                                                                    */
/*    FOSSIL (serial port) communications functions                   */
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
 *    $Id: ulibfs.h 1.6 1995/01/07 16:50:25 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: ulibfs.h $
 *    Revision 1.6  1995/01/07 16:50:25  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.5  1994/12/22 00:41:45  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/01/24 02:57:57  ahd
 *    Annual Copyright Update
 *
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

extern int fopenline(char *name, BPS baud, const KWBoolean direct);

extern unsigned int fsread(char UUFAR *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int fswrite(const char UUFAR *data, unsigned int len);

void fssendbrk(unsigned int duration);

void fcloseline(void);

void fSIOSpeed(BPS baud);

void fflowcontrol( KWBoolean );

void fhangup( void );

BPS fGetSpeed( void );

KWBoolean fCD( void );
