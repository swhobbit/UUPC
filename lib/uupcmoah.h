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
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uupcmoah.h 1.7 1997/03/31 07:07:48 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: uupcmoah.h $
 *    Revision 1.7  1997/03/31 07:07:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1996/11/19 00:25:20  ahd
 *    Correct C++ scoping
 *
 *    Revision 1.5  1996/03/18 03:48:14  ahd
 *    Allow compilation under C++ compilers
 *
 *    Revision 1.4  1996/01/01 20:59:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1995/01/05 03:45:13  ahd
 *    Don't allow Windows compiles under MSC, it doesn't work
 *
 *    Revision 1.2  1994/12/22 00:14:32  ahd
 *    Annual Copyright Update
 *
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
#else

#if defined(WINDOWS)          /* Real MSC doesn't work for Windows     */
#error This only compiles with the Borland C++ 3.1 compiler and EasyWin, sorry!
#endif

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
