/*--------------------------------------------------------------------*/
/*       d c p l i b . h                                              */
/*                                                                    */
/*       System dependent (non-communications related) routines       */
/*       for UUPC/extended.                                           */
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
 *    $Id: dcplib.h 1.3 1994/05/24 03:45:09 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: dcplib.h $
 *    Revision 1.3  1994/05/24 03:45:09  ahd
 *    Clean up header comments
 *
 *        Revision 1.2  1994/05/06  03:58:32  ahd
 *        Add copyright
 *
 */

boolean login(void);

boolean loginbypass(const char *user);
