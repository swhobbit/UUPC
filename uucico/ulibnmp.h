#ifndef ULIBNMP_H
#define ULIBNMP_H

/*--------------------------------------------------------------------*/
/*    u l i b n m p . h                                               */
/*                                                                    */
/*    OS/2 named pipes support                                        */
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
 *    $Id: ulibnmp.h 1.4 1994/01/24 02:58:14 ahd Exp $
 *
 *    Revision history:
 *    $Log: ulibnmp.h $
 * Revision 1.4  1994/01/24  02:58:14  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1994/01/01  19:24:45  ahd
 * Annual Copyright Update
 *
 * Revision 1.2  1993/12/24  05:22:49  ahd
 * Use UUFAR pointer for comm buffers
 *
 * Revision 1.1  1993/09/24  03:46:07  ahd
 * Initial revision
 *
 *
 */

int pactiveopenline(char *name, BPS baud, const boolean direct );

int ppassiveopenline(char *name, BPS baud, const boolean direct );

unsigned int psread(char UUFAR *output, unsigned int wanted, unsigned int timeout);

int pswrite(const char UUFAR *input, unsigned int len);

void pssendbrk(unsigned int duration);

void pcloseline(void);

void phangup( void );

void pSIOSpeed(BPS baud);

void pflowcontrol( boolean flow );

BPS pGetSpeed( void );

boolean pCD( void );

boolean pWaitForNetConnect(const unsigned int timeout);

int pGetComHandle( void );

void pSetComHandle( const int );

#endif
