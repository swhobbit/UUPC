/*--------------------------------------------------------------------*/
/*       e x e c u t e . C                                            */
/*                                                                    */
/*       Execute an external command for UUPC/extended functions      */
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
 *    $Id: execute.c 1.30 1994/02/28 01:02:06 ahd Exp $
 *
 *    Revision history:
 *    $Log: execute.c $
 * Revision 1.30  1994/02/28  01:02:06  ahd
 * Reopen output file after input file.
 * .\
 *
 * Revision 1.29  1994/02/19  04:39:30  ahd
 * Use standard first header
 *
 * Revision 1.28  1994/02/19  04:04:23  ahd
 * Use standard first header
 *
 * Revision 1.27  1994/02/19  03:47:51  ahd
 * Use standard first header
 *
 * Revision 1.26  1994/02/18  23:07:43  ahd
 * Use standard first header
 *
 * Revision 1.25  1994/02/14  01:03:56  ahd
 * Use properly sized buffer for executeCommand processing
 *
 * Revision 1.24  1994/02/13  13:52:33  rommel
 * Cosmetic changes to lower case file name extensions
 *
 * Revision 1.23  1994/01/01  19:01:27  ahd
 * Annual Copyright Update
 *
 * Revision 1.22  1993/12/29  02:46:47  ahd
 * Trap errors from dup2() calls
 *
 * Revision 1.21  1993/12/23  03:11:17  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.20  1993/12/06  01:59:07  ahd
 * Delete debug message from inner loop for search for internal commands
 *
 * Revision 1.19  1993/11/30  04:16:23  dmwatt
 * Add Windows NT executeAsync()
 *
 * Revision 1.18  1993/11/14  20:51:37  ahd
 * Correct Windows 3.1 compile error
 *
 * Revision 1.17  1993/11/13  17:37:02  ahd
 * Only use system() call for CMD files under OS/2
 *
 * Revision 1.16  1993/11/08  04:46:49  ahd
 * Add OS/2 specific support for seperate sessions
 *
 * Revision 1.15  1993/11/06  17:54:55  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.14  1993/10/30  17:10:40  rhg
 * Correct direction of redirection for Windows batch files
 *
 * Revision 1.13  1993/10/12  00:43:34  ahd
 * Normalize comments
 *
 * Revision 1.12  1993/10/09  15:46:15  rhg
 * ANSIify the source
 *
 * Revision 1.11  1993/10/03  22:09:09  ahd
 * Change debugging messages
 *
 * Revision 1.10  1993/10/02  22:56:59  ahd
 * Suppress compile warning
 *
 * Revision 1.9  1993/10/02  19:07:49  ahd
 * Suppress compiler warning
 *
 * Revision 1.8  1993/09/27  00:45:20  ahd
 * Fix Windows compile, add debug to OS/2 and DOS version
 *
 * Revision 1.7  1993/09/26  03:32:27  dmwatt
 * Use Standard Windows NT error message module
 *
 * Revision 1.6  1993/09/23  03:26:51  ahd
 * Use common file search routine
 *
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

#include "uupcmoah.h"

#include <errno.h>

#include <ctype.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>

#if defined(WIN32)
#include <windows.h>
#include <signal.h>
#elif defined(__OS2__) || defined(FAMILYAPI)
#define INCL_DOSSESMGR
#define INCL_DOSQUEUES
#define INCL_NOPM
#include <os2.h>
#endif

#include <direct.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "execute.h"

#ifdef _Windows
#include <windows.h>
#include "winutil.h"
#elif defined(WIN32)
#include "pnterr.h"
#elif defined(__OS2__) || defined(FAMILYAPI)
#include "pos2err.h"
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

#if defined(__OS2__) || defined(FAMILYAPI) || defined(WIN32)

static int executeAsync( const char *command,
                         const char *parameters,
                         const boolean synchronous,
                         const boolean foreground );
#endif

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

   boolean useBat = (input != NULL) || (output != NULL );

   char path[FILENAME_MAX];         /* String for executable file   */
   char batchFile[FILENAME_MAX];    /* String for batch driver file */
   char perfect[FILENAME_MAX];      /* String for results test file */

