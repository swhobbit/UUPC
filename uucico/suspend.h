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
 *    $Id: suspend.h 1.12 1999/01/04 03:55:33 ahd Exp $
 *
 *    Revision history:
 *    $Log: suspend.h $
 *    Revision 1.12  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.11  1998/03/01 01:42:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.10  1997/04/24 01:37:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1996/01/01 21:24:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1995/01/07 16:49:19  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.7  1994/12/22 00:41:07  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1994/10/24 23:41:49  rommel
 *    Prevent suspend processing hanging from rapid closing and opening of
 *    the same file.
 *
 *        Revision 1.5  1994/05/01  21:59:06  dmwatt
 *        Trap errors from failure of suspend_init to create pipe
 *
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
extern KWBoolean suspend_processing;

KWBoolean suspend_init(const char *port );

int suspend_other(const KWBoolean suspend,
                  const char *port );

CONN_STATE suspend_wait(void);
void suspend_ready(void);
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
