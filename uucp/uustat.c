/*--------------------------------------------------------------------*/
/*    u u s t a t . c                                                 */
/*                                                                    */
/*    Job status report for UUPC/extended                             */
/*                                                                    */
/*                                                                    */
/*    Copyright 1988 (C), Dewey Coffman                               */
/*    Changes Copyright 1991 (C), Andrew H. Derbyshire                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*         System include files                                       */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*         Local include files                                        */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dater.h"
#include "export.h"
#include "getopt.h"
#include "getseq.h"
#include "hlib.h"
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
#define MAXL      128      // MAX LINE LENGTH

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


struct data_queue {
   char name[FILENAME_MAX];
   struct data_queue *next_link;
   time_t created;
   long size;
   boolean execute;
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
      "Never called",
      "Dialing now",
      "Invalid device or speed in SYSTEMS file",
      "Device not available",
      "Conversation start-up failed",
      "Talking",
      "Callback required",
      "Modem initialization script failed",
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
                            char *command);

static void poll(const char *callee);

static void print_all(       char *job,
                       struct data_queue *current,
                       const char *user,
                       const char *sys);

static void refresh_job(const char *s);

static void touch( const char *fname );

static void usage( void );

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

void main(int  argc, char  **argv)
{
   int c;
   extern char *optarg;
   extern int   optind;
   COMMAND_CLASS command = LIST_DEFAULT;

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

   tzset();                      // Set up time zone information
   PushDir( E_spooldir );
   atexit( PopDir );

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

   while ((c = getopt(argc, argv, "amqk:r:s:u:x:P:")) !=  EOF)
      switch(c) {
      case 'a':
         command = LIST_ALL;
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

/*--------------------------------------------------------------------*/
/*                 Determine if we have a valid host                  */
/*--------------------------------------------------------------------*/

   if( (system != NULL) && !equal( system , ALL ) )
   {
      struct HostTable *hostp = checkreal( system );

      if (hostp  ==  BADHOST)
      {
         printf("Unknown host \"%s\", program terminating.\n",
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
         poll( system );
         break;

      default:
         panic();

   } /* switch */
   exit(0);

} /* main */

/*--------------------------------------------------------------------*/
/*    a l l                                                           */
/*                                                                    */
/*    Report on all systems                                           */
/*--------------------------------------------------------------------*/

void all( const char *system, const char *userid)
{
   char  canon[FILENAME_MAX];
   long  size, ltime;
   struct HostTable *hostp;
   boolean hit = FALSE;

   if ( equal(system,ALL) )
      hostp = nexthost( TRUE );
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
         boolean display = equali( userid, ALL );
         struct data_queue *data_link = NULL;
         char user[MAXL];
         char sys[MAXL];

         strcpy(user,userid);       // Nice default for the user
                                    /* Generates more output if
                                       straight copies in the queue  */
         strcpy(sys, E_nodename);   // Nice default for node as well

         printmsg(1,"ALL(%s)", fname);
         exportpath(canon, fname, hostp->hostname);

/*--------------------------------------------------------------------*/
/*               Determine what kind of Call file it is               */
/*--------------------------------------------------------------------*/

         switch(open_call(fname, hostp->hostname,
                           &data_link, user, sys, JOB_STATUS))
         {
            case POLL_CALL:
               if ( display )
               {
                  hit = TRUE;
                  printf( "%s %s %s\n",canon+2,dater(ltime, NULL),
                        "(POLL)");
               }
               break;

            case SEND_CALL:
            case RECEIVE_CALL:
               if( equal(userid , ALL) || equali(userid, user))
                  display = TRUE;

               if(display)
               {
                   hit = TRUE;
                   print_all( canon + 2, data_link, user,
                              hostp->hostname );
               }
               break;
         } /* switch */

      } /* while */

/*--------------------------------------------------------------------*/
/*    If processing all hosts, step to the next host in the queue     */
/*--------------------------------------------------------------------*/

      if( equal( system , ALL ))
         hostp = nexthost( FALSE );
      else
         hostp = BADHOST;

   } /* while */

   if ( !hit )
      printf("uustat: No jobs queued for system %s by user %s\n",
               system , userid );

} /* all */

/*--------------------------------------------------------------------*/
/*    p o l l                                                         */
/*                                                                    */
/*    Write a dummy call file to request a poll of a host             */
/*--------------------------------------------------------------------*/

static void poll(const char *callee)
{

   char tmfile[15];           // Call file, UNIX format name
   char msname[FILENAME_MAX];
   FILE *stream;
   static char *spool_fmt = SPOOLFMT;              // spool file name
   struct HostTable *hostp;

/*--------------------------------------------------------------------*/
/*              Determine first (only?) host to process               */
/*--------------------------------------------------------------------*/

   if ( equal(callee,ALL) )
      hostp = nexthost( TRUE );
   else
      hostp = checkreal( callee );

/*--------------------------------------------------------------------*/
/*                  Scan one or all host directories                  */
/*--------------------------------------------------------------------*/

   while  (hostp !=  BADHOST )
   {
      printmsg(1,"POLL(%s)", hostp->hostname);
      sprintf(tmfile,"%.8s",hostp->hostname);

      if ( ValidDOSName( tmfile, FALSE ) || !equal(callee, ALL))
      {
         sprintf(tmfile, spool_fmt, 'C', hostp->hostname,'Z' ,
                  "000");
         importpath( msname, tmfile, hostp->hostname);

         if ( access( msname , 0 ))   // Does the host file exist?
         {
            stream = FOPEN( msname ,"w", BINARY_MODE);

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
         hostp = nexthost( FALSE );
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
   boolean hit = FALSE;
   time_t ltime;
   long size;
   char buf[BUFSIZ];

   HostStatus();              // Load the host status table info

/*--------------------------------------------------------------------*/
/*                  Get the first system to process                   */
/*--------------------------------------------------------------------*/

   if ( equal(system,ALL) )
      hostp = nexthost( TRUE );
   else
      hostp = checkreal( system );

/*--------------------------------------------------------------------*/
/*              Begin loop to display status of systems               */
/*--------------------------------------------------------------------*/

   while(hostp != BADHOST )
   {
      char fname[FILENAME_MAX];
                              // Get list of files in the directory
      size_t jobs = 0;        // Declare, reset counter
      time_t oldest_file = now;  // Make the "oldest" file new

/*--------------------------------------------------------------------*/
/*           Inner loop to count files and determine oldest           */
/*--------------------------------------------------------------------*/

      while( readnext(fname, hostp->hostname, "C", NULL, &ltime, &size) != NULL )
      {
         if ((ltime > -1) && (ltime < oldest_file ))
            oldest_file = ltime;

         jobs++;

      } /* while */

/*--------------------------------------------------------------------*/
/*  We have all the information for this system; summary and display  */
/*--------------------------------------------------------------------*/

      if ( jobs > 0 )
      {
         if (oldest_file + DAY < now)     // File older than 24 hours?
            sprintf( buf , "(%d)", (now - oldest_file) / DAY );
                                          // Yes --> Format info
         else
            *buf = '\0';                  // No --> No, empty display

         printf("%-8.8s  %3dC%-4s  %s  %s\n", hostp->hostname, jobs , buf,
               dater( hostp->hstats->lconnect , NULL ),
               hostp->hstatus < last_status ?
                     host_status[ hostp->hstatus ] :
                     "*** INVALID/UNDOCUMENTED STATUS ***");
         hit = TRUE;

      } /* if ( jobs > 0 ) */

      if (equal(system, ALL))
         hostp = nexthost( FALSE );
      else
         hostp = BADHOST;
   } /* while */

   if ( !hit )
      printf("uustat: No jobs queued for system %s\n", system );
} /* long_stats */

/*--------------------------------------------------------------------*/
/*    s h o r t _ s t a t s                                           */
/*                                                                    */
/*    Report access to a system                                       */
/*--------------------------------------------------------------------*/

static void short_stats( const char *system )
{
   struct HostTable *hostp;

   HostStatus();              // Load the host status table info

/*--------------------------------------------------------------------*/
/*                  Get the first system to process                   */
/*--------------------------------------------------------------------*/

   if ( equal(system,ALL) )
      hostp = nexthost( TRUE );
   else
      hostp = checkreal( system );

/*--------------------------------------------------------------------*/
/*              Begin loop to display status of systems               */
/*--------------------------------------------------------------------*/

   while(hostp != BADHOST )
   {
      printf("%-8.8s  %s  %s\n", hostp->hostname,
            dater( hostp->hstats->lconnect , NULL ),
            hostp->hstatus < last_status ?
                  host_status[ hostp->hstatus ] :
                  "*** INVALID/UNDOCUMENTED STATUS ***");

      if (equal(system, ALL))
         hostp = nexthost( FALSE );
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
   char *system;              // System name returned by is_job()
   char host[FILENAME_MAX];
   char canon[FILENAME_MAX];
   char user[FILENAME_MAX];
   char sys[FILENAME_MAX];

   strcpy(canon,"C.");
   strcat(canon,jobid);
   strcpy( user, "uucp");
   system = is_job( canon ); // Never returns if an error occurs
   importpath( host, canon, system );
                              // Get the local name of the file
   open_call(host, system, NULL, user, sys, JOB_KILL);
   unlink( host );
   printf("Deleted file %s (%s)\n", canon, host);
   printf("Killed job %s (%s) queued for host %s by %s\n",
            jobid, host, system, user);

} /* kill_job */

/*--------------------------------------------------------------------*/
/*    r e f r e s h _ j o b                                           */
/*                                                                    */
/*    Refresh a job in the spool                                      */
/*--------------------------------------------------------------------*/

static void refresh_job(const char *jobid)
{
   char *system;              // System name returned by is_job()
   char host[FILENAME_MAX];
   char canon[FILENAME_MAX];
   char user[FILENAME_MAX];
   char sys[FILENAME_MAX];

   strcpy(canon,"C.");
   strcat(canon,jobid);
   strcpy( user, "uucp");
   system = is_job( canon ); // Never returns if an error occurs
   importpath( host, canon, system );
                              // Get the local name of the file
   open_call(host, system, NULL, user, sys, JOB_REFRESH);
   touch( host );
   printf("Rejuvenated job %s (%s) queued for host %s by %s\n",
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
            this_call = *type;

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
                         (equaln(tname ,"X.",2)))  // Execute file?
                     {
                        open_data( host, user, sys, current->name );
                        current->execute = TRUE;
                     }
                     else {
                        current->execute = FALSE;
                        strcpy( current->name , fname );
                     }
                     break;

                  case JOB_KILL:
                     if ((created != -1) && !equal(dname, "D.0"))
                     {
                        unlink( host );
                        printf("Deleted file %s (%s)\n", dname, host);
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
               current->execute = FALSE;
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
                      char *command)
{
   FILE  *data_fp;
   char  data_buf[BUFSIZ];
   char  *token = "(none)";
   static char f_name[ FILENAME_MAX ];
   size_t  bytes = 0;

   printmsg(1,"INSIDE OPEN_DATA(%s)",file);
   *f_name = '\0';

   printmsg(1,"OPENING(%s)",file);
   data_fp = FOPEN(file, "r", BINARY_MODE);
   if(data_fp ==  NULL){
      printerr( file );
      panic();
   }
   printmsg(1,"OPEN (%s) SIZE (%d)", file, BUFSIZ);

   while(fgets(data_buf, (int) BUFSIZ, data_fp) !=  NULL){

      size_t len = strlen( data_buf );
      bytes += strlen( data_buf );
      if ( data_buf[ --len ] == '\n')
         data_buf[ len ] = '\0';

/*--------------------------------------------------------------------*/
/*                 Determine what kind of line it is                  */
/*--------------------------------------------------------------------*/

      switch(data_buf[0]){
         case '#':
         printmsg(5,"COMMENT %s", data_buf);
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
            token = strtok( data_buf + 1 , WHITESPACE );
            strncpy( f_name, token , FILENAME_MAX );
            f_name[ FILENAME_MAX - 1 ] = '\0';
            break;

         case 'I':
            printmsg(5,"Input %s", data_buf);
            break;

         case 'C':
            printmsg(1,"Command %s", data_buf);
            token = strchr(data_buf,' ') + 1;
            strncpy(command, token , FILENAME_MAX - 1);
            command[ FILENAME_MAX - 1] = '\0';
            break;
         default:
            printmsg(1,"UNKNOWN LINE %s", data_buf);
            break;
      } /* switch */
   } /* while */

   printmsg(1,"CLOSED (%s), bytes = %d, d file = %s", file, bytes, token);
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
   char blanks[FILENAME_MAX];
   size_t subscript = 0;

/*--------------------------------------------------------------------*/
/*  Create a blank buffer for printing all of the first line of data  */
/*--------------------------------------------------------------------*/

   while ( job[subscript] != '\0')
      blanks[subscript++] = ' ';
   blanks[ subscript ] = '\0';

/*--------------------------------------------------------------------*/
/*                Loop through the files for this job                 */
/*--------------------------------------------------------------------*/

   while ( current != NULL )
   {
      struct data_queue *save_data = current->next_link;
      if ( current->execute)
         printf("%s %s %c %-8.8s %-8.8s %s\n", job,
                     dater( current->created, NULL ),
                     current->type,
                     sys, user, current->name );
      else
         printf("%s %s %c %-8.8s %-8.8s %ld %s\n", job,
                     dater( current->created, NULL ),
                     current->type,
                     sys, user, current->size , current->name );

      job = blanks;           // Don't print job name more than once
      free( current );        // Release the abused storage
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
   char host[FILENAME_MAX];
   boolean hit = FALSE;

/*--------------------------------------------------------------------*/
/*                  Get the first system to process                   */
/*--------------------------------------------------------------------*/

      hostp = nexthost( TRUE );

/*--------------------------------------------------------------------*/
/*              Begin loop to display local system                    */
/*--------------------------------------------------------------------*/

   while(hostp != BADHOST )
   {
      if (equaln(hostp->hostname, &callfile[2],
                min( strlen( hostp->hostname ), HOSTLEN)))
                              // Right host?
      {                       // Maybe --> Look for the file
         importpath( host, callfile, hostp->hostname);
         if ( !access( host, 0 ))   // Does the host file exist?
            return hostp->hostname; // Yes --> Return success
         hit = TRUE;
      }

      hostp = nexthost( FALSE );
   } /* while */

/*--------------------------------------------------------------------*/
/*          We didn't get the host; report an error and exit          */
/*--------------------------------------------------------------------*/

   if ( hit )
      printf("Unable to locate call file %s -- run uustat -a\n", callfile );
   else
      printf("Unable to locate call file %s -- hostname may be incorrect\n",
               callfile);
   exit(1);
   return NULL;                  // Make C compiler happy
} /* is_job */

/*--------------------------------------------------------------------*/
/*    t o u c h                                                       */
/*                                                                    */
/*    Update list modified time for a file by opening and closing it  */
/*--------------------------------------------------------------------*/

static void touch( const char *fname)
{
   printf("touch: function not available.  Parameter was \"%s\"\n",
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
