/*--------------------------------------------------------------------*/
/*       e x e c u t e . C                                            */
/*                                                                    */
/*       Execute an external command for UUPC/extended functions      */
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
 *    $Id: lib.h 1.10 1993/07/22 23:26:19 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <process.h>

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "execute.h"

#ifdef _Windows
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

#ifndef _Windows
currentfile();
#endif

static char *cmdline = NULL;

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static boolean internal( const char *command );

#ifdef _Windows

/*--------------------------------------------------------------------*/
/*       e x e c u t e                                                */
/*                                                                    */
/*       execute external command under Windows                       */
/*--------------------------------------------------------------------*/

int execute( const char *command,
             const char *parameters,
             const char *input,
             const char *output,
             const boolean synchronous,
             const boolean foreground )

{
   int result;

   printmsg(2, "execute: command %s %s",
               command,
               parameters == NULL ? "" : parameters );

   if ( parameters == NULL )
      result = SpawnWait( command ,
                          foreground ? SW_MAXIMIZE : SW_SHOWMINNOACTIVE );
   else if ( cmdline != NULL )
      result = SpawnWait( cmdline ,
                          foreground ? SW_MAXIMIZE : SW_SHOWMINNOACTIVE );
   else {
      char wincmd[BUFSIZ];

      strcpy( wincmd, command );
      strcat( wincmd, " " );
      strcat( wincmd, parameters );

      result = SpawnWait( wincmd ,
                          foreground ? SW_MAXIMIZE : SW_SHOWMINNOACTIVE );
   } /* else */

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( (result == 0 ) ? 8 : 1,"Result of spawn %s is ... %d",
                                 command, result);

   return result;

} /* execute */

#else

/*--------------------------------------------------------------------*/
/*       e x e c u t e                                                */
/*                                                                    */
/*       Generic execute external command with optional redirection   */
/*       of standard input and output                                 */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

int execute( const char *command,
             const char *parameters,
             const char *input,
             const char *output,
             const boolean synchronous,
             const boolean foreground )
{
   int result;

   printmsg(2, "execute: command %s %s",
               command,
               parameters == NULL ? "" : parameters );

/*--------------------------------------------------------------------*/
/*               Redirect STDIN and STDOUT as required                */
/*--------------------------------------------------------------------*/

   if ((input != NULL) && (freopen(input , "rb", stdin) == NULL))
   {
      printerr(input);
      return -2;
   }

   if ((output != NULL) && (freopen(output, "wt", stdout) == NULL))
   {
      printerr( output );
      return -2;
   }

/*--------------------------------------------------------------------*/
/*                  Execute the command in question                   */
/*--------------------------------------------------------------------*/

   if (internal(command))        /* Internal command?                */
   {

      if ( parameters == NULL )
         result = system( command );
      else {
         char buf[BUFSIZ];

         strcpy( buf, command );
         strcat( buf, " ");
         strcpy( buf, parameters );

         result = system( buf );
      } /* else */

   } /* if (internal(command)) */
   else  {                       /* No --> Invoke normally           */

      result = spawnlp( P_WAIT,
                        (char *) command,
                        (char *) parameters,
                        NULL);

      if (result == -1)       /* Did spawn fail?                     */
         printerr(command);   /* Yes --> Report error                */

   } /* else */

/*--------------------------------------------------------------------*/
/*                  Re-open our standard i/o streams                  */
/*--------------------------------------------------------------------*/

   errno = 0;

   if ( output != NULL )
      freopen("con", "wt", stdout);

   errno = 0;

   if ( input != NULL )
   {
      FILE *temp = freopen("con", "rt", stdin);

      if ( (temp == NULL) && (errno != 0) )
      {
         printerr("stdin");
         panic();
      }

   } /* if ( input != NULL ) */

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( (result == 0 ) ? 4 : 1,"Result of spawn %s is ... %d",
                                 command, result);

   return result;

} /* execute */

#endif

/*--------------------------------------------------------------------*/
/*       e x e c u t e C o m m a n d                                  */
/*                                                                    */
/*       Split command from its parameters for execute                */
/*--------------------------------------------------------------------*/

int executeCommand( const char *command,
                    const char *input,
                    const char *output,
                    const boolean synchronous,
                    const boolean foreground )
{
   char *cmdname;
   char *parameters;
   char buffer[BUFSIZ];
   int result;

   strcpy( buffer, command );
   cmdline = (const char *) command;   // Save full command line for Execute

   cmdname = strtok( buffer, WHITESPACE );
   parameters = strtok( NULL, "\r\n" );

   if ( parameters != NULL )
   {
      while (isspace( *parameters ) || iscntrl( *parameters ))
         parameters++;

      if ( !strlen( parameters ))
         parameters = NULL;
   }

   result = execute( cmdname,
                     parameters,
                     input,
                     output,
                     synchronous,
                     foreground );

   cmdline = NULL;               // Flag cmdline no longer available

   return result;

} /* executeCommand */

/*--------------------------------------------------------------------*/
/*    i n t e r n a l                                                 */
/*                                                                    */
/*    Determine if command is internal DOS command                    */
/*--------------------------------------------------------------------*/

static boolean internal( const char *command )
{
   static char *commands[] = { "break",   "cd",    "chdir",    "copy",
                               "ctty",    "date",  "del",      "dir",
                               "echo",    "erase", "for",      "md",
                               "mkdir",   "rd",    "rem",      "ren",
                               "rename",  "rmdir", "time",     "ver",
                               "verify",  "vol",
                               NULL };
   char **list;

/*--------------------------------------------------------------------*/
/*                   Determine command list to use                    */
/*--------------------------------------------------------------------*/

   if (E_internal == NULL )
      list = commands;
   else
      list = E_internal;

/*--------------------------------------------------------------------*/
/*                   Scan the list for the command                    */
/*--------------------------------------------------------------------*/

   while( *list != NULL )
   {
      printmsg(5,"Searching for \"%s\", comparing to \"%s\"",
                  *list, command);

      if (equali(*list++,command))
      {
         printmsg(4,"\"%s\" is an internal command",command);
         return TRUE;
      } /* if */

   } /* while( *list != NULL ) */

/*--------------------------------------------------------------------*/
/*       The command is not in the list; return FALSE (external       */
/*       command)                                                     */
/*--------------------------------------------------------------------*/

   printmsg(4,"\"%s\" is an external command",command);
   return FALSE;

} /* internal */
