#ifdef __UUPCMOAH
#error Program already included the Mother of All Headers!
#else
#define __UUPCMOAH

/*--------------------------------------------------------------------*/
/*       u u p c m o a h . h                                          */
/*                                                                    */
/*       UUPC/extended Mother of All Headers.  This file should       */
/*       be included FIRST in all UUPC/extended programs.             */
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
 *    $Id: uupcmoah.h 1.1 1994/02/20 19:16:21 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: uupcmoah.h $
 *    Revision 1.1  1994/02/20 19:16:21  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 */

#if defined(__IBMC__)
#pragma strings(readonly)       /* Don't clobber any literal string data */
#endif

#if _MSC_VER == 700
#pragma function(strcpy)        /* Get around a MS C7 code-generation bug:
                                   The C 7.00 compiler uses the wrong register
                                   for passing the return value of strcpy to
                                   another function!  The bug appears to be
                                   only in C7; Visual C fixed the problem. */
#endif /* _MSC_VER == 700 */


#ifdef __TURBOC__
#define __MSC                 /* Make Borland C++ 2.0 act like MS C   */
#endif

#if defined(WIN32) || defined(__OS2__) || defined(__32BIT__)
#define BIT32ENV
#endif

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

/*--------------------------------------------------------------------*/
/*                Standard UUPC/extended include files                */
/*--------------------------------------------------------------------*/

#include "uutypes.h"
#include "confvars.h"
#include "lib.h"
#include "hlib.h"

#endif /* __UUPCMOAH */
