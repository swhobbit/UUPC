/*
      Program:    uuxqt.c              23 September 1991
      Author:     Mitch Mitchell
      Email:      mitch@harlie.lonestar.org

      This is a re-write of the (much cleaner) UUXQT.C originally
      distributed with UUPC/Extended.  The modifications were
      intended primarily to lay a foundation for support for the
      more advanced features of UUX.

      Richard H. Gumpertz (RHG@CPS.COM) built upon that foundation
      and added most of the code necessary for implementing UUXQT
      correctly, but there may still be many minor problems.

      Usage:      uuxqt -xDEBUG -sSYSTEM
*/

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
 *    $Id: uuxqt.c 1.27 1993/11/13 17:43:26 ahd Exp $
 *
 *    Revision history:
 *    $Log: uuxqt.c $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

#ifdef _Windows
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "arpadate.h"
#include "dater.h"
#include "expath.h"
#include "getopt.h"
#include "getseq.h"
#include "hlib.h"
#include "hostable.h"
#include "import.h"
#include "lock.h"
#include "logger.h"
#include "pushpop.h"
#include "readnext.h"
#include "security.h"
#include "timestmp.h"
#include "usertabl.h"
#include "execute.h"

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

        E_NORMAL,
        E_NOACC,
        E_SIGNAL,
        E_STATUS,
        E_NOEXE,
        E_FAILED,

        UU_LAST

        } UU_FLAGS;

/*--------------------------------------------------------------------*/
/*                          Global Variables                          */
/*--------------------------------------------------------------------*/

static char *spool_fmt = SPOOLFMT;
static char *dataf_fmt = DATAFFMT;

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void usage( void );

static boolean copylocal(const char *from, const char *to);

static boolean do_uuxqt( const char *sysname );

static void process( const char *fname,
                     const char *remote,
                     const char *executeDirectory);

static void create_environment(const char *requestor);

static boolean AppendData( const char *input, FILE* dataout);

static boolean do_copy( char *localfile,
                       const char *rmtsystem,
                       const char *remotefile,
                       const char *requestor,
                       const boolean success );

static void ReportResults(const int   status,
                          const char *input,
                                char *output,
                          const char *command,
                          const char *job_id,
                          const time_t jtime,
                          const char *requestor,
                          const char *outnode,
                          const char *outname,
                          const boolean xflag[],
                          const char *statfil,
                          const char *machine,
                          const char *user);

static int shell(char *command,
                 const char *inname,
                 const char *outname,
                 const char *remotename,
                 boolean xflag[]);

static boolean MailStatus(char *tempfile,
                          char *address,
                          char *subject);

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

void main( int argc, char **argv)
{
   int c;
   extern char *optarg;
   extern int   optind;
   char *sysname = "all";

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
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

   while ((c = getopt(argc, argv, "s:x:")) !=  EOF)
      switch(c) {

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

   if (optind != argc) {
      fputs("Extra parameter(s) at end.\n", stderr);
      usage();
      exit(2);
   }

/*--------------------------------------------------------------------*/
/*                             Initialize                             */
/*--------------------------------------------------------------------*/

   if (!configure( B_UUXQT ))
      exit(1);   /* system configuration failed */

/*--------------------------------------------------------------------*/
/*                  Switch to the spooling directory                  */
/*--------------------------------------------------------------------*/

   PushDir( E_spooldir );
   atexit( PopDir );

/*--------------------------------------------------------------------*/
/*                     Initialize logging file                        */
/*--------------------------------------------------------------------*/

   openlog( NULL );

   checkuser( E_mailbox  );   /* Force User Table to initialize        */
   checkreal( E_mailserv );   /* Force Host Table to initialize        */

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
      char buf[BUFSIZ];
      char *p;
      sprintf(buf,"PATH=%s", E_uuxqtpath);
      p = newstr(buf);

      if (putenv( p ))
      {
         printmsg(0,"Unable to set path \"%s\"", p);
         panic();
      } /* if (putenv( p )) */

   } /* if ( E_uuxqtpath != NULL ) */

/*--------------------------------------------------------------------*/
/*              Disable OS/2 undelete support if desired              */
/*--------------------------------------------------------------------*/

   if ( !bflag[ F_UNDELETE ] )
      putenv( "DELDIR=");

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

   exit(0);

} /* main */

