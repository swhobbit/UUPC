/*--------------------------------------------------------------------*/
/*       e x e c u t e . C                                            */
/*                                                                    */
/*       Execute an external command for UUPC/extended functions      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: execute.c 1.46 1998/03/01 01:23:27 ahd v1-13b $
 *
 *    Revision history:
 *    $Log: execute.c $
 *    Revision 1.46  1998/03/01  01:23:27  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.45  1997/12/14 00:43:16  ahd
 *    Flush stderr/stdout before invoking inferior process
 *
 *    Revision 1.44  1997/03/31 07:12:17  ahd
 *    COpyright update
 *    Restore previous title of window
 *
 *    Revision 1.43  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.42  1996/01/01 20:50:52  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.41  1995/03/11 22:33:46  ahd
 *    Blow off undelete processing if so configured under OS/2
 *
 *    Revision 1.40  1995/03/11 15:49:23  ahd
 *    Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *    Revision 1.39  1995/03/08 03:00:20  ahd
 *    Work around IBM C/Set++ bug which generates spurious operand
 *    when no operands are passed to program.
 *
 *    Revision 1.38  1995/01/07 16:12:13  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.37  1995/01/05 03:41:36  ahd
 *    Print more precise error messages for file errors
 *
 *    Revision 1.36  1994/12/22 00:08:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.35  1994/10/23 23:29:44  ahd
 *    Under OS/2 and Windows NT, always use current window when processing
 *    synchronous commands.
 *
 * Revision 1.34  1994/10/02  20:47:38  ahd
 * Correct processing for synchronous command
 *
 * Revision 1.33  1994/08/08  00:21:00  ahd
 * Further OS/2 cleanup
 *
 * Revision 1.32  1994/08/07  21:28:54  ahd
 * Clean up OS/2 processing to not use new sessions, but rather simply user
 * command processor to allow firing off PM programs such as E and EPM.
 *
 * Revision 1.31  1994/05/23  22:47:23  ahd
 * Include commands ending in .cmd for ALL 32 bit environments
 *
 * Revision 1.30  1994/02/28  01:02:06  ahd
 * Reopen output file after input file.
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
#include "title.h"

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

static KWBoolean internal( const char *command );

static KWBoolean batch( const char *input, char *output);

#if defined(__OS2__) || defined(FAMILYAPI) || defined(WIN32)

static int executeAsync( const char *command,
                         const char *parameters,
                         const KWBoolean synchronous,
                         const KWBoolean foreground );
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
             const KWBoolean synchronous,
             const KWBoolean foreground )
{
   int result;

   KWBoolean useBat = (input != NULL) || (output != NULL );

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
      useBat = KWTrue;
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
         printmsg(0,"execute: Cannot generate batch file %s",
                     batchFile );
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
         printmsg(0,"Cannot generate test batch file %s", perfect );
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
      int unlinkResult = REMOVE( perfect );

      if (( result == 0 ) && (unlinkResult != 0))
         result = 255;

      if (REMOVE( batchFile ))
      {

         printmsg(0,"Cannot delete batch file %s", batchFile );
         printerr( batchFile );
      }

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
             const KWBoolean synchronous,
             const KWBoolean foreground )
{
   int result;
   int tempHandle;
   char path[BUFSIZ];

   printmsg(3,"Command = %s, parameters = \"%s\"%s%s%s%s, %s, %s.",
               command,
               parameters,
               input == NULL ? "" : ", input = ",
               input == NULL ? "" : input,
               output == NULL ? "" : ", output = ",
               output == NULL ? "" : output,
               synchronous ? "synchronous" : "asynchronous",
               foreground  ? "foreground"  : "background" );

/*--------------------------------------------------------------------*/
/*               Redirect STDIN and STDOUT as required                */
/*--------------------------------------------------------------------*/

   fflush( stderr );
   fflush( stdout );

   if ( (input != NULL) || (output != NULL) )
   {
      if ( ! synchronous )
      {
         printmsg(0, "execute: Internal error, "
                     "cannot redirect asynchronous command %s",
                 command );
         panic();

      } /* if ( ! synchronous ) */
   }

   if (input != NULL)
   {
     if ((tempHandle = open(input, O_RDONLY|O_BINARY)) == -1)
     {
        printmsg(0,"execute: Cannot redirect input from %s",
                    input );
       printerr(input);
       return -2;
     }

     if (dup2(tempHandle, 0))
     {
        printmsg(0,"execute: Cannot dup handle for %s",
                    input );
         printerr( input );
         panic();
     }
     close(tempHandle);
   }

   if (output != NULL)
   {
     if ((tempHandle = open(output, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, 0666)) == -1)
     {
        printmsg(0,"execute: Cannot redirect output to %s",
                    output );
       printerr( output );

       if ( input != NULL )
       {
         FILE *tempStream = freopen(CONSOLE, "r", stdin);

         if ( (tempStream == NULL) && (errno != 0) )
         {
            printmsg(0,"execute: Cannot reopen original standard in" );
            printerr("stdin");
            panic();
         }
         setvbuf( stdin, NULL, _IONBF, 0);

      } /* if ( input != NULL ) */

      return -2;

     }

     if (dup2(tempHandle, 1))
     {
         printmsg(0,"execute: Cannot dup original standard in" );
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

      result = system( path );

   } /* if (internal(command)) */
   else if ( ! *path )
      result = -3;                  /* Flag we never ran command     */
   else {

#if defined(WIN32) || defined(__OS2__) || defined(FAMILYAPI)

      result = executeAsync( path, parameters, synchronous, foreground );

#else

      fflush( stderr );
      fflush( stdout );
      result = spawnl(  P_WAIT,
                        (char *) path,
#ifdef __IBMC__
                        (char *) (parameters ? command : NULL),
#else
                        (char *) command,
#endif
                        (char *) parameters,
                        NULL);

      if (result == -1)       /* Did spawn fail?                   */
         printerr(command);   /* Yes --> Report error              */

#endif

   } /* else */

   setTitle( NULL);                 /* Restore previous title      */

/*--------------------------------------------------------------------*/
/*                  Re-open our standard i/o streams                  */
/*--------------------------------------------------------------------*/

   if ( input != NULL )
   {
      FILE *tempStream = freopen(CONSOLE, "r", stdin);

      if (tempStream == NULL)
      {
         printerr("execute:" CONSOLE);
         panic();
      }

      setvbuf( stdin, NULL, _IONBF, 0);

   } /* if ( input != NULL ) */

   if ( output != NULL )
   {
      FILE *tempStream = freopen(CONSOLE, "w", stdout);

      if (tempStream == NULL)
      {
         printerr( "execute: " CONSOLE);
         panic();
      }
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
                    const KWBoolean synchronous,
                    const KWBoolean foreground )
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

static KWBoolean internal( const char *command )
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
/*       Empty commands are a special signal to use the command       */
/*       processor to run the arguments.                              */
/*--------------------------------------------------------------------*/

   if ( *command == '\0' )
   {
      printmsg(4,"internal: Empty command, using command processor");
      return KWTrue;
   }

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
         return KWTrue;
      } /* if */

   } /* while( *list != NULL ) */

