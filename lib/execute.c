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
 *    $Id: execute.c 1.3 1993/08/03 03:11:49 ahd Exp $
 *
 *    Revision history:
 *    $Log: execute.c $
 * Revision 1.3  1993/08/03  03:11:49  ahd
 * Further Windows 3.x fixes
 *
 * Revision 1.2  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.1  1993/07/31  16:22:16  ahd
 * Initial revision
 *
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

#ifdef WIN32
#include <windows.h>
#include <signal.h>
#endif

#ifdef _Windows
#include <windows.h>
#include <shellapi.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "execute.h"

#ifdef _Windows
#include "winutil.h"
#include "dir.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

currentfile();

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
   static char *myDirectory = NULL;

   boolean usebat = (input != NULL) || (output != NULL );

   char batch[FILENAME_MAX];
   char perfect[FILENAME_MAX];
   char lpszResult[FILENAME_MAX];  /* String for executable file   */

   printmsg(2, "execute: command %s %s",
               command,
               parameters == NULL ? "" : parameters );

/*--------------------------------------------------------------------*/
/*                Determine local environment information             */
/*--------------------------------------------------------------------*/

   if ( myDirectory == NULL )
   {
      char *last;

      *batch = '\0';

      if ( GetModuleFileName( _hInstance, batch, sizeof batch ))
      {
         myDirectory = normalize( batch );   // Make slashes
         last = strrchr(myDirectory, '/');   // Locate file name start

         if ( last == NULL )
         {
            printmsg(0,"No path in module name: %s", myDirectory );
            panic();
         }

         *last = '\0';                          // Drop module name
         myDirectory = newstr( myDirectory );   // Save for posterity
         printmsg(4,"execute: Load directory is %s", myDirectory );
      }
      else {
         printmsg(0,"GetModuleFileName failed! (Buffer = \"%s\")",
                  batch);
         panic();
      }

   } /* if ( myDirectory == NULL ) */

/*--------------------------------------------------------------------*/
/*                          Locate the command                        */
/*--------------------------------------------------------------------*/

   if ( internal( command ))
   {
      strcpy( lpszResult, command );
      usebat = TRUE;
   }
   else {

      char *p;

      strcpy( batch, command );

      p = strrchr( batch, '/' ); // Get simple file name

      if ( p == NULL )
         p = strrchr( batch, '\\' );   // Try again for simple name

      if ( p == NULL )
         p = batch;                    // Okay, it's ALL simple.

      if ( strchr( p, '.' ) == NULL )  // If no extension ...
         strcat(p,".bat");             // ... search for .bat

      if ( equali(strrchr( batch, '.' ), ".bat") &&
          ((p = searchpath( batch )) != NULL ) )   // .bat exist?
      {
         if ( usebat )           // Using redirection?
         {
            printmsg(0,"Cannot use file redirection to batch file %s",
                        p );
            panic();
         }
         strcpy( lpszResult, command );   // Use simple name to allow
                                          // using normal DOS search rules
         usebat = TRUE;
      } /* if */

   } /* else */

/*--------------------------------------------------------------------*/
/*       Not an internal command nor is it a possible batch file;     */
/*       search for the actual executable file                        */
/*--------------------------------------------------------------------*/

   if ( ! usebat )
   {

      result = FindExecutable((LPSTR) command, myDirectory, lpszResult);

      if ( result <= 32 )
      {
         printmsg(0,"execute: FindExecutable returned error code %d", result);
         printmsg(2,"Command to execute for %s is %s",
                     command,
                     lpszResult );
         return -1;
      }
   }  /* else */

/*--------------------------------------------------------------------*/
/*     Generate a batch file for redirected DOS programs, if needed   */
/*--------------------------------------------------------------------*/

   if ( usebat )
   {
      FILE *stream ;

      mktempname( batch, "BAT");
      mktempname( perfect, "TMP");
      stream = FOPEN( batch, "w", TEXT_MODE );

      if ( stream == NULL )
      {
         printerr( batch );
         panic();
      }

      fprintf( stream ,
               "@echo off\n%s %s",
               lpszResult,
               parameters == NULL ? "" : parameters );

      if ( input != NULL )
         fprintf( stream, " < %s", input );

      if ( output != NULL )
         fprintf( stream, " < %s", output );

      fprintf( stream,
              "\nif errorlevel 1 erase %s\n",
               perfect );

      fclose ( stream );

      stream = FOPEN( perfect, "w", TEXT_MODE );
      if ( stream == NULL )
      {
         printerr( perfect );
         panic();
      }
      fclose( stream );

      strcpy( lpszResult, batch );     // Run the batch command

   } /* if ( usebat ) */

/*--------------------------------------------------------------------*/
/*                       Actually run the command                     */
/*--------------------------------------------------------------------*/

   result = SpawnWait( lpszResult,
                       parameters,
                       synchronous,
                       foreground ? SW_MAXIMIZE : SW_SHOWMINNOACTIVE );

