/*--------------------------------------------------------------------*/
/*       e x e c u t e . h                                            */
/*                                                                    */
/*       Execute an external command for UUPC/extended functions      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: execute.h 1.11 2001/03/12 13:50:09 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: execute.h $
 *    Revision 1.11  2001/03/12 13:50:09  ahd
 *    Annual copyright update
 *
 *    Revision 1.10  2000/05/12 12:40:50  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.8  1998/03/01 01:26:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1997/05/11 04:28:53  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.6  1997/03/31 06:58:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1996/01/01 20:56:26  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1995/01/07 16:15:37  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.3  1994/12/22 00:12:34  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/01/01 19:08:10  ahd
 *    Annual Copyright Update
 *
 * Revision 1.1  1993/07/31  16:28:59  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                   BSD code for temporary failure                   */
/*--------------------------------------------------------------------*/

#define EX_TEMPFAIL     75      /* temp failure; user is invited to retry */

/*--------------------------------------------------------------------*/
/*       e x e c u t e                                                */
/*                                                                    */
/*       Generic execute external command with optional redirection   */
/*       of standard input and output                                 */
/*--------------------------------------------------------------------*/

int execute( const char *command,
             const char *parameters,
             const char *input,
             const char *output,
             const KWBoolean synchronous,
             const KWBoolean foreground );

int executeCommand( const char *command,
                    const char *input,
                    const char *output,
                    const KWBoolean synchronous,
                    const KWBoolean foreground );
