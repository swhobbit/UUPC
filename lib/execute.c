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
 *    $Id: execute.c 1.5 1993/09/20 04:38:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: execute.c $
 * Revision 1.5  1993/09/20  04:38:11  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support
 *
 * Revision 1.4  1993/08/08  17:39:09  ahd
 * Denormalize path for opening on selected networks
 *
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <process.h>
#include <io.h>

#ifdef WIN32
#include <windows.h>
#include <signal.h>
#elif defined(_Windows)
#include <windows.h>
#include <shellapi.h>
#endif

#include <direct.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "execute.h"

#ifdef _Windows
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static boolean internal( const char *command );

static boolean batch( const char *input, char *output);

#ifdef _Windows

/*--------------------------------------------------------------------*/
/*       e x e c u t e                       (Windows 3.x version)    */
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

   boolean usebat = (input != NULL) || (output != NULL );

   char path[FILENAME_MAX];         /* String for executable file   */
   char batchFile[FILENAME_MAX];    /* String for batch driver file */
   char perfect[FILENAME_MAX];      /* String for results test file */

   printmsg(2, "execute: command %s %s",
               command,
               parameters == NULL ? "" : parameters );

/*--------------------------------------------------------------------*/
/*                          Locate the command                        */
/*--------------------------------------------------------------------*/

   if ( internal( command ) )
   {
      strcpy( path , command );
      usebat = TRUE;
   }
   else if (batch( command, path ))
   {
      if (useBat)                      // Using redirection?
      {
         printmsg(0,"Cannot use redirection with batch file %s",
                     path );
         return -2;
      }
   } /* else */
   else if ( !*path )                  // Error returned from search?
      return -1;                       // Yes --> Error already reported

/*--------------------------------------------------------------------*/
/*     Generate a batch file for redirected DOS programs, if needed   */
/*--------------------------------------------------------------------*/

   if ( usebat )
   {
      FILE *stream ;

      mktempname( batchFile, "BAT");
      mktempname( perfect, "TMP");
      stream = FOPEN( batchFile, "w", TEXT_MODE );

      if ( stream == NULL )
      {
         printerr( batchFile );
         panic();
      }

      fprintf( stream ,
               "@echo off\n%s %s",
               path,
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

      strcpy( path, batchFile );             // Run the batch command

   } /* if ( usebat ) */

/*--------------------------------------------------------------------*/
/*                       Actually run the command                     */
/*--------------------------------------------------------------------*/

   result = SpawnWait( path,
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

      if (unlink( batchFile ))
         printerr( batchFile );

   } /* if ( usebat ) */

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( 4,"Result of spawn %s is ... %d", command, result);

   return result;

} /* execute */

#elif defined(WIN32)

/*--------------------------------------------------------------------*/
/*    e x e c u t e                             (Windows NT version)  */
/*                                                                    */
/*    Execute an external program                                     */
/*--------------------------------------------------------------------*/