/*--------------------------------------------------------------------*/
/*                    Validate command redirection                    */
/*--------------------------------------------------------------------*/

   if ( ((input != NULL) || (output != NULL)) && ! synchronous )
   {
      printmsg(0, "execute: Internal error, cannot redirect asynchronous"
                  " command %s",
                 command );
      panic();
   }

/*--------------------------------------------------------------------*/
/*                          Locate the command                        */
/*--------------------------------------------------------------------*/

   if ( internal( command ) )
   {
      strcpy( path , command );
      useBat = TRUE;
   }
   else if (batch( command, path ))
   {
      if (useBat)                      /* Using redirection?          */
      {
         printmsg(0,"Cannot use redirection with batch file %s",
                     path );
         return -2;
      }
   } /* else */
   else if ( !*path )                  /* Error returned from search? */
      return -1;                       /* Yes --> Error already reported */

/*--------------------------------------------------------------------*/
/*     Generate a batch file for redirected DOS programs, if needed   */
/*--------------------------------------------------------------------*/

   if ( useBat )
   {
      FILE *stream ;

      mktempname( batchFile, "bat");
      mktempname( perfect, "tmp");
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
         fprintf( stream, " > %s", output );

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

      strcpy( path, batchFile );             /* Run the batch command */

   } /* if ( useBat ) */

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

   if ( useBat )
   {
      int unlinkResult = unlink( perfect );

      if (( result == 0 ) && (unlinkResult != 0))
         result = 255;

      if (unlink( batchFile ))
         printerr( batchFile );

   } /* if ( useBat ) */

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( 4,"Result of spawn %s is ... %d", command, result);

   return result;

} /* execute */

#else

#if _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

/*--------------------------------------------------------------------*/
/*       e x e c u t e                 (OS/2, DOS, and NT version)    */
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
   int tempHandle;
   char path[BUFSIZ];
   boolean redirected;

/*--------------------------------------------------------------------*/
/*               Redirect STDIN and STDOUT as required                */
/*--------------------------------------------------------------------*/

   if ( (input != NULL) || (output != NULL) )
   {
      redirected = TRUE;

      if ( ! synchronous )
      {
         printmsg(0, "execute: Internal error, "
                     "cannot redirect asynchronous command %s",
                 command );
         panic();

      } /* if ( ! synchronous ) */
   }
   else
      redirected = FALSE;

   if (input != NULL)
   {
     if ((tempHandle = open(input, O_RDONLY|O_BINARY)) == -1)
     {
       printerr(input);
       return -2;
     }

     if (dup2(tempHandle, 0))
     {
         printerr( input );
         panic();
     }
     close(tempHandle);
   }

   if (output != NULL)
   {
     if ((tempHandle = open(output, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, 0666)) == -1)
     {
       printerr( output );
       if ( input != NULL )
       {
         FILE *tempStream = freopen(CONSOLE, "r", stdin);

         if ( (tempStream == NULL) && (errno != 0) )
         {
            printerr("stdin");
            panic();
         }
         setvbuf( stdin, NULL, _IONBF, 0);

      } /* if ( input != NULL ) */

      return -2;

     }

     if (dup2(tempHandle, 1))
     {
         printerr( input );
         panic();
     }
     close(tempHandle);

   } /* if (output != NULL) */

/*--------------------------------------------------------------------*/
/*                  Execute the command in question                   */
/*--------------------------------------------------------------------*/

   strcpy(path, command);

   if (internal(path) ||
       batch(command, path))        /* Internal command or batch file? */
   {

      if ( parameters != NULL )
      {
         strcat( path, " ");
         strcat( path, parameters );
      }

#if defined(__OS2__) || defined(FAMILYAPI)
      result = system( path );
#else
      result = system( path );
#endif

   } /* if (internal(command)) */
   else if ( ! *path )
      result = -3;            /* Flag we never ran command         */
#if defined(__OS2__) || defined(FAMILYAPI)
   else if ((foreground && ! redirected) || ! synchronous)
      result = executeAsync( path, parameters, synchronous, foreground );
#endif
   else {
#ifdef WIN32
      result = executeAsync( path, parameters, synchronous, foreground );
#else
      result = spawnl(  P_WAIT,
                        (char *) path,
                        (char *) command,
                        (char *) parameters,
                        NULL);
      if (result == -1)       /* Did spawn fail?                   */
         printerr(command);   /* Yes --> Report error              */
#endif

   } /* else */

