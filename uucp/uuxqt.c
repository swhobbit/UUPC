/*--------------------------------------------------------------------*/
/*    Program:    uuxqt.c              23 September 1991              */
/*    Author:     Mitch Mitchell                                      */
/*    Email:      mitch@harlie.lonestar.org                           */
/*                                                                    */
/*    This is a re-write of the (much cleaner) UUXQT.C originally     */
/*    distributed with UUPC/Extended.  The modifications were         */
/*    intended primarily to lay a foundation for support for the      */
/*    more advanced features of UUX.                                  */
/*                                                                    */
/*    Richard H. Gumpertz (RHG@CPS.COM) built upon that foundation    */
/*    and added most of the code necessary for implementing UUXQT     */
/*    correctly, but there may still be many minor problems.          */
/*                                                                    */
/*    Usage:      uuxqt -xDEBUG -sSYSTEM                              */
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
 *    $Id: uuxqt.c 1.59 1997/11/29 12:49:01 ahd v1-12u $
 *
 *    Revision history:
 *    $Log: uuxqt.c $
 *    Revision 1.59  1997/11/29 12:49:01  ahd
 *    Don't dereference NULL pointer if missing operands when
 *    parsing RMAIL operands before RMAIL invocation
 *
 *    Revision 1.58  1997/11/24 02:58:14  ahd
 *    Don't allow non-mail programs to call checkname(), which could use
 *    uninitialized local domain name.
 *
 *    Revision 1.57  1997/11/21 16:06:43  ahd
 *    Invert security check for non-local files to deny if
 *    check passes.
 *
 *    Revision 1.56  1997/05/11 04:28:26  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.55  1997/04/24 01:41:35  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.54  1996/01/01 21:34:38  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.53  1995/09/24 19:10:36  ahd
 *    Correct compiler warning
 *
 *    Revision 1.52  1995/09/11 00:20:45  ahd
 *    Use "--" on RMAIL commands to prevent destructive behavior from
 *    untrusted nodes, make loop to process overlength RMAIL lines
 *    simpler.
 *
 *    Revision 1.51  1995/07/21 13:18:16  ahd
 *    Correct scope of loop for rmail deliveries
 *
 *    Revision 1.50  1995/03/12 16:42:24  ahd
 *    Don't pass NULL pointers to file delete routines, they get annoyed
 *
 *    Revision 1.49  1995/03/11 22:31:15  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.48  1995/03/08 03:01:54  ahd
 *    Always pass debuglevel to rnews under OS/2 to avoid C/Set++
 *    bug with spawns for with no program args
 *
 *    Revision 1.47  1995/02/25 18:21:44  ahd
 *    Don't require MAILSERV to be defined
 *
 *    Revision 1.46  1995/02/22 02:31:30  ahd
 *    Don't redirect output unless asked to by remote
 *
 *    Revision 1.45  1995/02/20 18:54:08  ahd
 *    news panic support
 *
 *    Revision 1.44  1995/02/20 17:28:43  ahd
 *    in-memory file support, 16 bit compiler clean up
 *
 *    Revision 1.43  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.42  1995/02/12 23:37:04  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.41  1995/01/07 16:42:05  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.40  1994/12/22 00:45:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.39  1994/10/23 23:29:44  ahd
 *    Be consistent in use of flags to execute() for foreground
 *    control.
 *
 * Revision 1.38  1994/10/03  01:01:25  ahd
 * Run programs foreground (drop silly OS/2 hack)
 *
 * Revision 1.37  1994/08/07  21:45:09  ahd
 * Correct selected changes in host title
 *
 * Revision 1.36  1994/03/05  21:12:05  ahd
 * Use standard name for NUL device
 *
 * Revision 1.35  1994/02/26  17:21:25  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 * Revision 1.34  1994/02/19  05:16:25  ahd
 * Use standard first header
 *
 * Revision 1.33  1994/01/06  12:45:33  ahd
 * Correct error message via mail when file is rejected
 *
 * Revision 1.32  1994/01/01  19:28:03  ahd
 * Annual Copyright Update
 *
 * Revision 1.31  1993/12/23  03:17:55  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.30  1993/12/23  01:52:33  ahd
 * Security enhancements
 *
 * Revision 1.29  1993/12/06  23:12:15  ahd
 * Use pair of buffers for setting requestor environment variable
 *
 * Revision 1.28  1993/12/06  01:59:07  ahd
 * Delete all unneeded environment variable resets to reduce
 * environment mangling
 *
 * Revision 1.27  1993/11/13  17:43:26  ahd
 * Update command line limit of RMAIL for 32 bit operating systems
 *
 * Revision 1.26  1993/11/06  13:04:13  ahd
 * Don't use more than 8 characters in XQT directory name
 *
 * Revision 1.25  1993/10/31  19:04:03  ahd
 * Use special name for NUL under Windows NT
 *
 * Revision 1.24  1993/10/30  17:23:37  ahd
 * Drop extra tzset() call
 *
 * Revision 1.23  1993/10/30  17:19:50  rhg
 * Additional clean up for UUX support
 *
 * Revision 1.22  1993/10/24  21:51:14  ahd
 * Drop unmigrated changes
 *
 * Revision 1.21  1993/10/24  19:42:48  rhg
 * Generalized support for UUX'ed commands
 *
 * Revision 1.20  1993/10/24  12:48:56  ahd
 * Trap RNEWS bad return code
 *
 * Revision 1.19  1993/10/12  01:34:47  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.18  1993/10/03  22:10:24  ahd
 * Use signed for length of parameters
 *
 * Revision 1.17  1993/10/03  20:43:08  ahd
 * Normalize comments to C++ double slash
 *
 * Revision 1.16  1993/09/29  23:29:56  ahd
 * Add xqtrootdir for UUXQT
 *
 * Revision 1.15  1993/09/20  04:48:25  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2)
 *
 * Revision 1.14  1993/08/03  03:35:58  ahd
 * Correct path pointer to initialized variable
 *
 * Revision 1.13  1993/08/03  03:11:49  ahd
 * Initialize buffer for shell() in non-Windows environment
 *
 * Revision 1.12  1993/07/31  16:27:49  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.11  1993/07/24  03:40:55  ahd
 * Agressively trap carriage returns at ends of lines (from X.* files
 * being edited by elves with DOS editors!)
 *
 * Revision 1.10  1993/07/20  21:45:37  ahd
 * Don't delete file after -2 abort from UUXQT
 *
 * Revision 1.9  1993/06/26  16:01:48  ahd
 * Normalize white space used to parse strings
 *
 * Revision 1.8  1993/06/13  14:06:00  ahd
 * Correct off-by-one error in RMAIL arg parse loop which crashed UUXQT
 * on long system names
 *
 * Revision 1.7  1993/04/11  00:35:46  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.6  1993/04/05  04:35:40  ahd
 * Use timestamp/file size information returned by directory search
 *
 * Revision 1.5  1992/11/25  12:59:17  ahd
 * Change NUL to /dev/nul to prevent ImportPath() mangling.
 *
 * Revision 1.4  1992/11/23  03:56:06  ahd
 * Selected fixes for use of generlized uux commands
 *
 * Revision 1.3  1992/11/19  03:03:33  ahd
 * drop rcsid
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <process.h>
#include <sys/stat.h>
#include <title.h>
#include <io.h>

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dater.h"
#include "execute.h"
#include "expath.h"
#include "getopt.h"
#include "getseq.h"
#include "hostable.h"
#include "import.h"
#include "lock.h"
#include "logger.h"
#include "pushpop.h"
#include "readnext.h"
#include "security.h"
#include "timestmp.h"
#include "usertabl.h"
#include "uundir.h"

