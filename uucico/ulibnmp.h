#ifndef ULIBNMP_H
#define ULIBNMP_H

/*--------------------------------------------------------------------*/
/*    u l i b n m p . h                                               */
/*                                                                    */
/*    OS/2 named pipes support                                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulibnmp.h 1.9 1997/04/24 01:38:24 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: ulibnmp.h $
 *    Revision 1.9  1997/04/24 01:38:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 21:24:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/01/07 16:50:38  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.6  1994/12/22 00:42:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/04/26 03:34:33  ahd
 *    hot (previously opened) handle support
 *
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

int pactiveopenline(char *name, BPS baud, const KWBoolean direct );

int ppassiveopenline(char *name, BPS baud, const KWBoolean direct );

unsigned int psread(char UUFAR *output, unsigned int wanted, unsigned int timeout);

int pswrite(const char UUFAR *input, unsigned int len);

void pssendbrk(unsigned int duration);

void pcloseline(void);

void phangup( void );

void pSIOSpeed(BPS baud);

void pflowcontrol( KWBoolean flow );

BPS pGetSpeed( void );

KWBoolean pCD( void );

KWBoolean pWaitForNetConnect(const unsigned int timeout);

int pGetComHandle( void );

void pSetComHandle( const int );

#endif
