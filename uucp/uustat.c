/*--------------------------------------------------------------------*/
/*    u u s t a t . c                                                 */
/*                                                                    */
/*    Job status report for UUPC/extended                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright 1988 (C), Dewey Coffman                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uustat.c 1.31 1996/11/19 00:14:35 ahd Exp $
 *
 *    $Log: uustat.c $
 *    Revision 1.31  1996/11/19 00:14:35  ahd
 *    Add more debugging information
 *
 *    Revision 1.30  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.29  1996/01/01 21:34:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.28  1995/07/21 13:18:16  ahd
 *    Improved time reporting for queued files
 *
 *    Revision 1.27  1995/03/11 22:30:54  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.26  1995/03/11 15:49:23  ahd
 *    Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *    Revision 1.25  1995/01/07 16:41:40  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.24  1995/01/02 05:04:25  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.23  1994/12/22 00:44:37  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.22  1994/04/27 00:02:15  ahd
 *    Properly trap failure to delete files when killing jobs
 *    consistently use printmsg() for output
 *
 * Revision 1.21  1994/03/15  03:02:26  ahd
 * Further shorten summary buffer
 *
 * Revision 1.20  1994/03/13  17:23:33  ahd
 * Lower memory usage under DOS
 *
 */

#include "uupcmoah.h"

static const char rcsid[] =
         "$Id: uustat.c 1.31 1996/11/19 00:14:35 ahd Exp $";

/*--------------------------------------------------------------------*/
/*         System include files                                       */
/*--------------------------------------------------------------------*/

#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

/*--------------------------------------------------------------------*/
/*         Local include files                                        */
/*--------------------------------------------------------------------*/

#include "dater.h"
#include "export.h"
#include "getopt.h"
#include "getseq.h"
#include "hostable.h"
#include "hostatus.h"
#include "import.h"
#include "pushpop.h"
#include "readnext.h"
#include "security.h"
#include "stater.h"
#include "timestmp.h"

#define DAY (60l * 60l * 24l)
#define ALL    "all"
#define MAXL      30       /* Max user and system length saved    */

#define STRCREAT(s, s2, s3)\
   strcpy(s, s2);\
   strcat(s, s3);\

/*--------------------------------------------------------------------*/
/*                          Local data types                          */
/*--------------------------------------------------------------------*/

typedef enum {
      POLL_CALL = 'P',
      RECEIVE_CALL = 'R',
      SEND_CALL = 'S'
      } CALLTYPE;

/*--------------------------------------------------------------------*/
/*       A running tug of war exists with the length of the           */
/*       data_queue file name buffer.  If its BUFSIZ then it can      */
/*       show more command information when used to show commands,    */
/*       but this consumes massive space under 16 systems such as     */
/*       DOS.  We make it the length of a file name, which is         */
/*       short but efficient AND about the same amount that UNIX      */
/*       shows.                                                       */
/*                                                                    */
/*       Note that some routines clobber the end of the buffer, so    */
/*       verify references are changed as well if you change this.    */
/*--------------------------------------------------------------------*/

struct data_queue {
   char name[FILENAME_MAX];
   struct data_queue *next_link;
   time_t created;
   long size;
   KWBoolean execute;
   char type;
} ;

/*--------------------------------------------------------------------*/
/*                             Verb list                              */
/*--------------------------------------------------------------------*/

typedef enum {
   LIST_DEFAULT = 1,
   LIST_ALL,
   LIST_ACCESS,
   LIST_QUEUE,
   KILL_JOB,
   REFRESH_JOB,
   FORCE_POLL
   } COMMAND_CLASS;

/*--------------------------------------------------------------------*/
/*                Processing to be taken by open_call                 */
/*--------------------------------------------------------------------*/

typedef enum {
   JOB_STATUS = 1,
   JOB_KILL,
   JOB_REFRESH
   } CALL_ACTION;

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static const char *host_status[] = {
      "(invalid - entry not properly initialized)",
      "(local host system)",
      "(host for gateway purposes only)",
      "(host for SMTP relay only)",
      "Never called",
      "Dialing now",
      "Invalid device/speed in SYSTEMS file",
      "Device not available",
      "Conversation start-up failed",
      "Talking",
      "Callback required",
      "Modem initialization failed",
      "Dial failed",
      "Script failed",
      "Max retry reached",
      "Retry time not reached",
      "Call succeeded",
      "Wrong machine name",
      "Unknown host",
      "Failed",
      "Wrong time to call",
      "(call successed, entry not reset)",
   } ;

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void all( const char *system, const char *userid );