int execute( const char *command,
             const char *parameters,
             const char *input,
             const char *output,
             const boolean synchronous,
             const boolean foreground )
{
   int result;
   char path[BUFSIZ];

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

   if (internal(strcpy(path,command)) ||
       batch(command, path))        // Internal or batch command?
   {

      if ( parameters == NULL )
         result = system( path );
      else {

         strcat( path, " ");
         strcat( path, parameters );

         result = system( path );

      } /* else */

   } /* if (internal(command)) */
   else  {                       /* No --> Invoke normally           */
      STARTUPINFO si;
      PROCESS_INFORMATION pi;
      void *oldCtrlCHandler;

      if ( ! *path )                // Did search fail?
         return -2;                 // Yes --> Msg issued, just return

      memset(&si, 0, sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      si.lpTitle = (LPSTR)command;
      si.dwFlags = STARTF_USESHOWWINDOW;
      si.wShowWindow = foreground ? SW_MAXIMIZE : SW_SHOWMINNOACTIVE;


      if (parameters != NULL)
      {
         strcat( path, " ");
         strcat( path, parameters );
      }

      result = CreateProcess(NULL,
                             path,
                             NULL,
                             NULL,
                             TRUE,
                             0,
                             NULL,
                             NULL,
                             &si,
                             &pi);

      if (!result)                  // Did CreateProcess() fail?
      {                             // Yes --> Report error
         printmsg(0, "execute:  CreateProcess failed, error %d",
                     GetLastError());
      }
      else {

         if (synchronous)
         {

/*--------------------------------------------------------------------*/
/*       Set things up so that we ignore Ctrl-C's coming in to the    */
/*       child, and wait for other application to finish.             */
/*--------------------------------------------------------------------*/

            oldCtrlCHandler = signal(SIGINT, SIG_IGN);

            WaitForSingleObject(pi.hProcess, INFINITE);
            GetExitCodeProcess(pi.hProcess, &result);

            signal(SIGINT, oldCtrlCHandler); // Re-enable Ctrl-C handling

         }  /* if (synchronous) */
         else
            result = 0;

/*--------------------------------------------------------------------*/
/*       If we're spawning asynchronously, we assume that we don't    */
/*       care about the exit code from the spawned process.           */
/*       Closing these makes it impossible to get at the old          */
/*       process's exit code.                                         */
/*--------------------------------------------------------------------*/

         CloseHandle(pi.hProcess);
         CloseHandle(pi.hThread);

      } /* else !result */

   } /* else internal command */

/*--------------------------------------------------------------------*/
/*                  Re-open our standard i/o streams                  */
/*--------------------------------------------------------------------*/

   if ( output != NULL )
   {
      freopen("con", "wt", stdout);
      setvbuf( stdout, NULL, _IONBF, 0);
   }

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

#else

#ifdef __TURBOC__
#pragma argsused
#endif

/*--------------------------------------------------------------------*/
/*       e x e c u t e                       (OS/2 + DOS version)     */
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
   char path[BUFSIZ];

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

   if (internal(strcpy(path,command)) ||
       batch(command,path))         // Internal command or batch file?
   {

      if ( parameters == NULL )
         result = system( path );
      else {

         strcat( path, " ");
         strcat( path, parameters );

         result = system( path );
      } /* else */

   } /* if (internal(command)) */
   else  {                       /* No --> Invoke normally           */

      if ( *path )
      {
         result = spawnlp( synchronous ? P_WAIT : P_NOWAIT,
                           (char *) path,
                           (char *) command,
                           (char *) parameters,
                           NULL);

         if (result == -1)       /* Did spawn fail?                  */
            printerr(command);   /* Yes --> Report error             */
      } /* else */
      else
         result = -3;            // Flag we never ran command

   } /* else */

/*--------------------------------------------------------------------*/
/*                  Re-open our standard i/o streams                  */
/*--------------------------------------------------------------------*/

   if ( output != NULL )
   {
      freopen("con", "wt", stdout);
      setvbuf( stdout, NULL, _IONBF, 0);
   }

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
   char buffer[FILENAME_MAX];
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

/*--------------------------------------------------------------------*/
/*    b a t c h                                                       */
/*                                                                    */
/*    Determine if a command is batch file                            */
/*--------------------------------------------------------------------*/

static boolean batch( const char *input, char *output)
{
   char *search = getenv("PATH");
   char *gotPath;
   char *period;

   const static char *extensions[] = { ".exe",
                                       ".com",
#if !defined(_DOS) && !defined(_Windows)
                                       ".cmd",
#endif
                                       ".bat",
                                       NULL };

/*--------------------------------------------------------------------*/
/*                  Validate the search path exists                   */
/*--------------------------------------------------------------------*/

   if ( search == NULL )
   {
      printmsg(0,"batch: Unable to retrieve PATH environment variable!");
      panic();
   }

/*--------------------------------------------------------------------*/
/*        Determine if we have path, and if we have an extension      */
/*--------------------------------------------------------------------*/

   gotPath = strchr( input, '/');
   if ( gotPath == NULL )
      gotPath = strchr( input, '\\');

   period = strchr( (gotPath == NULL) ? input : gotPath, '.');

   if ( period != NULL )         //    We have extension?
   {
      if ( gotPath )             // Extension + path?
      {                          // Yes --> Just look for the file

         char *fname = normalize( input );

         if ( access( input, 00))
            *output = '\0';
         else
            strcpy( output, fname );

      } /* if ( gotPath ) */
      else
         _searchenv( input, "PATH", output );

      if ( ! *output )           // No file found?
      {

         printerr( input );
         return FALSE;

      }  /* if ( ! *output ) */

#if defined(_DOS) || defined(_Windows)
      return equal( period, ".bat" );
#else
      return equali( period, ".cmd" ) || equali( period, ".bat" );
#endif

   } /* if ( p != NULL ) */

/*--------------------------------------------------------------------*/
/*       Walk the path looking for the file's possible types in       */
/*       the path's directories                                       */
/*--------------------------------------------------------------------*/

   while( *search )
   {
      char base[FILENAME_MAX];
      int extension = 0;

      if ( gotPath )
      {
         strcpy( base, input );
         search = "";                        // Force this to be last pass
      }
      else {

         char *next = strchr(search,';');    // Find next path component
         int len;

         if ( next == NULL )
            len = strlen( search );
         else
            len = next - search;

         memcpy( base, search, len );        // Path for search ...
         search += len + 1;                  // Step past semicolon
         if ( base[len - 1 ] != '\\' )       // Ending in back slash?
            base[len++] = '\\';              // No --> Add one
         strcpy( base + len , input );       // ... plus file name

      } /* else */

      printmsg(8,
               "Searching for extension of %s",
               base );

/*--------------------------------------------------------------------*/
/*       Search a single directory in a path for a file with          */
/*       various extensions.                                          */
/*--------------------------------------------------------------------*/

      while( extensions[extension] != NULL )
      {
         strcpy( output, base );
         strcat( output, extensions[extension] );

         if ( ! access(output, 00 ))
         {

#if defined(_DOS) || defined(_Windows)
            return equal( extensions[extension] , ".bat" );
#else
            return equal( extensions[extension] , ".cmd" ) ||
                   equal( extensions[extension] , ".bat" );
#endif
         } /* if ( result != NULL ) */

         extension++;

      }  /* while( extensions[extension] != NULL ) */

   } /* while( *search ) */

/*--------------------------------------------------------------------*/
/*       We could not find the file, report failure to the caller     */
/*--------------------------------------------------------------------*/

   printmsg(0, "batch: Unable to locate %s in search path", input);

   *output = '\0';                  // Flag no file found!
   return FALSE;

} /* batch */
