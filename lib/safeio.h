/*--------------------------------------------------------------------*/
/*    s a f e i o . h                                                 */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: safeio.h 1.12 2000/05/12 12:40:50 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: safeio.h $
 *    Revision 1.12  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.11  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.10  1998/03/01 01:27:28  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/03/31 07:06:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 20:58:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/01/07 16:16:23  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.6  1994/12/22 00:13:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/01/01 19:09:34  ahd
 *    Annual Copyright Update
 *
 *     Revision 1.4  1993/07/20  21:41:51  dmwatt
 *     Don't rely on standard I/O under Windows/NT
 *
 */

#include "uutypes.h"        /* Include UUPC/extended types           */

int  safein( void );
KWBoolean safepeek( void );
void safeout( char *str );
void safeflush( void );

#if defined(WIN32)
void InitConsoleInputHandle(void);
void InitConsoleOutputHandle(void);
#endif

