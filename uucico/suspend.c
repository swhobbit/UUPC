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
 *    $Id: suspend.c 1.1 1993/09/27 00:48:43 ahd Exp $
 *
 *    Revision history:
 *    $Log: suspend.c $
 * Revision 1.1  1993/09/27  00:48:43  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/types.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
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

void suspend_init(void)
{
}

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ o t h e r                                    */
/*                                                                    */
/*       Request another UUCICO give up a modem                       */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int suspend_other(boolean suspend)
{
   return 1;
}

/*--------------------------------------------------------------------*/
/*       s u s p e n d _ w a i t                                      */
/*                                                                    */
/*       Wait to take the serial port back                            */
/*--------------------------------------------------------------------*/

CONN_STATE suspend_wait(void)
{
  return CONN_EXIT;
}
