/*--------------------------------------------------------------------*/
/*    s a f e i o . h                                                 */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
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
 *    $Id: safeio.h 1.4 1993/07/20 21:41:51 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: safeio.h $
 *     Revision 1.4  1993/07/20  21:41:51  dmwatt
 *     Don't rely on standard I/O under Windows/NT
 *
 */

#include "uutypes.h"        /* Include UUPC/extended types           */

int  safein( void );
boolean safepeek( void );
void safeout( char *str );
void safeflush( void );

#if defined(WIN32)
void InitConsoleInputHandle(void);
void InitConsoleOutputHandle(void);
#endif

