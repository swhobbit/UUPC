/*--------------------------------------------------------------------*/
/*    c a t c h e r . h                                               */
/*                                                                    */
/*    Ctrl-Break handler prototypes for UUPC/extended                 */
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
 *    $Id: lib.h 1.11 1993/08/08 17:39:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */


void
#ifdef __TURBOC__
__cdecl
#endif
ctrlchandler( int sig );

extern boolean terminate_processing;
extern boolean norecovery;
extern boolean interactive_processing;
extern int panic_rc;
