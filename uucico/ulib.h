#ifndef ULIB_H
#define ULIB_H

/*--------------------------------------------------------------------*/
/*    u l i b . h                                                     */
/*                                                                    */
/*    Native mode (serial port) communications functions              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: ulib.h 1.14 1998/04/20 02:48:54 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: ulib.h $
 *    Revision 1.14  1998/04/20 02:48:54  ahd
 *    Windows 32 bit GUI environment/TAPI support
 *
 *    Revision 1.13  1998/03/01 01:42:56  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1997/04/24 01:38:02  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1996/01/01 21:24:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1995/01/07 16:49:42  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.9  1994/12/22 00:41:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1994/04/26 03:34:33  ahd
 *    hot (previously opened) handle support
 *
 * Revision 1.7  1994/01/01  19:24:19  ahd
 * Annual Copyright Update
 *
 */

extern int nopenline(char *name, BPS baud, const KWBoolean direct);

extern unsigned int nsread(char UUFAR *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

int nswrite(const char UUFAR *data, unsigned int len);

void nssendbrk(unsigned int duration);

void ncloseline(void);

#ifdef TAPI_SUPPORT
/* Hack for unique TAPI close function */
void mcloseline(void);
#endif

void nSIOSpeed(BPS baud);

void nflowcontrol( KWBoolean );

void nhangup( void );

BPS nGetSpeed( void );

KWBoolean nCD( void );

#if defined(BIT32ENV) || defined(FAMILYAPI)

int nGetComHandle( void );

void nSetComHandle( const int );

#endif

#endif