/*--------------------------------------------------------------------*/
/*    d o _ u u x q t                                                 */
/*                                                                    */
/*    Processing incoming eXecute (X.*) files for a remote system     */
/*--------------------------------------------------------------------*/

static boolean do_uuxqt( const char *sysname )
{
   struct HostTable *hostp;
   static char uu_machine[] = UU_MACHINE "=";
   char hostenv[sizeof uu_machine + 25 + 2];
   char executeDirectory[FILENAME_MAX];
   char *pattern;

/*--------------------------------------------------------------------*/
/*                 Determine if we have a valid host                  */
/*--------------------------------------------------------------------*/

   if( !equal( sysname , "all" ) ) {
      if (equal( sysname , E_nodename ))
          hostp = checkname( sysname );
      else
          hostp = checkreal( sysname );

      if (hostp  ==  BADHOST) {
         printmsg(0, "Unknown host \"%s\".", sysname );
         exit(1);
      }

   } else
        hostp = nexthost( TRUE );

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
      boolean locked = FALSE;

/*--------------------------------------------------------------------*/
/*                Initialize security for this remote                 */
/*--------------------------------------------------------------------*/

      if ( !equal(sysname, E_nodename) &&
           (securep = GetSecurity( hostp )) == NULL )
         printmsg(0,"No security defined for \"%s\","
                  " cannot process X.* files",
                  hostp->hostname );
      else {

/*--------------------------------------------------------------------*/
/*              Set up environment for the machine name               */
/*--------------------------------------------------------------------*/

         sprintf(hostenv,"%s%.25s", uu_machine, hostp->hostname);

         if (putenv( hostenv ))
         {
            printmsg(0,"Unable to set environment \"%s\"",hostenv);
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
               locked = TRUE;
            }
            else
               break;               /* We didn't get the lock         */

         } /* while */

         if ( locked )
            UnlockSystem();

      } /* else if */

/*--------------------------------------------------------------------*/
/*    If processing all hosts, step to the next host in the queue     */
/*--------------------------------------------------------------------*/

      if( equal(sysname,"all") )
         hostp = nexthost( FALSE );
      else
         hostp = BADHOST;

   } /*while nexthost*/

   return FALSE;

} /* do_uuxqt */

/*--------------------------------------------------------------------*/
/*    p r o c e s s                                                   */
/*                                                                    */
/*    Process a single execute file                                   */
/*--------------------------------------------------------------------*/

static void process( const char *fname,
                     const char *remote,
                     const char *executeDirectory)
{
   char *command = NULL,
        *input = NULL,
        *output = NULL,
        *job_id = NULL,
        line[BUFSIZ];
   char hostfile[FILENAME_MAX];
   boolean skip = FALSE;
   boolean reject = FALSE;
   FILE *fxqt;
   int status = 0;      /* initialized ONLY to suppress compiler warning */

   char *outnode = NULL;
   char *outname = NULL;
   char *user = NULL;
   char *requestor = NULL;
   char *statfil = NULL;
   char *machine = NULL;

   struct F_list {
      struct F_list *next;
      char *spoolname;
      char *xqtname;
   } *F_list = NULL;

   boolean xflag[UU_LAST - 1] = { 0 };
   time_t jtime = time(NULL);

/*--------------------------------------------------------------------*/
/*                         Open the X.* file                          */
/*--------------------------------------------------------------------*/

   if ( (fxqt = FOPEN(fname, "r", BINARY_MODE)) == NULL)
   {
      printerr(fname);
      return;
   }
   else
      printmsg(2, "processing %s", fname);

/*--------------------------------------------------------------------*/
/*                  Begin loop to read the X.* file                   */
/*--------------------------------------------------------------------*/

   while (!skip & (fgets(line, BUFSIZ, fxqt) != NULL))
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
            printmsg(0,"No user on U line in \"%s\"", fname );
            reject = xflag[F_CORRUPT] = TRUE;
            break;
         }

         user = strdup(cp);
         checkref(user);
                                    /* Get the system name            */
         if ( (cp = strtok(NULL, WHITESPACE)) == NULL)
         {                          /* Did we get a string?           */
            printmsg(2,"No node on U line in \"%s\"", fname );
            cp = (char *) remote;
         }
         else if (!equal(cp,remote))
         {
            printmsg(2,"Node on U line in \"%s\" doesn't match remote",
                     fname );
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
            reject = xflag[F_CORRUPT] = TRUE;
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
            expand_path(hostfile, E_pubdir/*??*/, E_pubdir, NULL);

            if (!equal(remote, E_nodename))
            /* Should the preceding "if (...)" be deleted?  --RHG */
               if (!ValidateFile( hostfile, ALLOW_READ))
               {
                  reject = xflag[S_NOREAD] = TRUE;
                  break;
               }
         }

         input = strdup(hostfile);
         checkref(input);

         break;

