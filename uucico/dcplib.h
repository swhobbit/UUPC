/*--------------------------------------------------------------------*/
/* d c p l i b . h                                                    */
/*                                                                    */
/* System dependent (non-communications related) routines for         */
/* UUPC/extended.                                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.21 1994/04/24 20:17:29 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

boolean login(void);

boolean loginbypass(const char *user);
