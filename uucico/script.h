/*--------------------------------------------------------------------*/
/*    s c r i p t . h                                                 */
/*                                                                    */
/*    Script processing routines used by UUPC/extended                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: script.h 1.4 1994/01/01 19:24:07 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: script.h $
 *    Revision 1.4  1994/01/01 19:24:07  ahd
 *    Annual Copyright Update
 *
 * Revision 1.3  1993/12/24  05:26:43  ahd
 * Add RCS header
 *
 */

int expectstr(char *Search, unsigned int Timeout, char **failure);

boolean sendstr(char  *str, unsigned int timeout, char **failure);

void echoCheck( const unsigned int timeout );