/*--------------------------------------------------------------------*/
/*       The command is not in the list; return KWFalse (external      */
/*       command)                                                     */
/*--------------------------------------------------------------------*/

   printmsg(4,"\"%s\" is an external command",command);
   return KWFalse;

} /* internal */

/*--------------------------------------------------------------------*/
/*    b a t c h                                                       */
/*                                                                    */
/*    Determine if a command is batch file                            */
/*--------------------------------------------------------------------*/

static KWBoolean batch( const char *input, char *output)
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
         return KWFalse;

      }  /* if ( ! *output ) */

#if defined(_DOS) || defined(_Windows)
      return equal( period, ".bat" );
#else

      if (equali( period, ".cmd" ) || equali( period, ".bat" ))
         return KWTrue;
      else
         return KWFalse;
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
            if (equal( extensions[extension] , ".cmd" ) ||
                   equal( extensions[extension] , ".bat" ))
               return KWTrue;
            else
               return KWFalse;
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

   return KWFalse;

} /* batch */

#if defined(__OS2__) || defined(FAMILYAPI)

/*--------------------------------------------------------------------*/
/*       e x e c u t e A s y n c                                      */
/*                                                                    */
/*       Run a command in a new session under OS/2                    */
/*--------------------------------------------------------------------*/

static int executeAsync( const char *command,
                         const char *parameters,
                         const KWBoolean synchronous,
                         const KWBoolean foreground )
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
/*       Special case foreground synchronous commands; this           */
/*       insures redirected input is processed properly.              */
/*--------------------------------------------------------------------*/

   if ( synchronous )
   {
      int result = spawnl(  P_WAIT,
                            (char *) command,
                            (char *) command,
                            (char *) parameters,
                            NULL);

      if (result == -1)       /* Did spawn fail?                   */
         printerr(command);   /* Yes --> Report error              */

      return result;

   } /*  */

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
/*       Build the queue to listen for the subtask ending.  This      */
/*       code is actually correct, but ignored since we returned      */
/*       after spawn() above.                                         */
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

   return (int) rc;

} /* executeAsync */

#elif defined(WIN32)

/*--------------------------------------------------------------------*/
/*    e x e c u t e A s y n c                   (Windows NT version)  */
/*                                                                    */
/*    Execute an external program                                     */
/*--------------------------------------------------------------------*/

int executeAsync( const char *command,
             const char *parameters,
             const KWBoolean synchronous,
             const KWBoolean foreground )
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

   if (synchronous)
      fdwCreate = 0;
   else
      fdwCreate = CREATE_NEW_CONSOLE;

   result = CreateProcess(NULL,
                          path,
                          NULL,
                          NULL,
                          KWTrue,
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

} /* executeAsync */

#endif
