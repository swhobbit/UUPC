/*--------------------------------------------------------------------*/
/*    c a t c h e r . h                                               */
/*                                                                    */
/*    Ctrl-Break handler prototypes for UUPC/extended                 */
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
 *    $Id: catcher.h 1.2 1993/09/20 04:51:31 ahd Exp $
 *
 *    Revision history:
 *    $Log: catcher.h $
 *     Revision 1.2  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 *     Revision 1.2  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
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
