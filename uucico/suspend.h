#ifndef SUSPEND_H
#define SUSPEND_H

/*--------------------------------------------------------------------*/
/*    s u s p e n d . h                                               */
/*                                                                    */
/*    suspend/resume uupoll/uucico daemon                             */
/*                                                                    */
/*    Author: Kai Uwe Rommel                                          */
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
 *    $Id: suspend2.c 1.1 1993/09/27 00:45:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: suspend2.c $
 */

#ifndef NO_SUSPEND_FUNCTIONS
extern boolean suspend_processing;

void suspend_init(const char *port );

int suspend_other(const boolean suspend,
                  const char *port );

CONN_STATE suspend_wait(void);
#endif

#define SUSPEND_PIPE "\\PIPE\\UUCICO\\ZZ"

#endif