/*--------------------------------------------------------------------*/
/*                      Output file for command                       */
/*--------------------------------------------------------------------*/

      case 'O':
         cp = strtok(line + 1, WHITESPACE);
         if ( cp == NULL )
         {
            printmsg(0,"No output file name on O line");
            reject = xflag[F_CORRUPT] = TRUE;
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
            expand_path(hostfile, E_pubdir/*??*/, E_pubdir, NULL);

            if (!equal(remote, E_nodename))
            /* Should the preceding "if (...)" be deleted?  --RHG */
               if (ValidateFile( hostfile, ALLOW_WRITE))
               /* Taylor/GNU uuxqt also rejects it if the output would be in
                  E_spooldir (to keep people from setting up phony requests).
                  I am not sure whether we want to do likewise.  --RHG */
               {
                  reject = xflag[S_NOWRITE] = TRUE;
                  break;
               }
         }

         outname = strdup(hostfile);
         checkref(outname);
         xflag[X_OUTPUT] = TRUE;  /* return output to "outnode"   */

         break;

/*--------------------------------------------------------------------*/
/*                         Command to execute                         */
/*--------------------------------------------------------------------*/

      case 'C':
         cp = line + 1 + strspn(line + 1, WHITESPACE);
         if ( *cp == '\0' )
         {
            printmsg(0,"No command name on C line");
            reject = xflag[F_CORRUPT] = TRUE;
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
            printmsg(0,"No job id on J line in \"%s\"", fname );
            reject = xflag[F_CORRUPT] = TRUE;
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
            printmsg(0,"Missing F parameter in \"%s\", command rejected",
                       fname);
            reject = xflag[F_CORRUPT] = TRUE;
            break;
         }

         if (equaln(cp,"D.",2)
             && (strchr(cp,'/') == NULL)
             && (strchr(cp,'\\') == NULL))
         {
            char temp[FILENAME_MAX];
            struct F_list *p;

            importpath(temp, cp, remote);
            mkfilename(hostfile, E_spooldir, temp);

            if ( access( hostfile, 0 ))   /* Does the host file exist?   */
            {                             /* No --> Skip the file        */
               printmsg(0,"Missing file %s (%s) for %s, command skipped",
                        cp, hostfile, fname);
               skip = TRUE;
               break;
            }

            p = malloc(sizeof *F_list);
            checkref(p);
            p->next = F_list;
            F_list = p;

            F_list->spoolname = strdup(hostfile);
            checkref(F_list->spoolname);

            F_list->xqtname = NULL;
         }
         else
         {
            printmsg(0,"Invalid F parameter in \"%s\", command rejected",
                       fname);
            reject = xflag[F_BADF] = TRUE;
            break;
         }

         cp = strtok(NULL, WHITESPACE);
         if (cp != NULL)
         {
            if (!ValidDOSName(cp, FALSE))
            {  /* Illegal filename --> reject the whole request */
               printmsg(0,"Illegal file \"%s\" in \"%s\", command rejected",
                          cp, fname);
               reject = xflag[F_BADF] = TRUE;
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
            printmsg(0,"No requestor on R line in \"%s\"", fname );
            reject = xflag[F_CORRUPT] = TRUE;
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
            printmsg(0,"No file name on M line in \"%s\"", fname);
            break;
         }

         statfil = strdup(cp);
         checkref(statfil);
         xflag[X_STATFIL] = TRUE;    /* return status to remote file  */

         break;

/*--------------------------------------------------------------------*/
/*                            Flag fields                             */
/*--------------------------------------------------------------------*/

      case 'Z': xflag[X_FAILED] = TRUE;   /* send status if command failed  */
         break;

      case 'N': xflag[X_FAILED] = FALSE;  /* send NO status if command failed */
         break;

      case 'n': xflag[X_SUCCESS] = TRUE;  /* send status if command succeeded */
         break;

      case 'z': xflag[X_SUCCESS] = FALSE; /* NO status if command succeeded */
         break;

      case 'B': xflag[X_INPUT] = TRUE;    /* return command input on error  */
         break;

      case 'e': xflag[X_USEEXEC] = FALSE; /* process command using sh(1)    */
         break;

      case 'E': xflag[X_USEEXEC] = TRUE;  /* process command using exec(2)  */
         break;

/*--------------------------------------------------------------------*/
/*                    Quietly ignore unknown fields                   */
/*--------------------------------------------------------------------*/

      default :
         break;

      } /* switch */
   } /* while (!skip & (fgets(line, BUFSIZ, fxqt) != NULL)) */

   if ( fxqt != NULL )
      fclose(fxqt);


   if ((command == NULL) && !skip)
   {
      printmsg(0,"No command supplied for X.* file %s, rejected", fname);
      reject = xflag[F_CORRUPT] = TRUE;
   }

