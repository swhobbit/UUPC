/*--------------------------------------------------------------------*/
/*       e x e c u t e . h                                            */
/*                                                                    */
/*       Execute an external command for UUPC/extended functions      */
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
 *    $Id: execute.h 1.2 1994/01/01 19:08:10 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: execute.h $
 *    Revision 1.2  1994/01/01 19:08:10  ahd
 *    Annual Copyright Update
 *
 * Revision 1.1  1993/07/31  16:28:59  ahd
 * Initial revision
 *
 */

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
             const boolean synchronous,
             const boolean foreground );

int executeCommand( const char *command,
                    const char *input,
                    const char *output,
                    const boolean synchronous,
                    const boolean foreground );
