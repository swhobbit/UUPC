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
 *    $Id: suspend.h 1.4 1994/01/01 19:24:13 ahd Exp $
 *
 *    Revision history:
 *    $Log: suspend.h $
 * Revision 1.4  1994/01/01  19:24:13  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1993/12/26  16:28:56  ahd
 * Use enumerated types for command verbs and responses
 *
 * Revision 1.2  1993/09/29  04:56:11  ahd
 * Suspend port by port name, not modem file name
 *
 */

#ifndef NO_SUSPEND_FUNCTIONS
extern boolean suspend_processing;

boolean suspend_init(const char *port );

int suspend_other(const boolean suspend,
                  const char *port );

CONN_STATE suspend_wait(void);
#endif

#define SUSPEND_PIPE "\\PIPE\\UUCICO\\ZZ"

#ifdef WIN32
#define SUSPEND_LOCAL "\\\\." /* Windows NT requires explicit local node   */
#else
#define SUSPEND_LOCAL ""      /* But OS/2 doesn't.                         */
#endif

typedef enum suspendCommands
   {
      /* Commands */
      SUSPEND_EXIT    = 'E',        /* Terminate UUCICO completely   */
      SUSPEND_QUERY   = 'Q',        /* Request UUCICO report status  */
      SUSPEND_RESUME  = 'R',        /* Resume use of port now        */
      SUSPEND_SLEEP   = 'S',        /* Surrender port now            */

      /* Responses */
      SUSPEND_ACTIVE  = 'A',        /* UUCICO currently running      */
      SUSPEND_BUSY    = 'B',        /* Busy, cannot suspend          */
      SUSPEND_ERROR   = 'E',        /* System error, suspend failed  */
      SUSPEND_OKAY    = 'O',        /* UUCICO has honored command    */
      SUSPEND_WAITING = 'W'         /* UUCICO currently suspended    */
   }; /* suspendCommands */
#endif