/*--------------------------------------------------------------------*/
/*           We have the data for this command; process it            */
/*--------------------------------------------------------------------*/

   if ( !skip )
   {
      if ( !reject )
      {
         if ( user == NULL )
         {
            user = strdup("uucp"); /* User if none given              */
            checkref(user);
         }

         if (requestor == NULL)
         {
            requestor = strdup(user);
            checkref(requestor);
         }

         if (input == NULL)
         {
#ifdef WIN32
            input = strdup("NUL:");
#else
            input = strdup("/dev/nul"); /* NOTE: DOS uses only one L in NUL */
#endif
         }

         output = mktempname(NULL, "OUT");

         printmsg(equaln(command,RMAIL,5) ? 2 : 0,
                  "uuxqt: executing \"%s\" for user \"%s\" at  \"%s\"",
                      command, user, machine);

/*--------------------------------------------------------------------*/
/*           Copy the input files to the execution directory          */
/*--------------------------------------------------------------------*/

         /* Make sure the directory exists before we copy the files */
         PushDir(executeDirectory);

         {
            struct F_list *p;

            for (p = F_list; p != NULL; p = p->next)
            {
               if (p->xqtname != NULL)
                  if (!copylocal(p->spoolname, p->xqtname))
                  {
                     /* Should we try again later in case its a temporary
                        error like execute directory on a full disk?  For
                        now, just reject it completely. */
                     printmsg(0, "Copy \"%s\" to \"%s\" failed",
                                 p->spoolname, p->xqtname);
                     reject = xflag[F_NOCOPY] = TRUE;
                     break;
                  }
            } /* for ( ;; ) */
        }

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
               reject = xflag[F_CORRUPT] = TRUE;
            }
            else if ((pipe = strchr(cmd, '|')) == NULL) /* Any pipes? */
               status = shell(cmd, input, output, remote, xflag); /* No */
            else
            { /* We currently do pipes by simulating them using files */
               char *pipefile = mktempname(NULL, "PIP");
               char *next_cmd = cmd; /* initialized ONLY to suppress compiler warning */

               *pipe = '\0';
               status = shell(cmd, input, pipefile, remote, xflag);
               /* *pipe = '|'; */

               while (status == 0
                      && (pipe = strchr(next_cmd = pipe + 1, '|')) != NULL)
               {
                  /* Swap the output and pipe files for the next pass */
                  char *p = pipefile; pipefile = output; output = p;

                  xflag[E_NORMAL] = FALSE;
                  *pipe = '\0';
                  status = shell(next_cmd, output, pipefile, remote, xflag);
                  /* *pipe = '|'; */
               }

               if (status == 0)
               {
                  xflag[E_NORMAL] = FALSE;
                  status = shell(next_cmd, pipefile, output, remote, xflag);
               }

               unlink(pipefile);
               free(pipefile);
            }

            free(cmd);
         }

