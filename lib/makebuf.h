#ifndef _MAKEBUF_H
#define _MAKEBUF_H
/*--------------------------------------------------------------------*/
/*       m a k e b u f . h                                            */
/*                                                                    */
/*       heap memory poll management routines; buffers are            */
/*       allocated off a LIFO queue; freeing of a buffer causes       */
/*       all buffers allocated it after to also be freed              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.36 1996/03/18 03:48:14 ahd Exp $
 *
 *    $Log$
 */


void *makeBuf( const size_t length, const char *file, const size_t line);

void
freeBuf( void *oldBuffer, const char *file, const size_t line );

#define MAKEBUF( l ) makeBuf( l, cfnptr, __LINE__ )

#define FREEBUF( p ) freeBuf( p, cfnptr, __LINE__ )

#endif  /* _MAKEBUF_H */