#ifdef _Windows
#include "winutil.h"
#endif

currentfile();

/*--------------------------------------------------------------------*/
/*                      Execution flag defines                        */
/*--------------------------------------------------------------------*/

typedef enum {
        X_OUTPUT,     /* 'O' return output to "outnode"       */
        X_FAILED,     /* 'Z' send status if command failed    */
        X_SUCCESS,    /* 'n' send status if command succeeded */
        X_INPUT,      /* 'B' return command input on error    */
        X_USEEXEC,    /* 'E' process command using exec(2)    */
        X_STATFIL,    /* 'M' return status to file on remote  */

        S_CORRUPT,
        S_NOREAD,
        S_NOWRITE,

        F_CORRUPT,
        F_NOCHDIR,
        F_NOCOPY,
        F_BADF,

        F_SAVEFILES,   /* Temp RMAIL/RNEWS error, save files   */

        E_NORMAL,
        E_NOACC,
        E_SIGNAL,
        E_STATUS,
        E_NOEXE,
        E_FAILED,

        UU_LAST

        } UU_FLAGS;

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void usage( void );

static KWBoolean copyLocal(const char *from, const char *to);

static KWBoolean do_uuxqt( const char *sysname );

static void process( const char *fname,
                     const char *remote,
                     const char *executeDirectory);

static void create_environment(const char *requestor);

static void appendData( const char *input, FILE* dataout);

static void do_copy(   const char *localfile,
                       const char *rmtsystem,
                       const char *remotefile,
                       const char *requestor,
                       const KWBoolean success );

static void ReportResults(const int   status,
                          const char *input,
                                char *output,
                          const char *command,
                          const char *job_id,
                          const time_t jtime,
                          const char *requestor,
                          const char *outnode,
                          const char *rmtOutName,
                          const KWBoolean xflag[],
                          const char *statfil,
                          const char *machine,
                          const char *user);

static int shell(char *command,
                 const char *inname,
                 const char *rmtOutName,
                 const char *remoteName,
                 KWBoolean xflag[]);

static void mailStatus(const char *tempfile,
                       const char *address,
                       const char *subject);

static void purify( const char *where );

/*--------------------------------------------------------------------*/
/*       m a i n                                                      */
/*                                                                    */
/*       Main program                                                 */
/*--------------------------------------------------------------------*/