/*--------------------------------------------------------------------*/
/*                  Clean up files after the command                  */
/*--------------------------------------------------------------------*/

         PopDir();

         {
            struct F_list *p;

            for (p = F_list; p != NULL; p = p->next)
               if (p->xqtname != NULL)
                  unlink(p->xqtname);
         }

      } /* if (!reject) */

      {
         struct F_list *p;

         for (p = F_list; p != NULL; p = p->next)
            unlink(p->spoolname);
      }

      ReportResults( status, input, output, command, job_id,
                     jtime, requestor, outnode, outname, xflag,
                     statfil, machine, user);

      if (!reject)
         unlink(output);

      unlink(fname);

   } /* (!skip) */

/*--------------------------------------------------------------------*/
/*              Free various temporary character strings              */
/*--------------------------------------------------------------------*/

   while (F_list != NULL)
   {
      struct F_list *next;

      free(F_list->spoolname);
      if (F_list->xqtname != NULL)
         free(F_list->xqtname);

      next = F_list->next;
      free(F_list);
      F_list = next;
   }

   if (command    != NULL) free(command);
   if (input      != NULL) free(input);
   if (job_id     != NULL) free(job_id);
   if (outnode    != NULL) free(outnode);
   if (output     != NULL) free(output);
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
                 const char *outname,
                 const char *remotename,
                 boolean xflag[])
{
   int    result = 0;

#if defined(BIT32ENV)
   char   commandBuf[1024];   /* New OS/2, Windows NT environments   */
#elif defined(__TURBOC__)
   char   commandBuf[128];    /* Original DOS environment            */
#else
   char   commandBuf[255];    /* Maybe MS C DOS, or OS/2 1.x         */
#endif

   char   *cmdname;
   char   *parameters;

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
      else
      {
         /* MISSING CODE: we should check the parameters to see that all file
            references are legitimate.  What do other implementations of uuxqt
            do for this access check?  Maybe check for READ access if a path
            is specified but accept anything that isn't a pathname (such as a
            username for an RMAIL command)?  I'm not yet completely sure.

            Taylor/GNU uuxqt seems to check for both READ and WRITE access
            if it starts with a '/' and also rejects most (but not all) names
            containing "..".

            Also, what quoting conventions should we follow when parsing?
            The DOS COMMAND.COM certainly does not handle \-style quoting,
            for example, so we don't want to get fooled by handling such here.

                                                                      --RHG */
      }
   } /* if ( parameters != NULL ) */

/*--------------------------------------------------------------------*/
/*    Verify we support the command, and get it's real name, if so    */
/*--------------------------------------------------------------------*/

   if ( (!equal(remotename, E_nodename)) && (!ValidateCommand( cmdname )) )
   {
      printmsg(0,"Command \"%s\" not allowed at this site", cmdname);
      xflag[E_NOEXE] = TRUE;
      return 99;
   }

/*--------------------------------------------------------------------*/
/*               We support the command; execute it                   */
/*--------------------------------------------------------------------*/

   fflush(logfile);

/*--------------------------------------------------------------------*/
/*               RNEWS may be special, handle it if so                */
/*--------------------------------------------------------------------*/

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
                        outname,
                        TRUE,
                        FALSE );
   }

