#ifndef DOS2UNIX_H
#define DOS2UNIX_H

/*--------------------------------------------------------------------*/
/*      d o s 2 u n i x . h                                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dos2unix.h 1.8 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: dos2unix.h $
 *    Revision 1.8  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.7  1998/03/01 01:26:19  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1997/03/31 06:58:47  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1996/01/01 20:56:19  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1994/12/22 00:12:30  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1994/01/01 19:07:58  ahd
 *    Annual Copyright Update
 *
 * Revision 1.2  1993/04/13  02:27:59  dmwatt
 * Windows/NT updates
 *
 * Revision 1.1  1993/04/05  04:38:55  ahd
 * Initial revision
 *
 */

#include "uundir.h"

time_t dos2unix( const FDATE ddmmyy,
                 const FTIME ssmmhh );

#ifdef WIN32
time_t nt2unix( FILETIME *nsec );
#endif

#endif