/*--------------------------------------------------------------------*/
/*       For batch files, we can only report zero/non-zero            */
/*       results.  Do so, and clean up our input file at the same     */
/*       time.                                                        */
/*--------------------------------------------------------------------*/

   if ( usebat )
   {
      int unlinkResult = unlink( perfect );

      if (( result == 0 ) && (unlinkResult != 0))
         result = 255;

      if (unlink( batch ))
         printerr( batch );

   } /* if ( usebat ) */

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( (result == 0 ) ? 8 : 1,"Result of spawn %s is ... %d",
                                 command, result);

   return result;

} /* execute */

#elif defined(WIN32)
int execute( const char *command,
             const char *parameters,
             const char *input,
             const char *output,
             const boolean synchronous,
             const boolean foreground )
{
   int result;
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
      if ( input != NULL )
      {
         FILE *temp = freopen("con", "rt", stdin);

         if ( (temp == NULL) && (errno != 0) )
         {
            printerr("stdin");
            panic();
         }
         setvbuf( stdin, NULL, _IONBF, 0);

      } /* if ( input != NULL ) */
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
         strcat( buf, parameters );

         result = system( buf );
      } /* else */

   } /* if (internal(command)) */
   else  {                       /* No --> Invoke normally           */
      char buf[BUFSIZ];
      STARTUPINFO si;
      PROCESS_INFORMATION pi;
      void *oldCtrlCHandler;
      const static char *extensions[] = { ".exe",
                                          ".bat",
                                          ".cmd",
                                          ".com",
                                          NULL };
      char *currentExtension = extensions[0];
      char *filePart;

      memset(&si, 0, sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      si.lpTitle = command;
      si.dwFlags = STARTF_USESHOWWINDOW;
      si.wShowWindow = foreground ? SW_MAXIMIZE : SW_SHOWMINNOACTIVE;

      strcpy( buf, command );

      strlwr(buf);

/* Put the extension on the file */
      if (strchr(buf, '.') == NULL) {
         char filename[BUFSIZ];

         while (currentExtension != NULL) {
            if (0 == SearchPath(NULL, buf, currentExtension, BUFSIZ, filename,
                  &filePart)) {
               currentExtension++;
            } else {
               strcpy(buf, filePart);
               break;
            }
         }
      }

      if (parameters != NULL) {
         strcat( buf, " ");
         strcat( buf, parameters );
      }

      result = CreateProcess(NULL, buf, NULL, NULL, TRUE,
         0, NULL, NULL, &si, &pi);

      if (!result) {       /* Did CreateProcess() fail?              */
         printmsg(0, "execute:  CreateProcess failed, error %d", GetLastError());
         printerr(command);   /* Yes --> Report error                */
      } else {

         result = 0;

         if (synchronous) {

/* Set things up so that we ignore Ctrl-C's coming in to the child */
            oldCtrlCHandler = signal(SIGINT, SIG_IGN);

/* Wait for other app to finish */
            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &result);

/* Re-enable Ctrl-C handling */
            signal(SIGINT, oldCtrlCHandler);
         }

         /* If we're spawning asynchronously, I'm assuming that we
         don't care about the exit code from the spawned process.  Closing
         these makes it impossible to get at the old process's exit code. */

         CloseHandle(pi.hProcess);
         CloseHandle(pi.hThread);

      } /* else !result */
   } /* else internal command */

/*--------------------------------------------------------------------*/
/*                  Re-open our standard i/o streams                  */
/*--------------------------------------------------------------------*/

   errno = 0;

   if ( output != NULL )
   {
      freopen("con", "wt", stdout);
      setvbuf( stdout, NULL, _IONBF, 0);
   }

   errno = 0;

   if ( input != NULL )
   {
      FILE *temp = freopen("con", "rt", stdin);

      if ( (temp == NULL) && (errno != 0) )
      {
         printerr("stdin");
         panic();
      }

      setvbuf( stdin, NULL, _IONBF, 0);

   } /* if ( input != NULL ) */

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( (result == 0 ) ? 4 : 1,"Result of spawn %s is ... %d",
                                 command, result);

   return result;


}

#else

#ifdef __TURBOC__
#pragma argsused
#endif

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
      if ( input != NULL )
      {
         FILE *temp = freopen("con", "rt", stdin);

         if ( (temp == NULL) && (errno != 0) )
         {
            printerr("stdin");
            panic();
         }
         setvbuf( stdin, NULL, _IONBF, 0);

      } /* if ( input != NULL ) */

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
         strcat( buf, parameters );

         result = system( buf );
      } /* else */

   } /* if (internal(command)) */
   else  {                       /* No --> Invoke normally           */

      result = spawnlp( synchronous ? P_WAIT : P_NOWAIT,
                        (char *) command,
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
   {
      freopen("con", "wt", stdout);
      setvbuf( stdout, NULL, _IONBF, 0);
   }

   errno = 0;

   if ( input != NULL )
   {
      FILE *temp = freopen("con", "rt", stdin);

      if ( (temp == NULL) && (errno != 0) )
      {
         printerr("stdin");
         panic();
      }
      setvbuf( stdin, NULL, _IONBF, 0);

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