/*--------------------------------------------------------------------*/
/*        RMAIL is special, we need to break up the parameters        */
/*--------------------------------------------------------------------*/

   else if (equal(cmdname,RMAIL) && ( inname != NULL )) /* rmail w/input?  */
   {
      parameters = strtok( parameters, WHITESPACE );

      while (( parameters != NULL ) && (result != -1 ))
      {

         boolean firstPass = TRUE;
         int left;

         int rlen = IsDOS() ? 126 : sizeof commandBuf - 2;

#ifdef _Windows
         if ( bflag[F_WINDOWS] )
         {
            strcpy( commandBuf, "-f ");
            strcat( commandBuf, inname);
            strcat( commandBuf, " ");
         }
         else
            *commandBuf = '\0';
#else
         *commandBuf = '\0';
#endif
         rlen -= strlen( commandBuf ) + strlen( RMAIL ) + 1;

/*--------------------------------------------------------------------*/
/*                   Copy addresses into the buffer                   */
/*--------------------------------------------------------------------*/

         left = rlen - strlen( parameters );

         while ((parameters != NULL) && (left > 0))
         {
            char *next = strtok( NULL, "");

            if ( *parameters == '-')   /* Option flag for mail?        */
               printmsg(0,"Disallowed option %s ignored",parameters);
            else {                     /* Not option, add to param list  */
               strcat( commandBuf, " ");
               strcat( commandBuf, parameters );
               rlen -= strlen( parameters ) + 1;
               firstPass = FALSE;
            }

/*--------------------------------------------------------------------*/
/*                       Step to next parameter                       */
/*--------------------------------------------------------------------*/

            if ( next == NULL )
               parameters = NULL;
            else
               parameters = strtok( next, WHITESPACE );

         } /* while ( parameters != NULL ) */

         if (firstPass)       /* Did we process at least one addr?     */
         {                    /* No --> Serious problem!              */
            printmsg(0,
                     "Address \"%s\" too long (%d chars)!  %d available, short fall would be %d",
                      parameters,
                      strlen(parameters),
                      rlen,
                      left );

            panic();
         } /* if (*commandBuf = '\0') */

      } /* while */

/*--------------------------------------------------------------------*/
/*               Execute one command line of addresses                */
/*--------------------------------------------------------------------*/

      result = execute( RMAIL,
                        commandBuf,
                        bflag[F_WINDOWS] ? NULL : inname,
                        outname,
                        TRUE,
                        FALSE );

      if ( result != 0 )    /* Did command execution fail?            */
      {
         printmsg(0,"shell: command \"%s %s\" returned error code %d",
               cmdname, commandBuf, result);
         panic();
      }

   } /* if (equal(cmdname,RMAIL) && ( inname != NULL )) */
   else
      result = execute( cmdname,
                        parameters,
                        inname,
                        outname,
                        TRUE,
                        FALSE );

/*--------------------------------------------------------------------*/
/*                    Determine result of command                     */
/*--------------------------------------------------------------------*/

   if ( result == 0 )
      xflag[E_NORMAL] = TRUE;
   else if ( equal(cmdname, RNEWS) )
                           /* Did command execution fail?            */
   {
      printmsg(0,"shell: command %s returned error code %d",
            cmdname, result);
      panic();
   }
   else if ( result > 0 )
      xflag[E_STATUS] = TRUE;

   fflush(logfile);

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

static boolean copylocal(const char *from, const char *to)
{
      int  fd_from, fd_to;
      int  nr;
      int  nw = -1;
      char buf[BUFSIZ];            /* faster if we alloc a big buffer  */

      /* This would be even faster if we determined that both files
         were on the same device, dos >= 3.0, and used the dos move
         function EXCEPT that we want a COPY, not a MOVE! */

      if ((fd_from = open(from, O_RDONLY | O_BINARY)) == -1)
         return FALSE;        /* failed                                */

      /* what if the to is a directory? */
      /* possible with local source & dest uucp */

      if ((fd_to = open(to, O_CREAT | O_BINARY | O_WRONLY, S_IWRITE | S_IREAD)) == -1) {
         close(fd_from);
         return FALSE;        /* failed                                */
         /* NOTE - this assumes all the required directories exist!  */
      }

      while  ((nr = read(fd_from, buf, sizeof buf)) > 0 &&
         (nw = write(fd_to, buf, nr)) == nr)
         ;

      close(fd_to);
      close(fd_from);

      if (nr != 0 || nw == -1)
         return FALSE;        /* failed in copy                       */
      return TRUE;
} /* copylocal */

