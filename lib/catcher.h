/*--------------------------------------------------------------------*/
/*    c a t c h e r . h                                               */
/*                                                                    */
/*    Ctrl-Break handler prototypes for UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: catcher.h 1.6 1996/01/01 20:56:08 ahd v1-12r $
 *
 *    Revision history:
 *    $Log: catcher.h $
 *    Revision 1.6  1996/01/01 20:56:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1995/01/07 16:15:29  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.4  1994/12/22 00:12:18  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/01/01 19:07:28  ahd
 *    Annual Copyright Update
 *
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

extern KWBoolean terminate_processing;
extern KWBoolean norecovery;
extern KWBoolean interactive_processing;
extern int panic_rc;
