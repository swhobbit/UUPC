#ifndef DOS2UNIX_H
#define DOS2UNIX_H

/*--------------------------------------------------------------------*/
/*      d o s 2 u n i x . h                                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: DOS2UNIX.H 1.1 1993/04/05 04:38:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: DOS2UNIX.H $
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