/*--------------------------------------------------------------------*/
/*    c r e a t e _ e n v i r o n m e n t                             */
/*                                                                    */
/*    Create the environment array for subprocesses                   */
/*--------------------------------------------------------------------*/

static void create_environment(const char *requestor)
{
   static char buffer[MAXADDR + 20];
   int subscript = 0;
   char *envp[3];

/*--------------------------------------------------------------------*/
/*               user id/nodename of original requestor               */
/*--------------------------------------------------------------------*/

   if ( requestor == NULL )
      panic();
   else if ( ! strlen( requestor ))
      sprintf(buffer,"%s=%s %s", UU_USER, "uucp", E_nodename );
   else {
      sprintf(buffer,"%s=%s",UU_USER, requestor);
   }

   envp[subscript++] =  buffer;

/*--------------------------------------------------------------------*/
/*               Now put the data into our environment                */
/*--------------------------------------------------------------------*/

   envp[subscript] =  NULL;   /* Terminate the list                    */

   while( subscript-- > 0)
   {
      if (putenv( envp[subscript] ))
      {
         printmsg(0,"Unable to set environment \"%s\"",envp[subscript]);
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

static boolean do_copy(char *localfile,
                       const char *rmtsystem,
                       const char *remotefile,
                       const char *requestor,
                       const boolean success )
{
      if (rmtsystem == NULL) {
          copylocal(localfile, remotefile);
      } else {
          char    tmfile[FILENAME_MAX];  /* Unix style name for c file  */
          char    idfile[FILENAME_MAX];  /* Unix style name for data file copy  */
          char    work[FILENAME_MAX]; /* temp area for filename hacking  */
          char    icfilename[FILENAME_MAX];  /* our hacked c file path  */
          char    idfilename[FILENAME_MAX];  /* our hacked d file path  */

          struct  stat    statbuf;

          long    int     sequence;
          static  char    subseq = 'A';
          char   *sequence_s;
          FILE   *cfile;


          sequence = getseq();
          sequence_s = JobNumber( sequence );

          sprintf(tmfile, spool_fmt, 'C', rmtsystem, 'Z', sequence_s);
          importpath(work, tmfile, rmtsystem);
          mkfilename(icfilename, E_spooldir, work);

          if (stat((char *) localfile, &statbuf) != 0)  {
              printerr( localfile );
              return FALSE;
          }

          sprintf(idfile , dataf_fmt, 'D', E_nodename, sequence_s,
                  (char) subseq++ );
          importpath(work, idfile, rmtsystem);
          mkfilename(idfilename, E_spooldir, work);

          if (!copylocal(localfile, idfilename))  {
             printmsg(0, "Copy \"%s\" to \"%s\" failed", localfile, idfilename);
             return FALSE;
          }

          if ((cfile = FOPEN(icfilename, "a",TEXT_MODE)) == NULL)  {
             printerr( icfilename );
             printf("cannot append to %s\n", icfilename);
             return FALSE;
          }

          fprintf(cfile, success ? "S %s %s uucp -n %s 0666 %s\n"
                                 : "S %s %s uucp - %s 0666\n",
                         localfile, remotefile, idfile, requestor);

          fclose(cfile);
    };

    return TRUE;
} /* do_copy */

/*--------------------------------------------------------------------*/
/*    R e p o r t R e s u l t s                                       */
/*                                                                    */
/*    report results of command execution as specified by flags in    */
/*    X.* file.                                                       */
/*--------------------------------------------------------------------*/

static void ReportResults(const int status,
                          const char *input,
                                char *output,
                          const char *command,
                          const char *job_id,
                          const time_t jtime,
                          const char *requestor,
                          const char *outnode,
                          const char *outname,
                          const boolean xflag[],
                          const char *statfil,
                          const char *machine,
                          const char *user)
{
   char address[MAXADDR];
   char subject[80];
   FILE *mailtmp = NULL;
   char *tempmail;


   if (!(xflag[X_FAILED] | xflag[X_SUCCESS] |
         xflag[X_INPUT]  | xflag[X_STATFIL] ))
   {  /* default actions */
      return;
   }

   tempmail = mktempname(NULL, "TMP");

   if ((mailtmp = FOPEN(tempmail, "w+", BINARY_MODE)) == NULL) {
      printerr(tempmail);
      return;
   }

   sprintf(subject, "\"[uucp job %s (%s)]\"", job_id, dater(jtime, NULL) );

   fprintf(mailtmp,"remote execution\n");
   fprintf(mailtmp,"%s\n", command);

#ifdef BETA_TEST
   strcpy(address,"postmaster");
#else
   if (equal(machine, E_nodename))
      strcpy(address, requestor);
   else
      sprintf(address,"%s!%s", machine, requestor);
#endif

   if (xflag[E_NORMAL])
   {                        /* command succeded, process appropriate flags */

      fprintf(mailtmp,"exited normally\n");

      if (xflag[X_OUTPUT])
         do_copy(output, outnode, outname, requestor, xflag[X_SUCCESS]);

      fclose(mailtmp);

      if (xflag[X_SUCCESS]) {
         if (xflag[X_STATFIL]) {
            do_copy(tempmail, outnode, statfil, requestor, xflag[X_SUCCESS]);
         } else {
            MailStatus(tempmail, address, subject);
         }
      }

   } else {            /* command failed, process appropriate flags   */
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
                  AppendData( input, mailtmp);
              };

           }

           fprintf(mailtmp,"===== stderr is unavailable =====\n");
        }
     }

     fclose(mailtmp);

     if (xflag[X_STATFIL]) {
         do_copy(tempmail, outnode, statfil, requestor, xflag[X_SUCCESS]);
     } else {
         MailStatus(tempmail, address, subject);
     }

   }

   unlink(tempmail);
   return;
} /* ReportResults */

