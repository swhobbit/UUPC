/*--------------------------------------------------------------------*/
/*    s u s p e n d . c                                               */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon (dummies for non-OS/2)      */
/*                                                                    */
/*    Author: Kai Uwe Rommel                                          */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) 1993 by Kai Uwe Rommel                         */
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
 *    $Id: suspend.c 1.6 1994/02/19 05:10:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: suspend.c $
 * Revision 1.6  1994/02/19  05:10:08  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/01/01  19:20:54  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1993/11/06  17:57:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.4  1993/11/06  17:57:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.3  1993/09/29  04:52:03  ahd
 * General clean up to be compatible with suspend2.c
 *
 * Revision 1.2  1993/09/27  02:42:11  ahd
 * Include header, use proper return values to match OS/2 functions
 *
 * Revision 1.1  1993/09/27  00:48:43  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "hostable.h"
#include "dcp.h"
#include "security.h"
#include "suspend.h"

boolean suspend_processing = FALSE;

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ i n i t                                      */
/*                                                                    */
/*       Initialize thread to handle port suspension                  */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

boolean suspend_init(const char *port )
{
   return FALSE;
}

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ o t h e r                                    */
/*                                                                    */
/*       Request another UUCICO give up a modem                       */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

int suspend_other(const boolean suspend,
                  const char *port )
{
   return 1;
}

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ w a i t                                      */
/*                                                                    */
/*       Wait to take the serial port back                            */
/*--------------------------------------------------------------------*/

CONN_STATE suspend_wait(void)
{
  return CONN_EXIT;
}