main( int argc, char **argv)
{
   int c;
   extern char *optarg;
   extern int   optind;
   char *sysname = "all";
   char *logname = NULL;

/*--------------------------------------------------------------------*/
/*     Report our version number and date/time compiled               */
/*--------------------------------------------------------------------*/

   debuglevel = 1;
   banner( argv );

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

   if (!configure( B_UUXQT ))
      exit(1);   /* system configuration failed */

/*--------------------------------------------------------------------*/
/*                     Initialize logging file                        */
/*--------------------------------------------------------------------*/

   openlog( logname );

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

   while ((c = getopt(argc, argv, "l:s:x:")) !=  EOF)
      switch(c)
      {

      case 'l':
         openlog( optarg );
         break;

      case 's':
         sysname = optarg;
         break;

      case 'x':
         debuglevel = atoi( optarg );
         break;

      case '?':
         usage();
         exit(1);
         break;

      default:
         printmsg(0, "uuxqt - invalid option -%c", c);
         usage();
         exit(2);
         break;
      }

   if (optind != argc)
   {
      fputs("Extra parameter(s) at end.\n", stderr);
      usage();
      exit(2);
   }

/*--------------------------------------------------------------------*/
/*                  Switch to the spooling directory                  */
/*--------------------------------------------------------------------*/

   PushDir( E_spooldir );
   atexit( PopDir );

   checkuser( E_mailbox  );   /* Force User Table to initialize        */
   checkreal( E_nodename );   /* Force Host Table to initialize        */

   if (!LoadSecurity())
   {
      printmsg(0,"Unable to initialize security, see previous message");
      exit(2);
   } /* if (!LoadSecurity()) */

#if defined(_Windows)
   atexit( CloseEasyWin );               /* Auto-close EasyWin on exit  */
#endif

/*--------------------------------------------------------------------*/
/*                Set up search path for our programs                 */
/*--------------------------------------------------------------------*/

   if ( E_uuxqtpath != NULL )
   {
      char buf[4096];
      char *p;
      sprintf(buf,"PATH=%s", E_uuxqtpath);
      p = newstr(buf);

      if (putenv( p ))
      {
         printmsg(0,"Unable to set path %s", p);
         panic();
      } /* if (putenv( p )) */

   } /* if ( E_uuxqtpath != NULL ) */

/*--------------------------------------------------------------------*/
/*                  Define the current user as UUCP                   */
/*--------------------------------------------------------------------*/

   if ( putenv( "LOGNAME=uucp" ))
   {
      printmsg(0,"Unable to set current user to UUCP");
      panic();
   }

/*--------------------------------------------------------------------*/
/*    Actually invoke the processing routine for the eXecute files    */
/*--------------------------------------------------------------------*/

   do_uuxqt( sysname );

   if( equal( sysname , "all" ) )
       do_uuxqt( E_nodename );

   return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*    d o _ u u x q t                                                 */
/*                                                                    */
/*    Processing incoming eXecute (X.*) files for a remote system     */
/*--------------------------------------------------------------------*/

static KWBoolean do_uuxqt( const char *sysname )
{
   struct HostTable *hostp;
   static char uu_machine[] = UU_MACHINE "=";
   char hostenv[sizeof uu_machine + 25 + 2];
   char executeDirectory[FILENAME_MAX];
   char *pattern;

/*--------------------------------------------------------------------*/
/*                 Determine if we have a valid host                  */
/*--------------------------------------------------------------------*/

   if( !equal( sysname , "all" ) )
   {
      if (equal( sysname , E_nodename ))
          hostp = checkself( sysname );
      else
          hostp = checkreal( sysname );

      if (hostp  ==  BADHOST)
      {
         printmsg(0, "Unknown host %s.", sysname );
         exit(1);
      }

   }
   else
        hostp = nexthost( KWTrue );

/*--------------------------------------------------------------------*/
/*                Define mask for execution directory                 */
/*--------------------------------------------------------------------*/

   if (( E_xqtRootDir == NULL ) || equali( E_xqtRootDir, E_spooldir ))
      sprintf( executeDirectory, "%s/%%.8s/XQT", E_spooldir );
                                 /* Nice parallel construction        */
   else
      sprintf( executeDirectory, "%s/XQT/%%.8s", E_xqtRootDir);
                                 /* Fewer directories than if we      */
                                 /* use the spool version              */

   pattern = newstr( executeDirectory );  /* Save pattern for posterity  */

/*--------------------------------------------------------------------*/
/*             Outer loop for processing different hosts              */
/*--------------------------------------------------------------------*/

   while  (hostp != BADHOST)
   {
      char fname[FILENAME_MAX];
      KWBoolean locked = KWFalse;

      setTitle( "Processing host %s", hostp->hostname );

/*--------------------------------------------------------------------*/
/*                Initialize security for this remote                 */
/*--------------------------------------------------------------------*/

      if ( (securep = GetSecurity( hostp )) == NULL )
         printmsg(0,"No security defined for %s,"
                  " cannot process X.* files",
                  hostp->hostname );
      else {

/*--------------------------------------------------------------------*/
/*              Set up environment for the machine name               */
/*--------------------------------------------------------------------*/

         sprintf(hostenv,"%s%.25s", uu_machine, hostp->hostname);

         if (putenv( hostenv ))
         {
            printmsg(0,"Unable to set environment %s",hostenv);
            panic();
         }
         else
            printmsg(6,"Set environment string %s", hostenv );

         sprintf(executeDirectory , pattern, hostp->hostname );
         printmsg(5,"Execute directory is %s", executeDirectory );

/*--------------------------------------------------------------------*/
/*           Inner loop for processing files from one host            */
/*--------------------------------------------------------------------*/

         while (readnext(fname, hostp->hostname, "X", NULL, NULL, NULL) )
         {
            if ( locked || LockSystem( hostp->hostname , B_UUXQT ))
            {
               process( fname , hostp->hostname, executeDirectory );
               locked = KWTrue;
            }
            else
               break;               /* We didn't get the lock         */

         } /* while */

         if ( locked )
         {
            purify( executeDirectory );
                                    /* Clean up after last command
                                       for host                      */
            UnlockSystem();
         } /* if ( locked ) */

      } /* else if */

/*--------------------------------------------------------------------*/
/*    If processing all hosts, step to the next host in the queue     */
/*--------------------------------------------------------------------*/

      if( equal(sysname,"all") )
         hostp = nexthost( KWFalse );
      else
         hostp = BADHOST;

   } /*while nexthost*/

   return KWFalse;

} /* do_uuxqt */

/*--------------------------------------------------------------------*/
/*    p r o c e s s                                                   */
/*                                                                    */
/*    Process a single execute file                                   */
/*--------------------------------------------------------------------*/

static void process( const char *eXecFileName,
                     const char *remote,
                     const char *executeDirectory)
{
   char *command = NULL,
        *inputName = NULL,
        *outputName = NULL,
        *job_id = NULL;
   char hostfile[FILENAME_MAX];

   struct F_list *qPtr;

#if defined(BIT32ENV)
   char   line[2048];   /* New OS/2, Windows NT environments   */
#else
   char line[BUFSIZ];
#endif

   KWBoolean skip = KWFalse;
   KWBoolean reject = KWFalse;
   FILE *fxqt;
   int status = 0;      /* initialized ONLY to suppress compiler warning */

   char *outnode = NULL;
   char *rmtOutName = NULL;
   char *user = NULL;
   char *requestor = NULL;
   char *statfil = NULL;
   char *machine = NULL;

   struct F_list {
      struct F_list *next;
      char *spoolname;
      char *xqtname;
   } *F_list = NULL;

   KWBoolean xflag[UU_LAST - 1];
   time_t jtime = time(NULL);

   memset( &xflag, 0, sizeof xflag );

/*--------------------------------------------------------------------*/
/*                         Open the X.* file                          */
/*--------------------------------------------------------------------*/

   if ( (fxqt = FOPEN(eXecFileName, "r", IMAGE_MODE)) == NULL)
   {
      printerr(eXecFileName);
      return;
   }
   else
      printmsg(2, "processing %s", eXecFileName);

/*--------------------------------------------------------------------*/
/*                  Begin loop to read the X.* file                   */
/*--------------------------------------------------------------------*/

   while (!skip && (fgets(line, sizeof line, fxqt) != NULL))
   {
      char *cp;

      if ( (cp = strchr(line, '\n')) != NULL )
         *cp = '\0';

      printmsg(5, "Input read: %s", line);

/*--------------------------------------------------------------------*/
/*            Process the input line according to its type            */
/*--------------------------------------------------------------------*/

      switch (*line)
      {

      case '#':
         break;

/*--------------------------------------------------------------------*/
/*                  User which submitted the command                  */
/*--------------------------------------------------------------------*/

      case 'U':
         if ( (cp = strtok(line + 1, WHITESPACE)) == NULL )
         {
            printmsg(0,"No user on U line in %s", eXecFileName );
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }

         user = strdup(cp);
         checkref(user);
                                    /* Get the system name            */
         if ( (cp = strtok(NULL, WHITESPACE)) == NULL)
         {                          /* Did we get a string?           */
            printmsg(2,"No node on U line in %s", eXecFileName );
            cp = (char *) remote;
         }
         else if (!equal(cp,remote))
         {
            printmsg(2,"Node on U line in %s doesn't match remote",
                     eXecFileName );
            cp = (char * ) remote;
         };
         machine = newstr(cp);

         break;

/*--------------------------------------------------------------------*/
/*                       Input file for command                       */
/*--------------------------------------------------------------------*/

      case 'I':
         cp = strtok(line + 1, WHITESPACE );
         if ( cp == NULL )
         {
            printmsg(0,"No input file name on I line");
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }

         if (equaln(cp,"D.",2)
             && (strchr(cp, '/') == NULL)
             && (strchr(cp, '\\') == NULL))
         {
            char temp[FILENAME_MAX];

            importpath(temp, cp, remote);
            mkfilename(hostfile, E_spooldir, temp);
         }
         else
         {
            strcpy(hostfile, cp);
            expand_path(hostfile,
                        securep->pubdir ,
                        securep->pubdir,
                        NULL);

            if (!equal(remote, E_nodename))
               if (!ValidateFile( hostfile, ALLOW_READ))
               {
                  reject = xflag[S_NOREAD] = KWTrue;
                  break;
               }
         }

         inputName = strdup(hostfile);
         checkref(inputName);

         break;

/*--------------------------------------------------------------------*/
/*                      Output file for command                       */
/*--------------------------------------------------------------------*/

      case 'O':
         cp = strtok(line + 1, WHITESPACE);
         if ( cp == NULL )
         {
            printmsg(0,"No output file name on O line");
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }

         strcpy(hostfile, cp);

         cp = strtok(NULL, WHITESPACE);
         if ( cp != NULL && !equal(cp, E_nodename) )
         {                /* Did we get a string indicating ANOTHER host? */
            outnode = strdup(cp);
            checkref(outnode);
            checkreal(outnode);
         }
         else
         {
            expand_path(hostfile, securep->pubdir, securep->pubdir, NULL);

            if (!equal(remote, E_nodename) &&
                ! ValidateFile( hostfile, ALLOW_WRITE))
               /* Taylor/GNU uuxqt also rejects it if the output would be in
                  E_spooldir (to keep people from setting up phony requests).
                  I am not sure whether we want to do likewise.  --RHG */
               {
                  reject = xflag[S_NOWRITE] = KWTrue;
                  break;
               }
         }

         rmtOutName = strdup(hostfile);
         checkref(rmtOutName);
         xflag[X_OUTPUT] = KWTrue;  /* return output to "outnode"  */

         break;

/*--------------------------------------------------------------------*/
/*                         Command to execute                         */
/*--------------------------------------------------------------------*/

      case 'C':
         cp = line + 1 + strspn(line + 1, WHITESPACE);
         if ( *cp == '\0' )
         {
            printmsg(0,"No command name on C line");
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }
         command = strdup( cp );
         checkref(command);

         break;

/*--------------------------------------------------------------------*/
/*                      Job Id for status reporting                   */
/*--------------------------------------------------------------------*/

      case 'J':
         if ( (cp = strtok(line + 1, WHITESPACE)) == NULL )
         {
            printmsg(0,"No job id on J line in %s", eXecFileName );
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }

         job_id = strdup( cp );
         checkref( job_id );

         break;

/*--------------------------------------------------------------------*/
/*                 Check that a required file exists                  */
/*--------------------------------------------------------------------*/

      case 'F':
         cp = strtok(line + 1, WHITESPACE);
         if (cp == NULL)
         {
            printmsg(0,"Missing F parameter in %s, command rejected",
                       eXecFileName);
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }

         if (equaln(cp,"D.",2)
             && (strchr(cp,'/') == NULL)
             && (strchr(cp,'\\') == NULL))
         {
            char temp[FILENAME_MAX];

            importpath(temp, cp, remote);
            mkfilename(hostfile, E_spooldir, temp);

            if ( access( hostfile, 0 ))   /* Does the host file exist?   */
            {                             /* No --> Skip the file        */
               printmsg(0,"Missing file %s (%s) for %s, command skipped",
                        cp, hostfile, eXecFileName);
               skip = KWTrue;
               break;
            }

            qPtr = malloc(sizeof *F_list);
            checkref(qPtr);
            qPtr->next = F_list;
            F_list = qPtr;

            F_list->spoolname = strdup(hostfile);
            checkref(F_list->spoolname);

            F_list->xqtname = NULL;
         }
         else
         {
            printmsg(0,"Invalid F parameter in %s, command rejected",
                       eXecFileName);
            reject = xflag[F_BADF] = KWTrue;
            break;
         }

         cp = strtok(NULL, WHITESPACE);
         if (cp != NULL)
         {
            if (!ValidDOSName(cp, KWFalse))
            {  /* Illegal filename --> reject the whole request */
               printmsg(0,"Illegal file %s in %s, command rejected",
                          cp,
                          eXecFileName);
               reject = xflag[F_BADF] = KWTrue;
               break;
            }
            else
            {
               mkfilename(hostfile, executeDirectory, cp);
               F_list->xqtname = strdup(hostfile);
               checkref(F_list->xqtname);
            }
         }

         break;

/*--------------------------------------------------------------------*/
/*             Requestor name (overrides user name, above)            */
/*--------------------------------------------------------------------*/

      case 'R':
         if ( (cp = strtok(line + 1, WHITESPACE)) == NULL )
         {
            printmsg(0,"No requestor on R line in %s", eXecFileName );
            reject = xflag[F_CORRUPT] = KWTrue;
            break;
         }

         requestor = strdup(cp);
         checkref(requestor);

         break;

/*--------------------------------------------------------------------*/
/*        Status file name to return info to on remote node           */
/*--------------------------------------------------------------------*/

      case 'M':
         if ( (cp = strtok(line + 1, WHITESPACE)) == NULL )
         {
            printmsg(0,"No file name on M line in %s", eXecFileName);
            break;
         }

         statfil = strdup(cp);
         checkref(statfil);
         xflag[X_STATFIL] = KWTrue;   /* return status to remote file  */

         break;

/*--------------------------------------------------------------------*/
/*                            Flag fields                             */
/*--------------------------------------------------------------------*/

      case 'Z': xflag[X_FAILED] = KWTrue;  /* send status if command failed  */
         break;

      case 'N': xflag[X_FAILED] = KWFalse;  /* send NO status if command failed */
         break;

      case 'n': xflag[X_SUCCESS] = KWTrue;  /* send status if command succeeded */
         break;

      case 'z': xflag[X_SUCCESS] = KWFalse; /* NO status if command succeeded */
         break;

      case 'B': xflag[X_INPUT] = KWTrue;   /* return command input on error  */
         break;

      case 'e': xflag[X_USEEXEC] = KWFalse; /* process command using sh(1)   */
         break;

      case 'E': xflag[X_USEEXEC] = KWTrue;  /* process command using exec(2)  */
         break;

/*--------------------------------------------------------------------*/
/*                    Quietly ignore unknown fields                   */
/*--------------------------------------------------------------------*/

      default :
         break;

      } /* switch */

   } /* while (!skip & (fgets(line, sizeof line, fxqt) != NULL)) */

   if ( fxqt != NULL )
      fclose(fxqt);

   if ((command == NULL) && !skip)
   {
      printmsg(0,"No command supplied for X.* file %s, rejected",
                 eXecFileName);
      reject = xflag[F_CORRUPT] = KWTrue;
   }

/*--------------------------------------------------------------------*/
/*           We have the data for this command; process it            */
/*--------------------------------------------------------------------*/

   if ( !skip )
   {
      if ( user == NULL )
      {
         user = strdup("uucp"); /* User if none given              */
         checkref(user);
      }

      if ( machine == NULL )
      {
         machine = strdup( remote );
         checkref( machine );
      }

      if (requestor == NULL)
      {
         requestor = strdup(user);
         checkref(requestor);
      }

/*--------------------------------------------------------------------*/
/*       Taylor/GNU uuxqt seems to check for both READ and WRITE      */
/*       access if it starts with a '/' and also rejects most (but    */
/*       not all) names containing "..".                              */
/*                                                                    */
/*       Also, what quoting conventions should we follow when         */
/*       parsing?  The DOS COMMAND.COM certainly does not handle      */
/*       \-style quoting, for example, so we don't want to get        */
/*       fooled by handling such here.                                */
/*--------------------------------------------------------------------*/

      if ( ! reject &&
#ifdef UUCP_SECURE
           ! equalni( command,"uucp ", 5) && /* Known to be secure   */
#endif
           ! equalni( command,"rmail ", 6))  /* Known to be secure   */
      {
         char *next;

         strcpy( line, command );
         strtok( line, WHITESPACE);       /* Discard command name    */

         next = strtok( NULL, "");        /* Get rest of string      */

         while( next && ! reject )
         {
            char *token = strtok( next, WHITESPACE "'\"" );
            next = strtok( NULL, "");     /* Get rest of string      */

            strncpy( hostfile, token, sizeof hostfile);

            if ((strlen( token ) > (sizeof hostfile -1)) || /* Too long ?  */
                 (strstr( token ,".." ) != NULL ) ||     /* Parent games?  */
                 (expand_path(hostfile,
                              executeDirectory,
                              securep->pubdir,
                              NULL) == NULL ))  /* Can't expand path?   */
               reject = xflag[E_NOACC] = KWTrue;
                                 /* Cannot determine true file name
                                    easily, reject it.               */
            else if (!equalni( executeDirectory,
                              hostfile,
                              strlen( executeDirectory )))
            {

               if (((strchr( token, ':' ) != NULL ) ||   /* drive letter?  */
                 (strchr( token, '/' ) != NULL ) ||      /* path sep?      */
                 (strchr( token, '\\') != NULL )) &&     /* path sep?      */
                 (!ValidateFile( hostfile, ALLOW_WRITE ) ||
                  !ValidateFile( hostfile, ALLOW_READ  )))
               reject = xflag[E_NOACC] = KWTrue;
            }

         } /* while( next && ! reject ) */

      } /* if ( ! reject && !equaln( command,"rmail ", 6)) */

      if ( !reject )
      {

         if (inputName == NULL)
            inputName = strdup(BIT_BUCKET);

         if (xflag[X_OUTPUT])
            outputName = mktempname(NULL, "tmp");

         printmsg(equaln(command,RMAIL,5) ? 2 : 0,
                     "uuxqt: executing \"%s\" for user %s at %s",
                      command,
                      user,
                      machine);

/*--------------------------------------------------------------------*/
/*           Copy the input files to the execution directory          */
/*--------------------------------------------------------------------*/

         /* Make sure the directory exists before we copy the files */

         PushDir(executeDirectory);

#ifdef RECURSIVE_PURIFY

/*--------------------------------------------------------------------*/
/*       This needs to be enabled after we allow multiple directory   */
/*       searches at once.                                            */
/*--------------------------------------------------------------------*/

         purify( executeDirectory );

#endif

         for (qPtr = F_list; qPtr != NULL; qPtr = qPtr->next)
         {
            if (qPtr->xqtname != NULL)

               if (!copyLocal(qPtr->spoolname, qPtr->xqtname))
               {
                  /* Should we try again later in case its a temporary
                     error like execute directory on a full disk?  For
                     now, just reject it completely. */

                  printmsg(0, "Copy %s to %s failed",
                              qPtr->spoolname, qPtr->xqtname);
                  reject = xflag[F_NOCOPY] = KWTrue;
                  break;

               }
         } /* for ( ;; ) */

/*--------------------------------------------------------------------*/
/*            Create the environment and run the command(s)           */
/*--------------------------------------------------------------------*/

         if (!reject)
         {
            char *pipe;
            char *cmd = strdup(command);        /* shell clobbers its arg */
            checkref(cmd);

            create_environment( requestor);  /* Set requestor id     */

/*--------------------------------------------------------------------*/
/*       The following code INTENTIONALLY ignores quoting of '|'      */
/*       (with \ or "..." or '...') because we can't count on the     */
/*       DOS shell (or spawnlp, etc.) to also handle quoting of '|'   */
/*       the exact same way (or even at all).  We also prohibit '<'   */
/*       and '>' for similar reasons.  (By the way, uux should have   */
/*       changed I/O redirecting '<' and '>' to I and O lines,        */
/*       respectively.)                                               */
/*--------------------------------------------------------------------*/

            if (strchr(cmd, '<') != NULL || strchr(cmd, '>') != NULL)
            {
               printmsg(0,"The characters \'<\' and \'>\' are not supported in remote commands");
               reject = xflag[F_CORRUPT] = KWTrue;
            }
            else if ((pipe = strchr(cmd, '|')) == NULL) /* Any pipes? */
               status = shell(cmd, inputName, outputName, remote, xflag); /* No */
            else
            { /* We currently do pipes by simulating them using files */
               char *pipefile = mktempname(NULL, "pip");
               char *next_cmd = cmd; /* initialized ONLY to suppress compiler warning */

               *pipe = '\0';
               status = shell(cmd, inputName, pipefile, remote, xflag);
               /* *pipe = '|'; */

               while (status == 0
                      && (pipe = strchr(next_cmd = pipe + 1, '|')) != NULL)
               {
                  /* Swap the output and pipe files for the next pass */
                  char *p = pipefile;
                  pipefile = outputName;
                  outputName = p;

                  xflag[E_NORMAL] = KWFalse;
                  *pipe = '\0';
                  status = shell(next_cmd, outputName, pipefile, remote, xflag);
                  /* *pipe = '|'; */
               }

               if (status == 0)
               {
                  xflag[E_NORMAL] = KWFalse;
                  status = shell(next_cmd, pipefile, outputName, remote, xflag);
               }

               if (REMOVE(pipefile))
                  printerr( pipefile );

               free(pipefile);
            }

            free(cmd);
         }

/*--------------------------------------------------------------------*/
/*                  Clean up files after the command                  */
/*--------------------------------------------------------------------*/

         PopDir();

         for (qPtr = F_list; qPtr != NULL; qPtr = qPtr->next)
            if ((qPtr->xqtname != NULL) && REMOVE(qPtr->xqtname))
               printerr( qPtr->xqtname);

      } /* if (!reject) */

      if ( ! xflag[F_SAVEFILES] )
      {
         for (qPtr = F_list; qPtr != NULL; qPtr = qPtr->next)
         {
            if ( REMOVE(qPtr->spoolname) )
               printerr( qPtr->spoolname );
         }
      }

      ReportResults( status,
                     inputName,
                     outputName,
                     command,
                     job_id,
                     jtime,
                     requestor,
                     outnode,
                     rmtOutName,
                     xflag,
                     statfil,
                     machine,
                     user);

      if ((!reject) && (outputName != NULL ) && REMOVE(outputName))
         printerr( outputName );

      if (! xflag[F_SAVEFILES] && REMOVE(eXecFileName))
         printerr( eXecFileName );

   } /* (!skip) */

/*--------------------------------------------------------------------*/
/*              Free various temporary character strings              */
/*--------------------------------------------------------------------*/

   while (F_list != NULL)
   {
      free(F_list->spoolname);

      if (F_list->xqtname != NULL)
         free(F_list->xqtname);

      qPtr = F_list->next;
      free(F_list);
      F_list = qPtr;

   }  /* while (F_list != NULL) */

   if (command    != NULL) free(command);
   if (inputName  != NULL) free(inputName);
   if (job_id     != NULL) free(job_id);
   if (outnode    != NULL) free(outnode);
   if (outputName != NULL) free(outputName);
   if (requestor  != NULL) free(requestor);
   if (statfil    != NULL) free(statfil);
   if (user       != NULL) free(user);

} /* process */

/*--------------------------------------------------------------------*/
/*    s h e l l                                                       */
/*                                                                    */
/*    Simulate a Unix command                                         */
/*--------------------------------------------------------------------*/

static int shell(char *command,
                 const char *inname,
                 const char *outputName,
                 const char *remoteName,
                 KWBoolean xflag[])
{
   int    result = 0;

#if defined(BIT32ENV)
   char   commandBuf[1024];   /* New OS/2, Windows NT environments   */
#elif defined(FAMILYAPI)
   char   commandBuf[255];    /* Original OS/2 1.x environment       */
#else
   char   commandBuf[128];    /* Original DOS environment            */
#endif

   char   *cmdname;
   char   *parameters;

   static char missing_address[] = "address-missing-on-rmail-command-line";

   if (xflag[X_USEEXEC])
      printmsg(2, "exec(2) not supported, executing using spawn");

/*--------------------------------------------------------------------*/
/*         Determine the command name and parameters, if any          */
/*--------------------------------------------------------------------*/

   cmdname = strtok( command, WHITESPACE );
   parameters = strtok( NULL, "\r\n" );

   if ( parameters != NULL )
   {
      parameters += strspn(parameters, WHITESPACE); /* drop leading whitespace */

      if ( *parameters == '\0' )
         parameters = NULL;

   } /* if ( parameters != NULL ) */

/*--------------------------------------------------------------------*/
/*    Verify we support the command, and get it's real name, if so    */
/*--------------------------------------------------------------------*/

   if ( (!equal(remoteName, E_nodename)) && (!ValidateCommand( cmdname )) )
   {
      printmsg(0,"Command %s not allowed at this site", cmdname);
      xflag[E_NOEXE] = KWTrue;
      return 99;
   }

   setTitle( "Processing host %.10s, command %.10s",
               remoteName,
               cmdname );

/*--------------------------------------------------------------------*/
/*               We support the command; execute it                   */
/*--------------------------------------------------------------------*/

   fflush(logfile);

/*--------------------------------------------------------------------*/
/*               RNEWS may be special, handle it if so                */
/*--------------------------------------------------------------------*/

#if defined(__OS2__)

/*--------------------------------------------------------------------*/
/*       To help debugging and workaround an OS/2 bug which causes    */
/*       spawnl() to generate bogus arguments when none are           */
/*       passed, we generate a debug flag under OS/2.  This will      */
/*       work with any version of UUPC/extended rnews, but not        */
/*       third party packages.                                        */
/*--------------------------------------------------------------------*/

   if (equal(cmdname,RNEWS) && (parameters == NULL))
   {
      sprintf( commandBuf, "-x %d" , debuglevel );
      parameters = commandBuf;
   }

   if (equali(cmdname, "newsrun") && (parameters == NULL))
   {
      sprintf( commandBuf, "-x %d" , debuglevel );
      parameters = commandBuf;
   }

#endif

   if (equal(cmdname,RNEWS) &&
       bflag[F_WINDOWS] &&
       ( inname != NULL ))       /* rnews w/input?                    */
   {
      strcpy( commandBuf, "-f " );
      strcat( commandBuf, inname );
      parameters = commandBuf;   /* We explicitly ignore all parameters  */
                                 /* on the RNEWS command              */

      result = execute( RNEWS,
                        commandBuf,
                        NULL,
                        outputName,
                        KWTrue,
                        KWFalse );
   }

/*--------------------------------------------------------------------*/
/*        RMAIL is special, we need to break up the parameters        */
/*--------------------------------------------------------------------*/

   else if (equal(cmdname,RMAIL) && ( inname != NULL )) /* rmail w/input?  */
   {
      /* rmail with no parameters is an remote error, we force it
         into an invalid address which causes a bounce message
         and prevents the tokenizing code below from crashing        */

      if ( parameters == NULL )
         parameters = missing_address;

      for ( ;; )
      {

         size_t parametersLength = strlen( parameters );
         size_t lastCharacter    = sizeof commandBuf - 2;

#if defined(_Windows)

         if ( bflag[F_WINDOWS] )
         {
            strcpy( commandBuf, "-f ");
            strcat( commandBuf, inname);
            strcat( commandBuf, " ");
         }
         else
            *commandBuf = '\0';

/*--------------------------------------------------------------------*/
/*       To help debugging and workaround an OS/2 bug which causes    */
/*       spawnl() to generate bogus arguments when none are           */
/*       passed, we generate a debug flag under OS/2.  This will      */
/*       work with any version of UUPC/extended rmail, but not        */
/*       third party packages.                                        */
/*--------------------------------------------------------------------*/

#elif defined(__OS2__) || defined( BIT32ENV )

         sprintf( commandBuf, "-x %d " , debuglevel );

#else

         *commandBuf = '\0';

#endif

         /* The local node is used for SMTP queuing support */
         if ( equal( remoteName, E_nodename ))
            strcat( commandBuf, "-q " );

         if ( *parameters == '-' )        /* Funny user id or
                                             funnier options?     */
            strcat( commandBuf, "-- " );  /* Treat as addresses   */

         lastCharacter -= strlen( commandBuf ) + strlen( RMAIL ) + 1;

/*--------------------------------------------------------------------*/
/*               Determine longest address we can copy                */
/*--------------------------------------------------------------------*/

         if ( lastCharacter > parametersLength )
            lastCharacter = parametersLength;
         else {

            while( lastCharacter && ! isspace( parameters[lastCharacter] ))
               lastCharacter--;
         }

/*--------------------------------------------------------------------*/
/*                Verify we found a command to process                */
/*--------------------------------------------------------------------*/

         if ( ! lastCharacter )
         {
            printmsg(0,
                     "Address \"%s\" too long (%d chars)!",
                      parameters,
                      parametersLength );

            panic();

         } /* if ( ! lastCharacter ) */

/*--------------------------------------------------------------------*/
/*                 Create the line of addresses                       */
/*--------------------------------------------------------------------*/

         parameters[lastCharacter] = '\0';
         strcat( commandBuf, parameters );

/*--------------------------------------------------------------------*/
/*               Execute one command line of addresses                */
/*--------------------------------------------------------------------*/

         result = execute( RMAIL,
                           commandBuf,
                           bflag[F_WINDOWS] ? NULL : inname,
                           outputName,
                           KWTrue,
                           KWFalse );

         if ( result != 0 )    /* Did command execution fail?            */
         {
            printmsg(0,"shell: command \"%s %s\" returned error code %d",
                  cmdname, commandBuf, result);

            if ( result == EX_TEMPFAIL )
            {
               xflag[F_SAVEFILES] = KWTrue;
               return result;
            }
            else {
               panic();
            }
         }

/*--------------------------------------------------------------------*/
/*           Locate the beginning of next addresses, if any           */
/*--------------------------------------------------------------------*/

         if ( lastCharacter == parametersLength )
            break;
         else {
            parameters += lastCharacter + 1;

            parameters += strspn(parameters, WHITESPACE);
                                    /* Drop leading whitespace       */

            if ( *parameters == '\0' )
               break;

         } /* else */

      } /* for ( ;; ) */

   } /* if (equal(cmdname,RMAIL) && ( inname != NULL )) */
   else
      result = execute( cmdname,
                        parameters,
                        inname,
                        outputName,
                        KWTrue,
                        KWFalse );

/*--------------------------------------------------------------------*/
/*                    Determine result of command                     */
/*--------------------------------------------------------------------*/

   if ( result == 0 )
      xflag[E_NORMAL] = KWTrue;
   else if ( (equali(cmdname, RNEWS) ||
              equali(cmdname, "newsrun")) &&
             bflag[F_NEWSPANIC] )
                           /* Did command execution fail?            */
   {
      printmsg(0,"shell: command %s returned error code %d",
            cmdname, result);

      if ( result == EX_TEMPFAIL )
         xflag[F_SAVEFILES]= KWTrue;
      else
         panic();
   }
   else if ( result > 0 )
      xflag[E_STATUS] = KWTrue;

   return result;

} /* shell */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report how to run this program                                  */
/*--------------------------------------------------------------------*/

static void usage( void )
{
   fputs("Usage: uuxqt [-xDEBUG] [-sSYSTEM]", stderr);
   exit(1);
} /* usage */

/*--------------------------------------------------------------------*/
/*    c o p y l o c a l                                               */
/*                                                                    */
/*    Copy Local Files                                                */
/*--------------------------------------------------------------------*/

static KWBoolean copyLocal(const char *from, const char *to)
{
      int  fd_from, fd_to;
      int  nr;
      int  nw = -1;
      char buf[BUFSIZ*2];        /* faster if we alloc a big buffer  */

      /* This would be even faster if we determined that both files
         were on the same device, dos >= 3.0, and used the dos move
         function EXCEPT that we want a COPY, not a MOVE! */

      if ((fd_from = open(from, O_RDONLY | O_BINARY)) == -1)
         return KWFalse;       /* failed                                */

      /* what if the to is a directory? */
      /* possible with local source & dest uucp */

      if ((fd_to = open(to, O_CREAT | O_BINARY | O_WRONLY, S_IWRITE | S_IREAD)) == -1)
      {
         close(fd_from);
         return KWFalse;       /* failed                                */

         /* NOTE - this assumes all the required directories exist!  */
      }

      while ((nr = read(fd_from, buf, sizeof buf)) > 0)
      {
         nw = write(fd_to, buf, (unsigned) nr);

         if (nw < nr)
         {
            nw = -1;
            break;
         }

      } /* while */

      close(fd_to);
      close(fd_from);

      if ((nr != 0) || (nw == -1))
         return KWFalse;       /* failed in copy                       */

      return KWTrue;

} /* copyLocal */

/*--------------------------------------------------------------------*/
/*    c r e a t e _ e n v i r o n m e n t                             */
/*                                                                    */
/*    Create the environment array for subprocesses                   */
/*--------------------------------------------------------------------*/

static void create_environment(const char *requestor)
{
   static char buffera[MAXADDR + 20];
   static char bufferb[MAXADDR + 20];
   static char *buffer = buffera;

   int subscript = 0;
   char *envp[3];

/*--------------------------------------------------------------------*/
/*               user id/nodename of original requestor               */
/*--------------------------------------------------------------------*/

   if ( buffer == buffera )
      buffer = bufferb;
   else
      buffer = buffera;

   if ( requestor == NULL )
      panic();
   else if ( ! strlen( requestor ))
      sprintf( buffer, "%s=%s %s", UU_USER, "uucp", E_nodename );
   else
      sprintf( buffer, "%s=%s", UU_USER, requestor);

   envp[subscript++] =  buffer;

/*--------------------------------------------------------------------*/
/*               Now put the data into our environment                */
/*--------------------------------------------------------------------*/

   envp[subscript] =  NULL;   /* Terminate the list                    */

   while( subscript-- > 0)
   {
      if (putenv( envp[subscript] ))
      {
         printmsg(0,"Unable to set environment %s",envp[subscript]);
         panic();
      }
      else
         printmsg(6,"Set environment string %s", envp[subscript] );

   } /* while */

} /* create_environment */

/*--------------------------------------------------------------------*/
/*    d o _ c o p y                                                   */
/*                                                                    */
/*    Send a file to remote node via uucp                             */
/*--------------------------------------------------------------------*/

static void     do_copy(const char *localfile,
                       const char *rmtsystem,
                       const char *remotefile,
                       const char *requestor,
                       const KWBoolean success )
{
      if (rmtsystem == NULL)
      {
          copyLocal(localfile, remotefile);
      }
      else {

          char commandOptions[FILENAME_MAX * 2];
          int status;

          sprintf( commandOptions,
                   "-C %s -a%s %s %s!%s",
                   (char *) (success ? "-n" : ""),
                   requestor,
                   localfile,
                   rmtsystem,
                   remotefile );

/*--------------------------------------------------------------------*/
/*       We don't have a good response to UUCP failing, so we         */
/*       pretty much ignore it if it fails                            */
/*--------------------------------------------------------------------*/

          status = execute( "UUCP",
                            commandOptions,
                            NULL,
                            NULL,
                            KWTrue,
                            KWFalse );

         if ( status > 0 )
               printmsg(0,"Command %s %s failed, status = %d",
                           "UUCP",
                           commandOptions,
                           status );

    };

} /* do_copy */

/*--------------------------------------------------------------------*/
/*    R e p o r t R e s u l t s                                       */
/*                                                                    */
/*    report results of command execution as specified by flags in    */
/*    X.* file.                                                       */
/*--------------------------------------------------------------------*/

static void ReportResults(const int status,
                          const char *input,
                                char *outputName,
                          const char *command,
                          const char *job_id,
                          const time_t jtime,
                          const char *requestor,
                          const char *outnode,
                          const char *rmtOutName,
                          const KWBoolean xflag[],
                          const char *statfil,
                          const char *machine,
                          const char *user)
{
   char addrBuf[MAXADDR];
   char address[MAXADDR];
   char subject[80];
   FILE *mailtmp = NULL;
   char *hisUser, *hisNode;
   char tempmail[FILENAME_MAX];

   if (!(xflag[X_FAILED] || xflag[X_SUCCESS] ||
         xflag[X_INPUT]  || xflag[X_STATFIL] ))
   {  /* default actions */
      return;
   }

#ifdef BETA_TEST
   strcpy(address,"postmaster");
#else

   if ( strlen(requestor) >= sizeof address )
   {
      printmsg(0, "ReportResults: Overlength address %s", requestor );
      panic();
   }

   strcpy( addrBuf, requestor );
   hisUser = strtok( addrBuf, WHITESPACE );
   hisNode = strtok( NULL,    WHITESPACE );

   if ( hisNode == NULL )
      hisNode = (char *) machine;

   if (equal(hisNode, E_nodename))
      strcpy(address, requestor);
   else {

      if ( equal( machine, hisNode ))
         sprintf(address,"%s!%s", hisNode, hisUser );
      else
         sprintf(address,"%s!%s!%s", machine, hisNode, hisUser );

      printmsg(4,"ReportResults: requestor %s, address %s",
                  requestor, address );

   } /* else */
#endif

   mktempname(tempmail, "tmp");

   if ((mailtmp = FOPEN(tempmail, "w+", IMAGE_MODE)) == NULL)
   {
      printerr(tempmail);
      return;
   }

   sprintf(subject, "\"[uucp job %s (%s)]\"", job_id, dater(jtime, NULL) );

   fprintf(mailtmp,"remote execution\n");
   fprintf(mailtmp,"%s\n", command);

   if (xflag[E_NORMAL])
   {                        /* command succeded, process appropriate flags */

      fprintf(mailtmp,"exited normally\n");

      if (xflag[X_OUTPUT])
         do_copy(outputName, outnode, rmtOutName, requestor, xflag[X_SUCCESS]);

      fclose(mailtmp);

      if (xflag[X_SUCCESS])
      {
         if (xflag[X_STATFIL])
         {
            do_copy(tempmail, outnode, statfil, requestor, xflag[X_SUCCESS]);
         }
         else {
            mailStatus(tempmail, address, subject);
         }
      }

   }
   else {            /* command failed, process appropriate flags   */
     if (xflag[F_CORRUPT])
        fprintf(mailtmp,"the X file was badly formatted\n");

     if (xflag[S_NOREAD])
        fprintf(mailtmp,"stdin was denied read permission\n");

     if (xflag[S_NOWRITE])
        fprintf(mailtmp,"stdout was denied write permission\n");

     if (xflag[F_NOCHDIR])
        fprintf(mailtmp,"unable to change directory to the execution directory\n");

     if (xflag[F_NOCOPY])
        fprintf(mailtmp,"unable to copy file(s) to the execution directory\n");

     if (xflag[F_BADF])
        fprintf(mailtmp,"invalid file name\n");

     if (xflag[E_NOACC])
         fprintf(mailtmp,"file access denied to %s!%s\n", machine, user);

     if (xflag[E_NOEXE])
        fprintf(mailtmp,"execution permission denied to %s!%s\n",
                machine, requestor);

     if (xflag[E_SIGNAL])
        fprintf(mailtmp,"terminated by signal\n");

     if (xflag[E_STATUS])
        fprintf(mailtmp,"exited with status %d\n", status);

     if (xflag[E_FAILED])
        fprintf(mailtmp,"failed completely\n");

     if (xflag[E_STATUS])
     {
        if (xflag[X_FAILED])
        {
           if (xflag[X_INPUT])
           {
              fprintf(mailtmp,"===== stdin was ");

              if (xflag[S_CORRUPT])
                  fprintf(mailtmp,"unreadable =====\n");
              else if (!xflag[S_NOREAD])
              {
                  fprintf(mailtmp,"=====\n");
                  appendData( input, mailtmp);
              };

           }

           fprintf(mailtmp,"===== stderr is unavailable =====\n");
        }
     }

     fclose(mailtmp);

     if (xflag[X_STATFIL])
     {
         do_copy(tempmail, outnode, statfil, requestor, xflag[X_SUCCESS]);
     }
     else {
         mailStatus(tempmail, address, subject);
     }

   }

   if (REMOVE(tempmail))
      printerr( tempmail );

   return;

} /* ReportResults */

/*--------------------------------------------------------------------*/
/*       a p p e n d D a t a                                          */
/*                                                                    */
/*       Append data to output file                                   */
/*--------------------------------------------------------------------*/

static void appendData( const char *input, FILE* dataout)
{
   FILE    *datain;
   char     buf[BUFSIZ];

/*--------------------------------------------------------------------*/
/*                      Verify the input opened                       */
/*--------------------------------------------------------------------*/

   if (input == NULL)
      return;
   else
      datain = FOPEN(input, "r",TEXT_MODE);

   if (datain == NULL)
   {
      printerr(input);
      return;
   } /* datain */

/*--------------------------------------------------------------------*/
/*                       Loop to copy the data                        */
/*--------------------------------------------------------------------*/

   while (fgets(buf, sizeof buf, datain) != 0)
   {
      if (fputs(buf, dataout) == EOF)     /* I/O error?               */
      {
         printmsg(0,"appendData: I/O error on output file");
         printerr("dataout");
         fclose(datain);
         return;
      } /* if */

   } /* while */

/*--------------------------------------------------------------------*/
/*                      Close up shop and return                      */
/*--------------------------------------------------------------------*/

   if (ferror(datain))        /* Clean end of file on input?           */
   {
      printerr(input);
      clearerr(datain);
   }

   fclose(datain);

} /* appendData */

/*--------------------------------------------------------------------*/
/*    M a i l S t a t u s                                             */
/*                                                                    */
/*    Send text in a mailbag file to address(es) specified by line.   */
/*--------------------------------------------------------------------*/

static
void mailStatus(const char *tempfile,
                const char *address,
                const char *subject)
{
   int status;
   char buf[BUFSIZ];

/*--------------------------------------------------------------------*/
/*                            Invoke RMAIL                            */
/*--------------------------------------------------------------------*/

   create_environment( "" );

   strcpy(buf, "-w -f " );
   strcat(buf, tempfile );
   if ( subject != NULL )
   {
      strcat(buf, " -s " );
      strcat(buf, subject );
   }
   strcat( buf, " -- " );
   strcat( buf, address );

   status = execute( RMAIL, buf, NULL, NULL, KWTrue, KWFalse );

/*--------------------------------------------------------------------*/
/*                       Report errors, if any                        */
/*--------------------------------------------------------------------*/

   if ( status < 0 )
   {
      printerr( RMAIL );
      printmsg(0,"Unable to execute rmail; status not delivered.");
   }
   else if ( status > 0 )
      printmsg(0, "Rmail returned error; "
                  "status delivery may be incomplete.");

} /* mailStatus */

/*--------------------------------------------------------------------*/
/*       p u r i f y                                                  */
/*                                                                    */
/*       Clean out a directory                                        */
/*--------------------------------------------------------------------*/

static void purify( const char *where )
{
   DIR *dirp = opendir( where );
   struct direct *dp;

/*--------------------------------------------------------------------*/
/*                     Open the directory to nuke                     */
/*--------------------------------------------------------------------*/

   if ( dirp == NULL )
      return;

/*--------------------------------------------------------------------*/
/*       Simple loop to delete all files left in the directory so     */
/*       it is safe for others to use.  Note that a subdirectory      */
/*       will cause the program abort.  For now, it's a known         */
/*       restriction.                                                 */
/*--------------------------------------------------------------------*/

   while ((dp = readdir(dirp)) != nil(struct direct))
   {
      char fname[FILENAME_MAX];

      sprintf(fname, "%s/%s", where, dp->d_name);

      printmsg(0,"purify: Deleting file %s", fname );

      if ( chmod( fname, S_IREAD | S_IWRITE ) || REMOVE( fname ))
      {
         printerr( fname );
         panic();
      }

   } /* while */

   closedir(dirp);

} /* purify */