/*--------------------------------------------------------------------*/
/* A p p e n d D a t a                                                */
/*                                                                    */
/* Append data to output file                                         */
/*--------------------------------------------------------------------*/

static boolean AppendData( const char *input, FILE* dataout)
{
   FILE    *datain;
   char     buf[BUFSIZ];
   boolean  status = TRUE;

/*--------------------------------------------------------------------*/
/*                      Verify the input opened                       */
/*--------------------------------------------------------------------*/

   if (input == NULL)
      return FALSE;
   else
      datain = FOPEN(input, "r",TEXT_MODE);

   if (datain == NULL) {
      printerr(input);
      printmsg(0,"Unable to open input file \"%s\"", input);
      return FALSE;
   } /* datain */

/*--------------------------------------------------------------------*/
/*                       Loop to copy the data                        */
/*--------------------------------------------------------------------*/

   while (fgets(buf, BUFSIZ, datain) != 0)
   {
      if (fputs(buf, dataout) == EOF)     /* I/O error?               */
      {
         printmsg(0,"AppendData: I/O error on output file");
         printerr("dataout");
         fclose(datain);
         return FALSE;
      } /* if */
   } /* while */

/*--------------------------------------------------------------------*/
/*                      Close up shop and return                      */
/*--------------------------------------------------------------------*/

   if (ferror(datain))        /* Clean end of file on input?           */
   {
      printerr(input);
      clearerr(datain);
      status = FALSE;
   }

   fclose(datain);
   return status;

} /* AppendData */

/*--------------------------------------------------------------------*/
/*    M a i l S t a t u s                                             */
/*                                                                    */
/*    Send text in a mailbag file to address(es) specified by line.   */
/*--------------------------------------------------------------------*/

static boolean MailStatus(char *tempfile,
                          char *address,
                          char *subject)
{
   boolean status;
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
   strcat( buf, " " );
   strcat( buf, address );

   status = execute( RMAIL, buf, NULL, NULL, TRUE, FALSE );

/*--------------------------------------------------------------------*/
/*                       Report errors, if any                        */
/*--------------------------------------------------------------------*/

   if ( status < 0 )
   {
      printerr( RMAIL );
      printmsg(0,"Unable to execute rmail; status not delivered.");
   }
   else if ( status > 0 )
      printmsg(0, "Rmail returned error;\
 status delivery may be incomplete.");

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return (status == 0 );

} /*MailStatus*/
