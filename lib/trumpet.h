/*--------------------------------------------------------------------*/
/*       t r u m p e t . h                                            */
/*                                                                    */
/*       Audio support for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: trumpet.h 1.8 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: trumpet.h $
 *    Revision 1.8  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.7  1998/03/16 06:15:28  ahd
 *    Control reporting of multiple announcements
 *
 *    Revision 1.6  1998/03/01 01:27:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1997/03/31 07:07:28  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1996/01/01 20:58:22  ahd
 *    Annual Copyright Update
 *
 */

#ifndef _TRUMPET_H
#define _TRUMPET_H
void trumpet( const char *tune, KWBoolean daemon, time_t last );
#endif
