/*--------------------------------------------------------------------*/
/*    s c r i p t . h                                                 */
/*                                                                    */
/*    Script processing routines used by UUPC/extended                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.17 1993/12/24 05:22:49 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

int expectstr(char *Search, unsigned int Timeout, char **failure);

boolean sendstr(char  *str, unsigned int timeout, char **failure);

void echoCheck( const unsigned int timeout );
