/*--------------------------------------------------------------------*/
/*    s a f e i o . h                                                 */
/*                                                                    */
/*    Console I/O functions for use during interrupt processing       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.9 1993/07/19 02:53:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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

