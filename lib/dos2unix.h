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
 *    $Id: lib.h 1.4 1993/03/06 23:09:50 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

#include "uundir.h"

time_t dos2unix( const FDATE ddmmyy,
                 const FTIME ssmmhh );

#endif