/*--------------------------------------------------------------------*/
/*                  Re-open our standard i/o streams                  */
/*--------------------------------------------------------------------*/

   if ( input != NULL )
   {
      FILE *tempStream = freopen(CONSOLE, "r", stdin);

      if ( (tempStream == NULL) && (errno != 0) )
      {
         printerr("stdin");
         panic();
      }
      setvbuf( stdin, NULL, _IONBF, 0);

   } /* if ( input != NULL ) */

   if ( output != NULL )
   {
      freopen(CONSOLE, "w", stdout);
      setvbuf( stdout, NULL, _IONBF, 0);
   }

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( 4,"Result of spawn %s is ... %d", command, result);

   return result;

} /* execute */

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

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
                               "rename",  "rmdir", "time",     "type",
                               "ver",     "verify",  "vol",
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

   static const char *extensions[] = { ".exe",
                                       ".com",
#if (!defined(_DOS) && !defined(_Windows)) || defined(BIT32ENV)
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

   if ( period != NULL )         /*    We have extension?             */
   {
      if ( gotPath )             /* Extension + path?                 */
      {                          /* Yes --> Just look for the file    */

         char *fname = normalize( input );

         if ( access( input, 00))
            *output = '\0';
         else
            strcpy( output, fname );

      } /* if ( gotPath ) */
      else
         _searchenv( (char *) input, "PATH", output );

      if ( ! *output )           /* No file found?                    */
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
         search = "";                        /* Force this to be last pass */
      }
      else {

         char *next = strchr(search,';');    /* Find next path component */
         size_t len;

         if ( next == NULL )
            len = strlen( search );
         else
            len = (size_t) (next - search);

         memcpy( base, search, len );        /* Path for search ...   */
         search += len + 1;                  /* Step past semicolon   */
         if ( base[len - 1 ] != '\\' )       /* Ending in back slash? */
            base[len++] = '\\';              /* No --> Add one        */
         strcpy( base + len , input );       /* ... plus file name    */

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

   *output = '\0';                  /* Flag no file found!            */

   return FALSE;

} /* batch */

#if defined(__OS2__) || defined(FAMILYAPI)

/*--------------------------------------------------------------------*/
/*       e x e c u t e A s y n c                                      */
/*                                                                    */
/*       Run a command in a new session under OS/2                    */
/*--------------------------------------------------------------------*/

static int executeAsync( const char *command,
                         const char *parameters,
                         const boolean synchronous,
                         const boolean foreground )
{
   STARTDATA sd;
   PID   childPID;
   static int instance = 0;      /* Number of program we've invoke   */

   char  queueName[FILENAME_MAX];
   PVOID queueDataAddress;
   BYTE  queueElementPriority;
   HQUEUE queueHandle;

#ifdef __OS2__
   APIRET rc;
   REQUESTDATA  queueRequest;
   ULONG sessID;
   ULONG queueDataLength;
#else
   USHORT rc;
   QUEUERESULT  queueRequest;
   USHORT sessID;
   USHORT queueDataLength;

#ifndef SSF_RELATED_CHILD

   #define SSF_RELATED_INDEPENDENT 0
   #define SSF_RELATED_CHILD       1

   #define SSF_FGBG_FORE           0
   #define SSF_FGBG_BACK           1

   #define SSF_TRACEOPT_NONE       0

   #define SSF_INHERTOPT_PARENT    1

   #define SSF_TYPE_DEFAULT        0

   #define SSF_CONTROL_MAXIMIZE    0x0002
   #define SSF_CONTROL_MINIMIZE    0x0004

#endif  /* SSF_RELATED_CHILD */
#endif

/*--------------------------------------------------------------------*/
/*              Initialize the start session parameters               */
/*--------------------------------------------------------------------*/

   memset( (void *) &sd, 0, sizeof sd );

   sd.Length      = 32;          /* Just basic info + InheritOpt     */

   sd.FgBg        = (USHORT) (foreground ? SSF_FGBG_FORE : SSF_FGBG_BACK);
   sd.TraceOpt    = SSF_TRACEOPT_NONE;
   sd.PgmName     = (PSZ) command;
   sd.PgmInputs   = (PSZ) parameters;
   sd.Environment = 0;           /* Just use our own envionment      */
   sd.InheritOpt  = SSF_INHERTOPT_PARENT; /* Pass it to child        */
   sd.SessionType = SSF_TYPE_DEFAULT;     /* Let the system pick
                                             session type            */
   sd.PgmControl  = (USHORT) (foreground ?
                        SSF_CONTROL_MAXIMIZE : SSF_CONTROL_MINIMIZE);

/*--------------------------------------------------------------------*/
/*          Build the queue to listen for the subtask ending          */
/*--------------------------------------------------------------------*/

   if ( synchronous )
   {
      sprintf(queueName,
              "\\queues\\pid%d\\pgm%d",
              (int) getpid(),
              instance++);

      rc = DosCreateQueue( &queueHandle,
#ifdef __OS2__
                           QUE_FIFO | QUE_CONVERT_ADDRESS,
#else
                           QUE_FIFO,
#endif
                           (PSZ) queueName );

      if ( rc )
      {
         printOS2error( queueName, rc );
         return -4;                 /* Report command never was run     */
      }

      sd.TermQ       = (PSZ) queueName;
      sd.Related     = SSF_RELATED_CHILD;       /* Child session        */

   } /* if ( synchronous ) */
   else {

      sd.TermQ       = (PSZ) 0;     /* Don't wait for session end       */
      sd.Related     = SSF_RELATED_INDEPENDENT; /* Not a child session  */

   } /* else */

   rc = DosStartSession( &sd,
                         &sessID,
                         &childPID );

   if ( rc )
   {
      printOS2error( command ? command : parameters, rc );
      return -5;
   }

/*--------------------------------------------------------------------*/
/*      If the command is running asynchonously return to caller      */
/*--------------------------------------------------------------------*/

   if ( ! synchronous )
      return 0;

/*--------------------------------------------------------------------*/
/*                   Wait for the child to complete                   */
/*--------------------------------------------------------------------*/

   memset( (void *) &queueRequest, 0, sizeof queueRequest );

   rc = DosReadQueue( queueHandle,
                      &queueRequest,
                      &queueDataLength,
                      &queueDataAddress,
                      0,            /* First element in the queue    */
                      0,            /* Wait for queue to be ready    */
                      &queueElementPriority,
                      0);           /* Semaphore handle -- not used  */

   if ( rc )
   {
      printOS2error( queueName, rc );
      panic();
   }

   rc = ((unsigned short FAR*) queueDataAddress)[1];

#ifdef __OS2__
   DosFreeMem( queueDataAddress );
#else
   DosFreeSeg( SELECTOROF(queueDataAddress) );
#endif

   return rc;

} /* executeAsync */

#elif defined(WIN32)
/*--------------------------------------------------------------------*/
/*    e x e c u t e A s y n c                   (Windows NT version)  */
/*                                                                    */
/*    Execute an external program                                     */
/*--------------------------------------------------------------------*/

int executeAsync( const char *command,
             const char *parameters,
             const boolean synchronous,
             const boolean foreground )
{
   int result;
   char path[BUFSIZ];
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   void *oldCtrlCHandler;
   DWORD fdwCreate;

/*--------------------------------------------------------------------*/
/*                  Execute the command in question                   */
/*--------------------------------------------------------------------*/

   strcpy(path, command);

   if ( ! *path )                /* Did search fail?               */
      return -2;                 /* Yes --> Msg issued, just return */

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

   if (!synchronous)
      fdwCreate = CREATE_NEW_CONSOLE;
   else if (synchronous && foreground)
      fdwCreate = CREATE_NEW_CONSOLE;
   else
      fdwCreate = 0;

   result = CreateProcess(NULL,
                          path,
                          NULL,
                          NULL,
                          TRUE,
                          fdwCreate,
                          NULL,
                          NULL,
                          &si,
                          &pi);

   if (!result)                  /* Did CreateProcess() fail?      */
   {                             /* Yes --> Report error           */
      DWORD dwError = GetLastError();
      printmsg(0, "execute:  CreateProcess failed");
      printNTerror("CreateProcess", dwError);
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

         signal(SIGINT, oldCtrlCHandler); /* Re-enable Ctrl-C handling */

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

/*--------------------------------------------------------------------*/
/*                     Report results of command                      */
/*--------------------------------------------------------------------*/

   printmsg( 4 ,"Result of spawn %s is ... %d", command, result);

   return result;

} /* execute */

#endif