static char *is_job(const char *jobid );

static void kill_job(const char *s);

static void long_stats( const char *system );

static void short_stats( const char *system );

static CALLTYPE open_call( const char *callname,
                           const char *remote,
                                 struct data_queue **current,
                                 char *user,
                                 char *sys,
                           const CALL_ACTION action);

static void open_data(const char *file,
                            char *user,
                            char *sys,
                            char *command,
                            const size_t commandLen);

static void poll(const char *callee, const char grade );

static void print_all(       char *job,
                       struct data_queue *current,
                       const char *user,
                       const char *sys);

static void refresh_job(const char *s);

static void touch( const char *fname );

static void usage( void );

/*--------------------------------------------------------------------*/
/*       Because we're tight on storage under DOS, enable stack       */
/*       checking.                                                    */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma -N
#elif !defined(__IBMC__)
#pragma check_stack( on )
#endif

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

main(int  argc, char  **argv)
{
   int c;
   extern char *optarg;
   extern int   optind;
   COMMAND_CLASS command = LIST_DEFAULT;

   char grade = 'Z';

   char *system = NULL;
   char *userid = NULL;
   char *job    = NULL;

/*--------------------------------------------------------------------*/
/*     Report our version number and date/time compiled               */
/*--------------------------------------------------------------------*/

   debuglevel = 0;
   banner( argv );

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   if (!configure( B_UUSTAT ))
      exit(1);   /* system configuration failed */

/*--------------------------------------------------------------------*/
/*                   Switch to the spool directory                    */
/*--------------------------------------------------------------------*/

   PushDir( E_spooldir );
   atexit( PopDir );

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

   while ((c = getopt(argc, argv, "ag:k:mqr:s:u:x:P:")) !=  EOF)
      switch(c) {
      case 'a':
         command = LIST_ALL;
         break;

      case 'g':
         grade = *optarg;
         break;

      case 'm':
         command = LIST_ACCESS;
         system = optarg;
         break;

      case 'q':
         command = LIST_QUEUE;
         break;

      case 'k':
         command = KILL_JOB;
         job = optarg;
         break;

      case 'r':
         command = REFRESH_JOB;
         job = optarg;
         break;

      case 's':
         if ( system != NULL )
         {
            printmsg(0,"Invalid or duplicate option -s %s",optarg);
            usage();
         }
         system = optarg;
         break;

      case 'u':
         userid = optarg;
         break;

      case 'x':
         debuglevel = atoi( optarg );
         break;

      case 'P':
         command = FORCE_POLL;
         if ( system != NULL )
         {
            printmsg(0,"Invalid or duplicate option -P %s",optarg);
            usage();
         }
         system = optarg;
         break;

      case '?':
         usage();
   }

   if (optind != argc) {
      puts("Extra parameter(s) at end.");
      exit(2);
   }

   checkname(E_nodename);     /* Force loading of host table         */

/*--------------------------------------------------------------------*/
/*                 Determine if we have a valid host                  */
/*--------------------------------------------------------------------*/

   if( (system != NULL) &&
       !equal( system , ALL ) &&
       !equal( system, E_nodename ))
   {
      struct HostTable *hostp = checkreal( system );

      if (hostp  ==  BADHOST)
      {
         printmsg(0,"Unknown host \"%s\", program terminating.",
               system );
         panic();
      }
   } /* if */

/*--------------------------------------------------------------------*/
/*                   Execute the requested command                    */
/*--------------------------------------------------------------------*/

   switch ( command )
   {
      case LIST_DEFAULT:
         if ( (system == NULL ) && ( userid == NULL) )
         {
            all( ALL, E_mailbox );
            break;
         }
            /* Otherwise, fall through ... */

      case LIST_ALL:
         if ( system == NULL )
            system = ALL;
         if ( userid == NULL )
            userid = ALL;
         all( system, userid );
         break;

      case LIST_ACCESS:
         if ( system == NULL )
            system = ALL;
         short_stats( system );
         break;

      case LIST_QUEUE:
         if ( system == NULL )
            system = ALL;
         long_stats( system );
         break;

      case KILL_JOB:
         kill_job( job );
         break;

      case REFRESH_JOB:
         refresh_job( job );
         break;

      case FORCE_POLL:
         poll( system, grade );
         break;

      default:
         panic();

   } /* switch */

   return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*    a l l                                                           */
/*                                                                    */
/*    Report on all systems                                           */
/*--------------------------------------------------------------------*/

void all( const char *system, const char *userid)
{
   long  size;
   time_t ltime;
   struct HostTable *hostp;
   KWBoolean hit = KWFalse;

   if ( equal(system,ALL) )
      hostp = nexthost( KWTrue );
   else
      hostp = checkreal( system );

/*--------------------------------------------------------------------*/
/*                  Scan one or all host directories                  */
/*--------------------------------------------------------------------*/

   while  (hostp !=  BADHOST )
   {
      char fname[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*   Examine all of the files in the spool directory for this host    */
/*--------------------------------------------------------------------*/

      while( readnext(fname , hostp->hostname, "C", NULL, &ltime, &size) )
      {
         KWBoolean display;
         struct data_queue *data_link = NULL;
         char  canon[FILENAME_MAX];
         char user[MAXL];
         char sys[MAXL];

         if ( equali( userid, ALL ))
            display = KWTrue;
         else
            display = KWFalse;

         strcpy(user,userid);       /* Nice default for the user      */
                                    /* Generates more output if
                                       straight copies in the queue  */
         strcpy(sys, E_nodename);   /* Nice default for node as well  */

         printmsg(1,"ALL(%s)", fname);

         exportpath(canon, fname, hostp->hostname);

/*--------------------------------------------------------------------*/
/*               Determine what kind of Call file it is               */
/*--------------------------------------------------------------------*/

         switch(open_call(fname,
                           hostp->hostname,
                           &data_link,
                           user,
                           sys,
                           JOB_STATUS))
         {
            case POLL_CALL:
               if ( display )
               {
                  hit = KWTrue;
                  printmsg(0,"%-12s %s %s",
                          canon+2,
                          dater(ltime, NULL),
                        "(POLL)");
               }
               break;

            case SEND_CALL:
            case RECEIVE_CALL:
               if( equal(userid , ALL) || equali(userid, user))
                  display = KWTrue;

               if (display)
               {
                   hit = KWTrue;
                   print_all( canon + 2,
                              data_link,
                              user,
                              hostp->hostname );
               }
               else while( data_link ) /* Free w/o printing          */
               {
                  struct data_queue *next = data_link->next_link;
                  free( data_link );
                  data_link = next;
               } /* while */
               break;

            default:
               printmsg(0,"Unknown call type in file %s", fname );
               panic();

         } /* switch */

      } /* while */

/*--------------------------------------------------------------------*/
/*    If processing all hosts, step to the next host in the queue     */
/*--------------------------------------------------------------------*/

      if ( equal( system , ALL ))
         hostp = nexthost( KWFalse );
      else
         hostp = BADHOST;

   } /* while */

   if ( !hit )
      printmsg(0,"uustat: No jobs queued for system %s by user %s",
               system , userid );

} /* all */

/*--------------------------------------------------------------------*/
/*    p o l l                                                         */
/*                                                                    */
/*    Write a dummy call file to request a poll of a host             */
/*--------------------------------------------------------------------*/

static void poll(const char *callee, const char grade )
{

   char tmfile[15];           /* Call file, UNIX format name           */
   char msname[FILENAME_MAX];
   FILE *stream;
   static char *spool_fmt = SPOOLFMT;              /* spool file name  */
   struct HostTable *hostp;

/*--------------------------------------------------------------------*/
/*              Determine first (only?) host to process               */
/*--------------------------------------------------------------------*/

   if ( equal(callee,ALL) )
      hostp = nexthost( KWTrue );
   else
      hostp = checkreal( callee );

/*--------------------------------------------------------------------*/
/*                  Scan one or all host directories                  */
/*--------------------------------------------------------------------*/

   while  (hostp !=  BADHOST )
   {
      printmsg(1,"POLL(%s) grade(%c)", hostp->hostname, grade);

      sprintf(tmfile,"%.8s",hostp->hostname);

      if ( ValidDOSName( tmfile, KWFalse ) || !equal(callee, ALL))
      {
         sprintf(tmfile, spool_fmt, 'C', hostp->hostname, grade ,
                  "000");
         importpath( msname, tmfile, hostp->hostname);

         if ( access( msname , 0 ))   /* Does the host file exist?     */
         {
            stream = FOPEN( msname ,"w", IMAGE_MODE);

            if ( stream == NULL )
            {
               printerr( msname );
               printmsg(0,"uustat: problem creating poll file %s (%s)",
                        tmfile, msname);
               panic();
            }

            fclose( stream );

            printmsg(0,"Created dummy job %s (%s) for system %s",
                  tmfile, msname, hostp->hostname );

         } /* if */
         else
            printmsg(0,"Dummy job %s (%s) already exists for system %s",
                  tmfile, msname, hostp->hostname );

      } /* if ( ValidDOSName( tmfile ) || !equal(callee, ALL)) */
      else {
         printmsg(0,"%s is not a valid host name, skipping",
                     hostp->hostname);
      } /* else */

/*--------------------------------------------------------------------*/
/*    If processing all hosts, step to the next host in the queue     */
/*--------------------------------------------------------------------*/

      if( equal( callee , ALL ))
         hostp = nexthost( KWFalse );
      else
         hostp = BADHOST;

   } /* while  (hostp !=  BADHOST ) */

} /* poll */

/*--------------------------------------------------------------------*/
/*    l o n g _ s t a t s                                             */
/*                                                                    */
/*    Report full information on jobs for a host                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/* fifi      1C     03/30-16:34 LOGIN FAILED Retry: 22:39 Count: 130  */
/* helps     2C     03/30-16:32 SUCCESSFUL                            */
/* im4u      5C(1)  03/30-06:50 LOGIN FAILED Retry: 11:15 Count: 9    */
/* irasun           03/30-16:23 TALKING                               */
/* killer    5C(6)  03/29-11:13 WRONG TIME TO CALL                    */
/* radian    18C    03/30-16:42 WRONG TIME TO CALL                    */
/* shemp      1C    03/30-14:43 LOGIN FAILED Retry: 20:48 Count: 107  */
/* unisec     2C(1) 03/30-16:50 WRONG TIME TO CALL                    */
/* ut-emx     1X    03/30-16:50 SUCCESSFUL                            */
/*--------------------------------------------------------------------*/

static void long_stats( const char *system )
{
   struct HostTable *hostp;
   time_t now = time( NULL );
   KWBoolean hit = KWFalse;
   KWBoolean firstPass = KWTrue;

   HostStatus();              /* Load the host status table info      */

/*--------------------------------------------------------------------*/
/*                  Get the first system to process                   */
/*--------------------------------------------------------------------*/

   if ( equal(system,ALL) )
      hostp = checkname( E_nodename ); /* Start with local system    */
   else
      hostp = checkname( system );  /* not checkreal, may be local   */

/*--------------------------------------------------------------------*/
/*              Begin loop to display status of systems               */
/*--------------------------------------------------------------------*/

   while(hostp != BADHOST )
   {
      static const char *prefix[2] = { "C","X" };

      char fname[FILENAME_MAX];
      char summary[30];          /* Generous, really only need ~ 20  */
      KWBoolean work = KWFalse;
      size_t subscript;

      *summary = '\0';           /* Clear output buffer              */

/*--------------------------------------------------------------------*/
/*       Middle loop to handle both call file and execute files       */
/*--------------------------------------------------------------------*/

      for ( subscript = 0; subscript < 2; subscript++ )
      {

         size_t jobs = 0;
         time_t oldest_file = now;

         time_t ltime;
         long size;

/*--------------------------------------------------------------------*/
/*        Inner loop to count call files and determine oldest         */
/*--------------------------------------------------------------------*/

         while( readnext(fname,
                         hostp->hostname,
                         prefix[subscript],
                         NULL, &ltime, &size) != NULL )
         {
            if ((ltime < oldest_file)
                && (ltime != (time_t) -1L)
                && (ltime != (time_t) -2L))
               oldest_file = ltime;

            jobs ++;

         } /* while */

/*--------------------------------------------------------------------*/
/*      Now format the input for this call or execute file list       */
/*--------------------------------------------------------------------*/

         if ( jobs > 0 )
         {
            size_t unit = 0;
            static const char label[] = { 's', 'm', 'h', 'd', '?' };
            static const long step[]  = {   1,  60,  60,  24, LONG_MAX };
            time_t age = now - oldest_file;

/*--------------------------------------------------------------------*/
/*       Determine the best unit for the age; we require at least     */
/*       two of a unit before using it over the next lower unit to    */
/*       allow more accurate reporting.                               */
/*--------------------------------------------------------------------*/

            while (( age / step[unit + 1]) > 1 )
            {
               age /= step[++unit];
            }

/*--------------------------------------------------------------------*/
/*                 Format the information on the jobs                 */
/*--------------------------------------------------------------------*/

            sprintf( fname ,
                     "%d%s(%ld%c)",
                     jobs,
                     prefix[subscript],
                     age,
                     label[unit] );

            work =  hit = KWTrue;

         } /* if */
         else
            *fname = '\0';             /* No information to display  */

         sprintf(summary + strlen(summary), "%-10s ",fname );

      } /* for ( subscript = 0; subscript < 3; subscript++ ) */

/*--------------------------------------------------------------------*/
/*       We have all the information for this system; display if it   */
/*       has any work.                                                */
/*--------------------------------------------------------------------*/

      if ( work )
      {
         if ( equal(hostp->hostname, E_nodename ))
            printmsg(0,"%-10.10s %s",
                    hostp->hostname,
                    summary );
         else
            printmsg(0,"%-10.10s %s%s %s",
                    hostp->hostname,
                    summary,
                    dater( hostp->status.lconnect , NULL ),
                    hostp->status.hstatus < HS_ZZ_LAST ?
                        host_status[ hostp->status.hstatus ] :
                        "*** INVALID/UNDOCUMENTED STATUS ***");
      } /* if (work) */

/*--------------------------------------------------------------------*/
/*       Step to next host in list if processing all hosts, else      */
/*       exit the loop                                                */
/*--------------------------------------------------------------------*/

      if (equal(system, ALL))
      {
         hostp = nexthost( firstPass );
         firstPass = KWFalse;
      }
      else
         hostp = BADHOST;

   } /* while */

/*--------------------------------------------------------------------*/
/*                Report if nothing queued for anyone                 */
/*--------------------------------------------------------------------*/

   if ( !hit )
      printmsg(0,"uustat: No jobs queued for system %s", system );

} /* long_stats */

/*--------------------------------------------------------------------*/
/*    s h o r t _ s t a t s                                           */
/*                                                                    */
/*    Report access to a system                                       */
/*--------------------------------------------------------------------*/

static void short_stats( const char *system )
{
   struct HostTable *hostp;

   HostStatus();              /* Load the host status table info      */

/*--------------------------------------------------------------------*/
/*                  Get the first system to process                   */
/*--------------------------------------------------------------------*/

   if ( equal(system,ALL) )
      hostp = nexthost( KWTrue );
   else
      hostp = checkreal( system );

/*--------------------------------------------------------------------*/
/*              Begin loop to display status of systems               */
/*--------------------------------------------------------------------*/

   while(hostp != BADHOST )
   {
      printmsg(0,"%-10.10s  %s  %s", hostp->hostname,
            dater( hostp->status.lconnect , NULL ),
            hostp->status.hstatus < HS_ZZ_LAST ?
                  host_status[ hostp->status.hstatus ] :
                  "*** INVALID/UNDOCUMENTED STATUS ***");

      if (equal(system, ALL))
         hostp = nexthost( KWFalse );
      else
         hostp = BADHOST;
   } /* while */

} /* short_stats */

/*--------------------------------------------------------------------*/
/*    k i l l _ j o b                                                 */
/*                                                                    */
/*    Kill a queued UUPC/extended job                                 */
/*--------------------------------------------------------------------*/

static void kill_job(const char *jobid)
{
   char *system;              /* System name returned by is_job()     */
   char host[FILENAME_MAX];
   char canon[FILENAME_MAX];
   char user[FILENAME_MAX];
   char sys[FILENAME_MAX];

   strcpy(canon,"C.");
   strcat(canon,jobid);
   strcpy( user, "uucp");
   system = is_job( canon ); /* Never returns if an error occurs      */
   importpath( host, canon, system );
                              /* Get the local name of the file        */
   open_call(host, system, NULL, user, sys, JOB_KILL);
   if ( REMOVE( host ) )
   {
      printmsg(0,"Unable to delete file %s (%s)", canon, host );
      printerr(host);
   }
   else {
      printmsg(0,"Deleted file %s (%s)", canon, host);
      printmsg(0,"Killed job %s (%s) queued for host %s by %s",
            jobid, host, system, user);
   }

} /* kill_job */

/*--------------------------------------------------------------------*/
/*    r e f r e s h _ j o b                                           */
/*                                                                    */
/*    Refresh a job in the spool                                      */
/*--------------------------------------------------------------------*/

static void refresh_job(const char *jobid)
{
   char *system;              /* System name returned by is_job()     */
   char host[FILENAME_MAX];
   char canon[FILENAME_MAX];
   char user[FILENAME_MAX];
   char sys[FILENAME_MAX];

   strcpy(canon,"C.");
   strcat(canon,jobid);
   strcpy( user, "uucp");
   system = is_job( canon ); /* Never returns if an error occurs      */
   importpath( host, canon, system );
                              /* Get the local name of the file        */
   open_call(host, system, NULL, user, sys, JOB_REFRESH);
   touch( host );
   printmsg(0,"Rejuvenated job %s (%s) queued for host %s by %s",
            jobid, host, system, user);

} /* refresh_job */

/*--------------------------------------------------------------------*/
/*    o p e n _ c a l l                                               */
/*                                                                    */
/*    Process a call file                                             */
/*--------------------------------------------------------------------*/

static CALLTYPE open_call( const char *callname,
                           const char *remote,
                                 struct data_queue **top,
                                 char *user,
                                 char *sys,
                           const CALL_ACTION action )
{
   FILE *stream;
   char buf[BUFSIZ];
   struct data_queue *current = NULL;
   CALLTYPE this_call = POLL_CALL;

   char host[FILENAME_MAX];
   char type[FILENAME_MAX], fname[FILENAME_MAX], tname[FILENAME_MAX];
   char flgs[FILENAME_MAX], dname[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*                    Open the file for processing                    */
/*--------------------------------------------------------------------*/

   printmsg(2,"Scanning call file \"%s\"", callname );
   stream = FOPEN( callname, "r",TEXT_MODE );

   if ( stream == NULL )
   {
      printerr( callname );
      panic();
   }

   while( fgets( buf, BUFSIZ, stream) != NULL)
   {
      switch (*buf)
      {
         case '#':
            break;

         case 'S':
         case 'R':
            *dname = '\0';
            sscanf(buf, "%s %s %s %s %s %s",
                         type, fname, tname, user , flgs, dname);
            this_call = (CALLTYPE) (*type);

            if ( ! strlen( dname ) )
            {
               printmsg(0,"Invalid data in call file \"%s\"",callname);
               panic();
            }

/*--------------------------------------------------------------------*/
/*    If returning job status, allocate next link in status queue     */
/*--------------------------------------------------------------------*/

            if ( action == JOB_STATUS )
            {
              if ( current == NULL)
                 *top = current = malloc( sizeof *current );
              else {
                 current->next_link = malloc( sizeof *current );
                 current = current->next_link;
              }
              checkref( current );

              current->next_link = NULL;
              current->type      = *buf;
            } /* action */

/*--------------------------------------------------------------------*/
/*    Get the UNIX and local filenames for data file to be            */
/*    processed; we always report the original file name, but we      */
/*    always look at and/or update the spool copy of the data; we     */
/*    never alter or delete the original                              */
/*--------------------------------------------------------------------*/

            if ( this_call == SEND_CALL )
            {
               time_t created;
               long size;

               if (equal(dname, "D.0"))
                  strcpy( host, fname );
               else
                  importpath( host, dname, remote );

               created   = stater( host,  &size);

               switch  ( action )
               {
                  case JOB_STATUS:
                     current->created = created;
                     current->size    = size;

                     if ((created != -1) &&
                         (equaln(tname ,"X.",2)))  /* Execute file?   */
                     {
                        open_data( host,
                                   user,
                                   sys,
                                   current->name,
                                   sizeof current->name );
                        current->execute = KWTrue;
                     }
                     else {
                        current->execute = KWFalse;
                        strcpy( current->name , fname );
                     }
                     break;

                  case JOB_KILL:
                     if ((created != -1) && !equal(dname, "D.0"))
                     {
                        if ( REMOVE( host ) )
                        {
                           printmsg(0,"Unable to delete %s (%s) -- %s",
                                      dname,
                                      host,
                                      "may to have to be deleted by hand" );
                           printerr( host );
                        }
                        else
                           printmsg(0,"Deleted file %s (%s)", dname, host);
                     }
                     break;

                  case JOB_REFRESH:
                     if ((created != -1) && !equal(dname, "D.0"))
                        touch( host );
                     break;

                  default:
                     panic();
               } /* switch */
            }
            else if ( action == JOB_STATUS )
            {
               current->execute = KWFalse;
               strcpy( current->name , fname );
               current->created = stater( callname, &current->size);
            }

            break;

         default:
            printmsg(0,"Invalid line \"%s\" in call file \"%s\"",
                              buf, callname);
            break;
      } /* switch */
   } /* while */

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   if (ferror( stream ))
   {
      printerr( callname );
      clearerr( stream );
   }

   fclose( stream );
   return this_call;

} /* open call */

/*--------------------------------------------------------------------*/
/*    o p e n _ d a t a                                               */
/*                                                                    */
/*    Process a single execute file                                   */
/*--------------------------------------------------------------------*/

static void open_data(const char *file,
                      char *user,
                      char *sys,
                      char *command,
                      const size_t commandLen)
{
   FILE  *data_fp;
   char  data_buf[BUFSIZ];
   char  *token = "(none)";

   printmsg(1,"INSIDE OPEN_DATA(%s)",file);

   data_fp = FOPEN(file, "r", IMAGE_MODE);
   if(data_fp ==  NULL){
      printerr( file );
      panic();
   }

   while(fgets(data_buf, (int) BUFSIZ, data_fp) !=  NULL){

      size_t len = strlen( data_buf );
      if ( data_buf[ --len ] == '\n')
         data_buf[ len ] = '\0';

/*--------------------------------------------------------------------*/
/*                 Determine what kind of line it is                  */
/*--------------------------------------------------------------------*/

      switch(data_buf[0]){
         case '#':
#ifdef UDEBUG
            printmsg(5,"COMMENT %s", data_buf);
#endif
            break;

          case 'U':
            token = strtok( data_buf + 1, WHITESPACE );
            if ( token == NULL )
               break;
            strncpy( user, token , MAXL );
            user[ MAXL-1 ] = '\0';
            token = strtok( NULL , WHITESPACE );
            if ( token == NULL )
               break;
            strncpy(  sys, token , MAXL);
            sys[ MAXL-1 ] = '\0';
            printmsg(1,"SYSTEM = (%s), USER = (%s)",sys,user);
            break;

         case 'F':
            printmsg(5,"File %s", data_buf);
            break;

         case 'I':
#ifdef UDEBUG
            printmsg(5,"Input %s", data_buf);
#endif
            break;

         case 'C':
            printmsg(1,"Command %s", data_buf);
            token = strchr(data_buf,' ') + 1;
            strncpy(command, token , commandLen );
            command[ commandLen - 1 ] = '\0';
            break;

         default:
#ifdef UDEBUG
            printmsg(1,"UNKNOWN LINE %s", data_buf);
#endif
            break;

      } /* switch */
   } /* while */

   fclose(data_fp);

} /* open_data */

/*--------------------------------------------------------------------*/
/*    p r i n t _ a l l                                               */
/*                                                                    */
/*    Print all the information related to a job in the spool         */
/*--------------------------------------------------------------------*/

static void print_all(       char *job,
                             struct data_queue *current ,
                       const char *user,
                       const char *sys )
{

/*--------------------------------------------------------------------*/
/*                Loop through the files for this job                 */
/*--------------------------------------------------------------------*/

   while ( current != NULL )
   {
      struct data_queue *save_data = current->next_link;

      if ( current->execute)
         printmsg(0,"%-12s %s %c %-8.8s %-8.8s %s",
                     job,
                     dater( current->created, NULL ),
                     current->type,
                     sys,
                     user,
                     current->name );
      else
         printmsg(0,"%-12s %s %c %-8.8s %-8.8s %ld %s",
                     job,
                     dater( current->created, NULL ),
                     current->type,
                     sys,
                     user,
                     current->size,
                     current->name );

      job = "";               /* Don't print job name more than once  */
      free( current );        /* Release the abused storage           */
      current = save_data;

   } /* while */

} /* print_all */

/*--------------------------------------------------------------------*/
/*    i s _ j o b                                                     */
/*                                                                    */
/*    Determine if a file is a valid job                              */
/*                                                                    */
/*    The task of finding a proper job made is harder by the fact     */
/*    UUPC inserts all these jobs into separate directories.  We      */
/*    brute force it, checking all known hosts for the wayward        */
/*    file.  This is relatively cheap, since most hosts will fail     */
/*    on the host name and thus we never look on disk.  Since job     */
/*    sequence numbers are global across UUPC/extende, a false hit    */
/*    on the host doesn't hurt because the job number will never      */
/*    match.                                                          */
/*--------------------------------------------------------------------*/

static char *is_job(const char *callfile)
{
   struct HostTable *hostp;
   char hname[FILENAME_MAX];
   KWBoolean hit = KWFalse;

/*--------------------------------------------------------------------*/
/*                  Get the first system to process                   */
/*--------------------------------------------------------------------*/

      hostp = nexthost( KWTrue );

/*--------------------------------------------------------------------*/
/*              Begin loop to display local system                    */
/*--------------------------------------------------------------------*/

   while(hostp != BADHOST )
   {
      if (equaln(hostp->hostname, &callfile[2],
                min( strlen( hostp->hostname ), HOSTLEN)))
                              /* Right host?                          */
      {                       /* Maybe --> Look for the file           */
         importpath( hname, callfile, hostp->hostname);
         if ( !access( hname, 0 ))  /* Does the host file exist?      */
            return hostp->hostname; /* Yes --> Return success         */
         hit = KWTrue;
      }

      hostp = nexthost( KWFalse );
   } /* while */

/*--------------------------------------------------------------------*/
/*          We didn't get the host; report an error and exit          */
/*--------------------------------------------------------------------*/

   if ( hit )
      printmsg(0,"Unable to locate call file %s -- run uustat -a", callfile );
   else
      printmsg(0,"Unable to locate call file %s -- hostname may be incorrect",
               callfile);
   exit(1);
   return NULL;                  /* Make C compiler happy              */

} /* is_job */

/*--------------------------------------------------------------------*/
/*    t o u c h                                                       */
/*                                                                    */
/*    Update list modified time for a file by opening and closing it  */
/*--------------------------------------------------------------------*/

static void touch( const char *fname)
{
   printmsg(0,"touch: function not available.  Parameter was \"%s\"",
            fname);
} /* touch */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report how to use program                                       */
/*--------------------------------------------------------------------*/

static void usage( void )
{
   fputs("Usage:\tuustat\t[-m | -q | -a] [-u userid | all ] [-s system | all] [-x debug]\n\
\t\t[-k | -r ] jobid [-x debug]\n\
\t\t[-P system | all] [-x debug]\n\n\
\tDefault is to display jobs queued by current user\n\
\t-a\t\tDisplays all jobs for all users\n\
\t-k jobid\tKills job \"jobid\"\n\
\t-m\t\tDisplays accessability of known systems\n\
\t-q\t\tDisplays summary of queues for systems with work\n\
\t-r jobid\tRefreshes timestamps on job \"jobid\"\n\
\t-s system\tRestricts output to system (Ignored by -k -P -r)\n\
\t-u userid\tRestricts output to userid (Ignored by -k -m -P -q -r)\n\
\t-P system\tQueues dummy job for system \"system\" to force poll\n\
\t-x debuglevel",
            stdout );
   exit(1);
} /* usage */
