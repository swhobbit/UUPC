/*--------------------------------------------------------------------*/
/*       d c p l i b . h                                              */
/*                                                                    */
/*       System dependent (non-communications related) routines       */
/*       for UUPC/extended.                                           */
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
 *    $Id: dcplib.h 1.2 1994/05/06 03:58:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: dcplib.h $
 *        Revision 1.2  1994/05/06  03:58:32  ahd
 *        Add copyright
 *
 */

boolean login(void);

boolean loginbypass(const char *user);
