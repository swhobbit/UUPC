/*--------------------------------------------------------------------*/
/*       r m a i l . c                                                */
/*                                                                    */
/*       Delivery agent for UUPC/extended                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: rmail.c 1.76 2000/05/12 12:35:45 ahd v1-13g $
 *
 *    $Log: rmail.c $
 *    Revision 1.76  2000/05/12 12:35:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.75  1999/01/11 05:43:36  ahd
 *    Remove special case SMTP code (most of it)
 *
 *    Revision 1.74  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.73  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.72  1998/11/01 20:38:15  ahd
 *    Don't generate duplicate Message-IDs in -t order
 *
 *    Revision 1.71  1998/08/29 14:17:11  ahd
 *    Add setTitle() commands to report delivery progress
 *
 * Revision 1.70  1998/04/19  15:30:08  ahd
 * Relax error traps for UUCP from line
 *
 *    Revision 1.69  1998/03/16 07:47:40  ahd
 *    Correct definition of LOCAL_BUFSIZ
 *
 *    Revision 1.68  1998/03/16 06:39:32  ahd
 *    Add trumpet remote user support
 *
 *    Revision 1.67  1998/03/08 23:07:12  ahd
 *    Better support of remote vs. local source of mail
 *
 *    Revision 1.66  1998/03/01 01:32:49  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.65  1997/12/15 02:33:58  ahd
 *    Cleanup OS/2 compiler errors
 *
 *    Revision 1.64  1997/12/14 00:43:16  ahd
 *    Further cleanup of new sender protocol
 *
 *    Revision 1.63  1997/12/13 18:05:06  ahd
 *    Change parsing and passing of sender address information
 *
 *    Revision 1.62  1997/11/30 04:21:39  ahd
 *    Delete older RCS log comments, force full address for SMTP delivery,
 *    recongize difference between local and remote delivery
 *
 *    Revision 1.61  1997/11/29 12:59:50  ahd
 *    Suppress compiler warnings
 *
 *    Revision 1.60  1997/11/25 05:05:06  ahd
 *    More robust SMTP daemon
 *
 *    Revision 1.59  1997/11/24 02:52:26  ahd
 *    First working SMTP daemon which delivers mail
 *
 *    Revision 1.58  1997/06/03 03:25:31  ahd
 *    First compiling SMTPD
 *
 *    Revision 1.57  1997/05/11 18:15:50  ahd
 *    Allow faster SMTP delivery via fastsmtp flag
 *    Move TCP/IP dependent code from rmail.c to deliver.c
 *    Allow building rmail without SMTP or TCP/IP support
 *
 *    Revision 1.56  1997/05/11 04:27:40  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.55  1997/05/03 17:10:18  ahd
 *    Delete erronous check for null fromNode buffer
 *
 *    Revision 1.54  1997/04/24 01:10:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.53  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.52  1996/03/19 03:37:44  ahd
 *    Pass in previous state of header parsing (header vs. body)
 *    when copying remainder of file to holding tank.
 *
 *    Revision 1.51  1996/01/20 13:00:29  ahd
 *    Specify text/binary mode when opening in-memory files
 *    Correctly trap incomplete From line on mail in rmail mode
 *
 *    Revision 1.50  1996/01/01 21:03:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.49  1995/11/30 03:06:56  ahd
 *    Trap truly invalid addresses in tokenizer
 *
 *    Revision 1.48  1995/09/11 00:20:45  ahd
 *    Add debugging information to X.* files
 *    Use "--" option to insure user names with leading dash don't
 *    cause problems.
 *
 *    Revision 1.47  1995/09/04 18:43:37  ahd
 *    Correctly perform host lookup even if verification of sender is
 *    disabled
 *
 *    Revision 1.46  1995/07/21 13:23:19  ahd
 *    Correct wildcard routing for local host to reject mail not actually
 *    destined to us.
 *
 *    Revision 1.45  1995/03/23 01:42:52  ahd
 *    Handle empty forward files which result in no delivery more gracefully
 *
 *    Revision 1.44  1995/03/12 16:42:24  ahd
 *    Suppress compiler warnings
 *
 *    Revision 1.43  1995/03/11 22:28:40  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.42  1995/01/28 22:07:13  ahd
 *    Correctly trap no From line on remote mail
 *
 *    Revision 1.41  1995/01/15 19:48:35  ahd
 *    Allow active file to be optional
 *    Delete fullbatch global option
 *    Add "local" and "batch" flags to SYS structure for news
 *
 *    Revision 1.40  1995/01/09 01:39:22  ahd
 *    Optimize UUCP processing for remote mail, break out logical
 *    queuing from actually writing the files, and don't write call
 *    file (which UUCICO could see by mistake) until we're writing
 *    it for the final time.
 *
 *    Revision 1.39  1995/01/08 21:02:02  ahd
 *    Correct BC++ 3.1 compiler warnings
 *
 *    Revision 1.38  1995/01/08 19:52:44  ahd
 *    Add in memory files to RMAIL, including additional support and
 *    bug fixes.
 *
 *    Revision 1.37  1995/01/07 17:35:06  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 */

/*--------------------------------------------------------------------*/
/*    Function:   Stand alone mail delivery module for                */
/*                UUPC/extended                                       */
/*    Language:   Borland C++ 3.1 (ANSI C mode) or Microsoft C 6.0.   */
/*    Arguments:  One or more addresses to deliver mail to            */
/*                or "-t" to direct rmail to read the addresses       */
/*                from the RFC-822 header.                            */
/*                A third mode of operation is to specify '-w' and/   */
/*                or '-s subject' followed by one or more addresses   */
/*                with optional carbon copy flags (-c or -b) before   */
/*                additional addresses.  This causes rmail to         */
/*                function like a bare bones batch version of the     */
/*                MAIL program; a valid RFC-822 header is generated   */
/*                and the mail is delivered, aliases are not expanded */
/*                and the mail is not locally logged.  (The current   */
/*                user id as the from address is used unless the      */
/*                environment variable LOGNAME is set, in which case  */
/*                it is used.)                                        */
/*                Optional argument "-f" to denote file to read in    */
/*                place of stdin.                                     */
/*                Optional argument "-F" to denote file to read in    */
/*                place of stdin and DELETE after readering.          */
/*                Optional argument "-x" to for debug level           */
/*    Input:      mail to be delivered, with RFC-822 header, on       */
/*                stdin.                                              */
/*    Output:     'From' and 'Received:' headers are added,           */
/*                'Bcc:' headers are removed, and the mail is         */
/*                delivered to one or more local users and/or         */
/*                one or more remote users.                           */
/*    Exit code:  0  Success                                          */
/*                1  One or more letters not delivered                */
/*                2  No mail delivered                                */
/*                3 Configuration file error                          */
/*                4 Invalid option/help specified                     */
/*                5 Input/output error                                */
/*                6 Input/output error                                */
/*                7 Input/output error                                */
/*                                                                    */
/*    Note:       The "-t" flag is supported by BSD sendmail for the  */
/*                purpose listed above, but we also turn use it to    */
/*                control other special options, all of which         */
/*                basically cause the program to act more like a      */
/*                local mailer than a remote mailer; these options    */
/*                include:                                            */
/*                                                                    */
/*                      Stripping off blind carbon copies             */
/*                                                                    */
/*                      Generating the UUCP From line differently     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <signal.h>

#ifdef _Windows
#include <windows.h>
#endif

#include <windows.h>

/*--------------------------------------------------------------------*/
/*                     Application include files                      */
/*--------------------------------------------------------------------*/

#include "address.h"
#include "arpadate.h"
#include "deliver.h"
#include "getopt.h"
#include "hostable.h"
#include "logger.h"
#include "security.h"
#include "usertabl.h"
#include "timestmp.h"
#include "catcher.h"
#include "execute.h"
#include "title.h"

#ifdef _Windows
#include "winutil.h"
#endif


/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#define LOCAL_BUFSIZ BUFSIZ

/*--------------------------------------------------------------------*/
/*                   Prototypes for internal files                    */
/*--------------------------------------------------------------------*/

static KWBoolean CopyTemp(IMFILE *imf,
                           FILE *datain,
                           KWBoolean header);

static void ParseFrom(
                       MAIL_ADDR *sender,
                       const char *forwho,
                       IMFILE *imf,
                       FILE *datain);

static char **Parse822(
                        MAIL_ADDR *sender,
                        KWBoolean *header,
                        size_t *count,
                        IMFILE *imf,
                        FILE *datain);

static void Terminate(const int rc, IMFILE *imf, FILE *datain);

 static void PutHead(const char *label,
                      const char *operand,
                      IMFILE *imf,
                      const KWBoolean resent);

static KWBoolean DaemonMail(
                           MAIL_ADDR *sender,
                           const char *subject,
                           char **address,
                           size_t count,
                           IMFILE *imf);

 static void usage(void);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

RCSID("$Id: rmail.c 1.76 2000/05/12 12:35:45 ahd v1-13g $");

 static char received[] = "Received:";
 static char receivedlen = sizeof(received) - 1;

/*--------------------------------------------------------------------*/
/*                            main program                            */
/*--------------------------------------------------------------------*/

int main(int argc, char **argv)
{
   KWBoolean readHeader = KWFalse;  /* True = Parse RFC-822 header     */

   MAIL_ADDR sender;                /* Source mailbox of message       */
   int  option;                     /* For parsing option list         */
   int  tempHandle;                 /* For redirecting stdin           */
   char **address;                  /* Pointer to list of target
                                          addresses                   */
   size_t addressees;               /* Number of delivery address      */
   size_t count;                    /* Loop variable for delivery      */
   size_t delivered = 0;            /* Count of successful deliveries  */
   int user_debug  = -1;
   KWBoolean inHeader = KWTrue;     /* Assume terminated header        */
   KWBoolean DeleteInput = KWFalse;

   KWBoolean daemonMode = KWFalse;
   KWBoolean queueMode = KWFalse;

   char *subject = NULL;
   char *logname = NULL;
   char *namein = CONSOLE;

   FILE *datain = NULL;             /* Input mail stream               */
   IMFILE  *imf = NULL;             /* Temporary storage stream        */

/*--------------------------------------------------------------------*/
/*    Make a copy of the Borland copyright for debugging purposes     */
/*--------------------------------------------------------------------*/

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   logfile = stderr;                /* Prevent redirection of error
                                          msgs during configuration   */

   myProgramName = newstr(argv[0]); /* Copy before banner() mangles    */

   setDeliveryGrade(E_mailGrade);   /* Get grade from configuration    */
   memset( &sender, 0, sizeof sender); /* Set fields to defaults       */

/*--------------------------------------------------------------------*/
/*       Load the UUPC/extended configuration file, and exit if       */
/*       any errors                                                   */
/*--------------------------------------------------------------------*/

   banner(argv);

   debuglevel =  0;

   if (!configure(B_RMAIL))
      Terminate(3, imf, datain);

   datain = stdin;

/*--------------------------------------------------------------------*/
/*                       Begin logging messages                       */
/*--------------------------------------------------------------------*/

   openlog(logname);

/*--------------------------------------------------------------------*/
/*                      Parse our operand flags                       */
/*--------------------------------------------------------------------*/

   while ((option = getopt(argc, argv, "qg:ws:tF:f:l:x:")) != EOF)
   {
      switch (option)
      {

      case 'F':
         DeleteInput = KWTrue;
         /* Fall through to regular file name choice */

      case 'f':
         namein = optarg;
         break;

      case 'g':
         if (isalnum(*optarg) && (strlen(optarg) == 1))
            setDeliveryGrade(*optarg);
         else {
            printmsg(0,"Invalid grade for mail: %s", optarg);
            usage();
         }
         break;

      case 'q':
         queueMode = KWTrue;
         break;

      case 'l':
         openlog(optarg);
         break;

      case 's':
         subject = optarg;
         daemonMode = KWTrue;
         break;

      case 't':
         readHeader = KWTrue;
         break;

      case 'w':
         daemonMode = KWTrue;
         break;

      case 'x':
         user_debug = debuglevel = atoi(optarg);
         break;

      case '?':
         usage();
         break;

      } /* switch */
   } /* while */

/*--------------------------------------------------------------------*/
/*                    Handle control-C interrupts                     */
/*--------------------------------------------------------------------*/

    if(signal(SIGINT, ctrlchandler) == SIG_ERR)
    {
        printmsg(0, "Couldn't set SIGINT");
        panic();
    }

   if (debuglevel > 1)
   {
      for (count = 1; (int) count < argc; count ++)
         printmsg(4,"rmail argv[%d] = \"%s\"", count, argv[count]);
   } /* if (debuglevel > 4) */

   if ((optind == argc) != (int) readHeader)
   {
      printmsg(0,"Missing/extra parameter(s) at end.");
      usage();
   }

   checkname(E_nodename);           /* Force loading of E_fdomain      */

#if defined(_Windows)
   atexit(CloseEasyWin);            /* Auto-close EasyWin on exit      */
#endif

   /* Never deliver immediately if called from another daemon  */
   if (daemonMode)
      bflag[F_FASTSMTP] = KWFalse;

   /* Exception ... always deliver immediatelt if queue mode */
   if (queueMode)
      bflag[F_FASTSMTP] = KWTrue;

/*--------------------------------------------------------------------*/
/*    If in local mode and the user doesn't want output, suppress     */
/*    routine delivery messages                                       */
/*--------------------------------------------------------------------*/

   if ((user_debug == -1) && (debuglevel == 0))
   {
      if (daemonMode || ! readHeader)
         debuglevel = 1;
      else
         debuglevel = (int) bflag[F_VERBOSE];
   }

/*--------------------------------------------------------------------*/
/*               Verify we have input stream available                */
/*--------------------------------------------------------------------*/

   if (! equal(namein, CONSOLE))
      datain = FOPEN(namein, "r",TEXT_MODE);

   if (datain == NULL)
   {
      printerr(namein);
      Terminate(6, imf, datain);
   } /* if */

/*--------------------------------------------------------------------*/
/*                   Open up the output data stream                   */
/*--------------------------------------------------------------------*/

   fflush(datain);
   imf = imopen(filelength(fileno(datain)) + 512, TEXT_MODE);

   if (imf == NULL)
   {
      printerr("imopen");
      Terminate(5, imf, datain);
   } /* if */

/*--------------------------------------------------------------------*/
/*   If in local mail mode, make up a list of addresses to mail to    */
/*--------------------------------------------------------------------*/

   if (daemonMode)
   {
      addressees = (unsigned int) (argc - optind);
      address = &argv[optind];
      DaemonMail(&sender, subject, address, addressees, imf);
      inHeader = KWFalse;
   }
   else if (readHeader)
      address = Parse822(&sender, &inHeader, &addressees, imf, datain);
   else {
      addressees = (unsigned int) (argc - optind);
      address = &argv[optind];
      ParseFrom(&sender,
                 addressees > 1 ? "multiple addressees" : *address,
                 imf,
                 datain);           /* Copy remote header instead      */
   } /* if */

   if (!address || ! addressees)    /* Can we deliver mail?            */
   {
      printmsg(0, "No addressees to deliver to!");
      Terminate(2, imf, datain);    /* No --> Punt formation           */
   }

/*--------------------------------------------------------------------*/
/*                    Validate the sender address                     */
/*--------------------------------------------------------------------*/

   if ((sender.host == NULL) || !strlen(sender.host))
   {
      printmsg(0,"rmail: Cannot parse sender host (result was %s)",
                 (sender.host == NULL) ? "NULL" : "empty" );
      sender.host = "not-for-mail.UUCP";
   }

   if ((sender.user == NULL) || !strlen(sender.user))
   {
      printmsg(0,"rmail: Cannot parse sender user (result was %s)",
                 (sender.user == NULL) ? "NULL" : "empty" );
      sender.user = "nobody";
   }

#ifdef UDEBUG
   printmsg( 4,"rmail: Sender is %s (%s at %s via %s)",
            sender.address,
            sender.user,
            sender.host,
            (sender.relay == NULL) ? "*local*" : sender.relay );
#endif

/*--------------------------------------------------------------------*/
/*       Copy the rest of the input file into our holding tank        */
/*--------------------------------------------------------------------*/

   if (! CopyTemp(imf, datain, inHeader))
      inHeader = KWFalse;

   if (inHeader)                    /* Was header ever terminated?     */
   {
      printmsg(0,"rmail: Improper header, adding trailing newline");
      imputc('\n', imf);            /* If not, it is now ...           */
   }

/*--------------------------------------------------------------------*/
/*       We always make sure standard input is open to a null         */
/*       device because execute will redirect it, perhaps             */
/*       improperly if we close it here and another routine (like     */
/*       the alias functions) opens a file and gets file handler      */
/*       zero.                                                        */
/*                                                                    */
/*       (Rich Gumpertz warned me about this one, and I ignored him.  */
/*       Ooops.)                                                      */
/*--------------------------------------------------------------------*/

   if (stdin != datain)             /* Insure input is closed too      */
      fclose(datain);

   if (DeleteInput)                 /* Make room for data on disk      */
      REMOVE(namein);

   if ((tempHandle = open(BIT_BUCKET, O_RDONLY | O_BINARY)) == -1)
   {
     printerr(BIT_BUCKET);          /* Aw, heck, bit bucket is full?   */
     panic();                       /* If we can't open it, kick it    */
   }
   else if (dup2(tempHandle, 0))    /* Swap stdin to empty input       */
   {
       printerr("dup2");
       panic();
   }

   close(tempHandle);               /* Don't need original handle      */

/*--------------------------------------------------------------------*/
/*                    Perform delivery of the mail                    */
/*--------------------------------------------------------------------*/

   for (count = 0; count < addressees; count++)
   {
         if (*address[count] == '\0')
            delivered ++;           /* Ignore optn flags on delivery   */
         else
            delivered += Deliver(imf, &sender, address[count], KWTrue);
   }

   setTitle("Cleaning up");

   flushQueues(imf, &sender );

/*--------------------------------------------------------------------*/
/*                       Terminate the program                        */
/*--------------------------------------------------------------------*/

   printmsg(8,"rmail: %d addressees, delivered to %d mailboxes",
            addressees, delivered);

   if (delivered >= addressees)
      Terminate(0, imf, datain);    /* All mail delivered              */
   else if (delivered == 0)
   {
      printmsg(0,"Unable to deliver/bounce to any addresses!");
      Terminate(2, imf, datain);    /* No mail delivered               */
   }
   else {
      printmsg(0,"Unable to deliver/bounce to all addresses!");
      Terminate(1, imf, datain);    /* Some mail delivered             */
   }

   return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*    T e r m i n a t e                                               */
/*                                                                    */
/*    Cleanup open files and return to operating system               */
/*--------------------------------------------------------------------*/

static void Terminate(const int rc, IMFILE *imf, FILE *datain)
{
   if ((datain != stdin) && (datain != NULL))
      fclose(datain);

   if (imf != NULL)
      imclose(imf);

   exit(rc);                        /* Return to operating systems     */

}  /* Terminate */

/*--------------------------------------------------------------------*/
/*    P a r s e F r o m                                               */
/*                                                                    */
/*    Read the from address of incoming data from UUCP                */
/*--------------------------------------------------------------------*/

static void ParseFrom(
                        MAIL_ADDR *sender,
                        const char *forwho,
                        IMFILE *imf,
                        FILE *datain)
{
   static const char mName[] = "ParseFrom";
   static const char from[] = "From ";
   static const char remote[] = "remote from ";
   static const size_t remoteLen = sizeof remote - 1;
   static const size_t fromLen = sizeof from - 1;


   char *token;
   char buf[LOCAL_BUFSIZ];
   KWBoolean hit;
   KWBoolean bWriteReceived = KWTrue;

   sender->remote = KWTrue;

   BREAKPOINT;

/*--------------------------------------------------------------------*/
/*           Use UUXQT Information for nodename, if available         */
/*--------------------------------------------------------------------*/

   sender->relay = getenv(UU_MACHINE);

   if ((sender->relay != NULL) &&
       equal(sender->relay, E_nodename)) /* Our local queue?           */
   {
      sender->relay = NULL;         /* Yes --> We don't count          */
      bWriteReceived = KWFalse;     /* Don't write a Received: line    */
   }

/*--------------------------------------------------------------------*/
/*            Now look at the UUCP From line, if it exists            */
/*--------------------------------------------------------------------*/

   if (fgets(buf, LOCAL_BUFSIZ, datain) == NULL)
   {
      printmsg(0,"ParseFrom: Input file is empty!");
      panic();
   }

   hit = equaln(buf, from, fromLen); /* true = UUCP From line          */

/*--------------------------------------------------------------------*/
/*       It's a From line.  Grab the initial address and then look    */
/*       for the final node at the end.                               */
/*--------------------------------------------------------------------*/

   if (hit && ((token = strtok(buf + fromLen, " ")) != NULL))
   {
      char fUser[MAXADDR];
      char fHost[MAXADDR];
      char *rest;

      sender->address = token;
                                    /* Get second token on line
                                       with user's address             */
      rest = strtok(NULL, "\n");    /* Get first non-blank of third
                                       token on the line               */

      if ((strlen(sender->address) + strlen(sender->address)) >=
                              MAXADDR) /* overlength addr?             */
      {
         sender->address += strlen(sender->address) - MAXADDR;
                                    /* Reduce to length we can handle  */
         token = strchr(sender->address, '!');

         if (token != NULL)
            sender->address = token + 1; /* Step past truncated node   */

      } /* if */

      if (tokenizeAddress(sender->address, NULL, fHost, fUser))
      {
         sender->address = newstr(sender->address);
         sender->user = newstr(fUser);

         if (equali(HostAlias(E_nodename), HostAlias(fHost)))
         {
            char address[MAXADDR];
            sender->host =  E_domain; /* Use long local name           */
            sprintf(address, "%s@%s", sender->user, sender->host);
            sender->address = newstr(address);
            sender->relay = NULL;   /* Flag address as local           */
         }
         else
            sender->relay = sender->host = newstr(fHost);
      }
      else {

         printmsg(0, "%s: Cannot parse address %s: %s",
                   mName,
                   sender->address,
                   fHost);

         if (sender->relay == NULL)
         {
            char address[MAXADDR];
            sender->host = E_domain;
            sender->user = POSTMASTER;
            sprintf(address, "%s@%s", sender->user, sender->host);
            sender->address = newstr(address);
         }

      } /* else */

/*--------------------------------------------------------------------*/
/*       Make more heroic efforts to determine the fromNode.  This    */
/*       is done so programs like v-mail server which don't put on    */
/*       a remote on the From line -or- set any environment variables */
/*       still get valid information generated by deliver()           */
/*--------------------------------------------------------------------*/

      if ((sender->relay == NULL) && (rest != NULL))
      {
         token = strstr(rest, remote);

         if (token != NULL)         /* Do we have a remote on line?    */
            sender->relay = newstr(token + remoteLen);
                                    /* Yes --> Use as fromNode         */
         else if (! equal(HostAlias(E_domain),
                            HostAlias(sender->host)))
            sender->relay =  sender->host;

      } /* if (sender->relay == NULL) */

   } /* if */
   else {
      /* We really want to read the header of the message and
         find any addresses in it, but for now, we cheat!            */
      char fAddress[MAXADDR];
      sender->user = E_mailbox;
      sender->host = E_domain;
      sprintf( fAddress, "%s@%s", sender->user, sender->host );
      sender->address = newstr( fAddress );

   }

/*--------------------------------------------------------------------*/
/*             Generate required "Received" header lines              */
/*--------------------------------------------------------------------*/

   if (bWriteReceived)
   {
      imprintf(imf,"%-10s from %s by %s (%s %s) with %s\n%-10s for %s; %s\n",
               "Received:",
               sender->relay ? sender->relay : "localhost",
               E_domain,
               compilep,
               compilev,
               sender->relay ? "UUCP" : "unknown protocol",
               " ",
               forwho,
               arpadate());
    }

/*--------------------------------------------------------------------*/
/*       If what we read wasn't a From line, write it into the new    */
/*       file after the generated Received: line                      */
/*--------------------------------------------------------------------*/

   if (!hit)
   {
      imputs(buf, imf);

      if (imerror(imf))
      {
         printerr("imputs");
         Terminate(6, imf, datain);
      } /* if */

   } /* if */

}  /* ParseFrom */

/*--------------------------------------------------------------------*/
/*    P a r s e 8 2 2                                                 */
/*                                                                    */
/*    Parse an RFC-822 header generated by that esteemed mail user    */
/*    agent, UUPC/extended's MAIL.                                    */
/*                                                                    */
/*    Note that we parse the header in the format we KNOW that UUPC   */
/*    generated it in:  "To:", "Cc:", "Bcc:", optionally prefixed     */
/*    by "Resent-".  We also know that mail comes in one address      */
/*    per line, and that the Resent- headers, if any, precede the     */
/*    original headers.                                               */
/*--------------------------------------------------------------------*/

static char **Parse822(
                        MAIL_ADDR *sender,
                        KWBoolean *header,
                        size_t *count,
                        IMFILE *imf,
                        FILE *datain)
{

/*--------------------------------------------------------------------*/
/*       Define the headers we will be examining and variables for    */
/*       their lengths                                                */
/*--------------------------------------------------------------------*/

   static const char resent[] = "Resent-";
   static const size_t resentLen =  sizeof resent - 1;

   size_t offset = 0;               /* Subscript for examining headers,
                                       which allows us to ignore
                                       Resent-                         */

   size_t allocated = 5;            /* Reasonable first size for list  */
                                    /* Note: MUST BE AT LEAST 2
                                       because we add 50% below!       */

   char **addrlist = calloc(sizeof *addrlist, allocated);
   char buf[LOCAL_BUFSIZ];                /* Input buffer to read header     */
   char outputBuffer[sizeof buf+MAXADDR]; /* Output buffer for addresses   */
   char address[MAXADDR];           /* Buffer for parsed address       */
   char fHost[MAXADDR];
   char fUser[MAXADDR];
   int senderID = -1, dateID = -1, fromID = -1, messageID = -1;

   typedef struct _HEADERS
   {
      const char *text;
      char  *address;
      const KWBoolean blind;
      const KWBoolean required;
      const KWBoolean output;
      KWBoolean found;
   } HEADERS;

   KWBoolean blind = KWFalse;
   KWBoolean output = KWFalse;

   char sAddress[MAXADDR];
   char fAddress[MAXADDR];

   static HEADERS headerTable[] =
   {
      { "From:",         NULL,  KWFalse, KWTrue,  KWFalse, KWFalse },
      { "Sender:",       NULL,  KWFalse, KWFalse, KWFalse, KWFalse },
      { "To:",           NULL,  KWFalse, KWFalse, KWTrue,  KWFalse },
      { "Cc:",           NULL,  KWFalse, KWFalse, KWTrue,  KWFalse },
      { "Bcc:",          NULL,  KWTrue,  KWFalse, KWTrue,  KWFalse },
      { "Date:",         NULL,  KWFalse, KWFalse, KWFalse, KWFalse },
      { "Message-ID:",   NULL,  KWFalse, KWFalse, KWFalse, KWFalse },
      { NULL }
   };

   size_t subscript;

/*--------------------------------------------------------------------*/
/*               Determine selected subscript information             */
/*--------------------------------------------------------------------*/

   for (subscript = 0;
         headerTable[subscript].text != NULL;
         subscript++)
   {
      if (equal(headerTable[subscript].text, "Date:"))
         dateID = (int) subscript;
      else if (equal(headerTable[subscript].text, "From:"))
         fromID = (int) subscript;
      else if (equal(headerTable[subscript].text, "Sender:"))
         senderID = (int) subscript;
      else if (equal(headerTable[subscript].text, "Message-ID:"))
         messageID = (int) subscript;

      headerTable[subscript].found = KWFalse;

   } /* for */

/*--------------------------------------------------------------------*/
/*                   Initialized string information                   */
/*--------------------------------------------------------------------*/

   if ((dateID < 0) || (fromID < 0) || (senderID < 0))
      panic();

   headerTable[fromID].address   = fAddress;
   headerTable[senderID].address = sAddress;

   *outputBuffer = '\0';            /* Flag no addresses to send yet   */

/*--------------------------------------------------------------------*/
/*                          Begin processing                          */
/*--------------------------------------------------------------------*/

   *count = 0;                      /* No addresses discovered yet     */
   checkref(addrlist);              /* Verify we had room for list     */

   imprintf(imf,"%-10s by %s (%s %s);\n%-10s %s\n",
            "Received:",
            E_domain,
            compilep,
            compilev,
            " ",
            arpadate());

/*--------------------------------------------------------------------*/
/*                        Find the From: line                         */
/*--------------------------------------------------------------------*/

   while(*header && (fgets(buf, sizeof buf, datain) != NULL))
   {
      char *startAddress = buf;

      if (*buf == '\n')             /* end of the header?              */
      {
         output = *header = KWFalse; /* Yes --> reset all our flags    */
         blind = KWTrue;            /* We'll print terminator later    */
      }

/*--------------------------------------------------------------------*/
/*              Set flags whenever we find a new header               */
/*--------------------------------------------------------------------*/

      else if (isgraph(*buf))       /* Start of a new header?          */
      {
         blind = output = KWFalse;  /* Reset processing flags for this
                                    header                           */

         if (equalni(buf, resent, resentLen))  /* Msg a resend?        */
            offset = resentLen;     /* Yes --> Only use Resent- hdrs   */

/*--------------------------------------------------------------------*/
/*         Loop to find the header in our table, if possible          */
/*--------------------------------------------------------------------*/

         for (subscript = 0;
               headerTable[subscript].text != NULL;
               subscript++)
         {
            size_t headerLen = strlen(headerTable[subscript].text);

            if ((! offset || equalni(buf, resent, offset)) &&
                  equalni(buf + offset,
                          headerTable[subscript].text,
                          headerLen))
            {
               if (headerTable[subscript].found) /* Been here before?  */
               {
                  printmsg(0,"Parse822: Error: Duplicate header: %s",
                              buf);
                  return NULL;
               }

               headerTable[subscript].found = KWTrue;
               blind  = headerTable[subscript].blind;
               output = headerTable[subscript].output;

               startAddress = buf + offset + headerLen;

               if (isgraph(*startAddress))  /* Non-blank?              */
               {                    /* I'm SO confused                 */
                  printmsg(0,"Parse822: Invalid header, cannot continue: %s",
                              buf);
                  return NULL;
               }

/*--------------------------------------------------------------------*/
/*                 Save address if table requested it                 */
/*--------------------------------------------------------------------*/

               if (headerTable[subscript].address != NULL)
               {
                  ExtractAddress(headerTable[subscript].address,
                                  startAddress,
                                  ADDRESSONLY);
               }

/*--------------------------------------------------------------------*/
/*            Insert separator between addresses if needed            */
/*--------------------------------------------------------------------*/

               if (*outputBuffer && output)
                  strcat(outputBuffer, ","); /* Sep addresses          */

               break;               /* drop out of the for (;;)        */

            } /* if equalni() */

         } /* for */

      } /* else if */

/*--------------------------------------------------------------------*/
/*              Write the line out unless a blind header              */
/*--------------------------------------------------------------------*/

      if (! blind)
         imputs(buf, imf);

/*--------------------------------------------------------------------*/
/*                       Save output addresses                        */
/*--------------------------------------------------------------------*/

      if (output)
      {
         while(*startAddress && !isgraph(*startAddress))
            startAddress++;         /* Step past while space in addr   */

         printmsg(10,"Adding address [%s] to [%s]",
               startAddress,
               outputBuffer);
         strcat(outputBuffer, startAddress);
      }

/*--------------------------------------------------------------------*/
/*       Spit out any addresses if we may overflow the buffer or      */
/*       we're done collecting them.                                  */
/*--------------------------------------------------------------------*/

      while(*outputBuffer &&
             (! output || (strlen(outputBuffer) > MAXADDR)))
      {
         char *next = ExtractAddress(address,
                                      outputBuffer,
                                      ADDRESSONLY);
                                    /* Get address to add to list      */

         if (allocated == (*count+1)) /* Do we have room for addr?     */
         {
            allocated += allocated / 2; /* Choose larger array         */
            addrlist = realloc(addrlist,
                                allocated * sizeof(*addrlist));
            checkref(addrlist);     /* Verify the allocation worked    */
         } /* if */

         if (!strlen(address))
         {
            printmsg(0,"Parse822: Could not locate expected address in header");
            return NULL;
         } /* if */
         else {
            addrlist[*count] = newstr(address);
                                    /* Save permanent copy of addr     */
            printmsg(4,"address[%d]= \"%s\"",*count, address);
            *count += 1;            /* Flag we got the address         */
         } /* else */

         if (next)
            memmove(outputBuffer, next, strlen(next) + 1);
                                    /* Shift buffer up to recover space
                                    used by now parsed address        */
         else
            *outputBuffer = '\0';   /* End of addresses                */

      } /* while */

   } /* while((fgets(buf, sizeof buf, datain) != NULL) && *header) */

/*--------------------------------------------------------------------*/
/*               Now validate the information we received             */
/*--------------------------------------------------------------------*/

   if (*header)                     /* Did we receive proper header?   */
   {                                /* No --> Very bad news            */
      printmsg(0,"Parse822: Premature end of input file, message rejected");
      return NULL;
   }

   for (subscript = 0;
         headerTable[subscript].text != NULL;
         subscript++)
   {
      if (headerTable[subscript].required &&
           ! headerTable[subscript].found)
      {
         printmsg(0, "Parse822: Missing header \"%s\", cannot continue.",
                      headerTable[subscript].text);
         return NULL;
      }

   } /* for (;;) */

/*--------------------------------------------------------------------*/
/*                Fill in the sender field, if needed                 */
/*--------------------------------------------------------------------*/

   sender->host = E_fdomain;        /* Just a default for now          */

   if ( headerTable[senderID].found )
      sender->address = sAddress;
   else
      sender->address = fAddress;

   if (tokenizeAddress( sender->address, NULL, fHost, fUser))
   {
      sender->user = newstr(fUser);

      if ( isOnlyLocalAddress( sender->user ))
         sprintf( sender->address, "%s@%s", sender->user, sender->host );
      else if (!equali(HostAlias(fHost), HostAlias(E_fdomain)))
         sender->relay = sender->host = newstr(fHost);

   }
   else {
      printmsg(0, "%s: %s",
                   headerTable[senderID].found ? sAddress : fAddress,
                   fHost);
      sender->host = E_domain;
      sender->user = "##invalid##";
      sprintf( sender->address, "%s@%s", sender->user, sender->host );
   }

   sender->address = newstr( sender->address ); /* Make static copy    */

/*--------------------------------------------------------------------*/
/*                 Flag who is running the command                    */
/*--------------------------------------------------------------------*/

   sender->activeUser = E_mailbox;

/*--------------------------------------------------------------------*/
/*                     Insert a date field if needed                  */
/*--------------------------------------------------------------------*/

   if (! headerTable[dateID].found)
      PutHead("Date:", arpadate(), imf, (KWBoolean) offset);

/*--------------------------------------------------------------------*/
/*                       Generate a message-id                        */
/*--------------------------------------------------------------------*/

   if (! headerTable[messageID].found)
   {
      sprintf(buf, "<%lx.%s.%s@%s>",
              time(NULL),
              E_mailbox,
              E_nodename,
              E_domain);
      PutHead("Message-ID:", buf, imf, (KWBoolean) offset);
      PutHead(NULL, NULL, imf, KWFalse); /* Terminate header         */
   }

/*--------------------------------------------------------------------*/
/*                        Terminate the header                        */
/*--------------------------------------------------------------------*/

   PutHead(NULL, NULL, imf, KWFalse); /* End the headers               */
   imputc('\n', imf);

/*--------------------------------------------------------------------*/
/*                   Return address list to caller                    */
/*--------------------------------------------------------------------*/

   return addrlist;

} /* Parse822 */

/*--------------------------------------------------------------------*/
/*    C o p y T e m p                                                 */
/*                                                                    */
/*    Copy the un-parsed parts of a message into the holding file     */
/*--------------------------------------------------------------------*/

static KWBoolean CopyTemp(IMFILE *imf,
                           FILE *datain,
                           KWBoolean header)
{
   KWBoolean newline = KWTrue;
   char buf[LOCAL_BUFSIZ];

   while (fgets(buf, sizeof buf, datain) != NULL)
   {
      if (header)
      {
         if (*buf == '\n')
            header = KWFalse;
         else if (equalni(received, buf, receivedlen))
            hops++;
      }

      if (buf[ strlen(buf) - 1 ] == '\n')
         newline = KWTrue;
      else
         newline = KWFalse;

      if (imputs(buf, imf) == EOF)  /* I/O error?                      */
      {
         printerr("CopyTemp: imputs");
         return KWFalse;
      } /* if */

   } /* while */

   if (ferror(datain))              /* Clean end of file on input?     */
   {
      printerr("CopyTemp: fgets");
      Terminate(7, imf, datain);
   }

   if (!newline)                    /* Is file terminated properly?    */
   {
      printmsg(0, "rmail: Improperly formed message, adding final newline!");
      imputc('\n', imf);
   }

   return header;

}  /* CopyTemp */

/*--------------------------------------------------------------------*/
/*    D a e m o n M a i l                                             */
/*                                                                    */
/*    Send text in a mailbag file to address(es) specified by address */
/*--------------------------------------------------------------------*/

static KWBoolean DaemonMail(
                          MAIL_ADDR *sender,
                          const char *subject,
                          char **address,
                          size_t count,
                          IMFILE *imf)
{
   char buf[LOCAL_BUFSIZ];
   char *token;
   char *fullName;
   struct UserTable *userp;
   char *header = "To:";
   char *cc     = "Cc:";
   KWBoolean print = KWTrue;

/*--------------------------------------------------------------------*/
/*                         Validate the input                         */
/*--------------------------------------------------------------------*/

   if (count == 0)
   {
      printmsg(0,"rmail: No addresseses to deliver to!");
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                       Determine our user id                        */
/*--------------------------------------------------------------------*/

   sender->user = getenv(LOGNAME);
   if (sender->user == NULL)
      sender->user = E_mailbox;

   sender->activeUser = sender->user;
   sender->host = bflag[F_BANG] ? E_nodename : E_fdomain;
                                    /* Use full address, if possible   */

/*--------------------------------------------------------------------*/
/*              Get the name of the user, or make one up              */
/*--------------------------------------------------------------------*/

   userp = checkuser(sender->user); /* Locate user id in table         */

   if ((userp != BADUSER) &&
       (userp->realname != NULL) &&
       !equal(userp->realname, EMPTY_GCOS))
      fullName = userp->realname;
   else if (equali(sender->user, E_postmaster) ||
             equali(sender->user, POSTMASTER))
      fullName = "Postmaster";
   else if (equali(sender->user, "uucp"))
      fullName = "Unix to Unix Copy";
   else
      fullName = sender->user;      /* Dummy for formatting From:      */

   if (bflag[F_BANG])
      sprintf(buf,"%s!%s", sender->host, sender->user);
   else
      sprintf(buf, "%s@%s", sender->user, sender->host);
   sender->address = newstr( buf );

/*--------------------------------------------------------------------*/
/*    Add the boilerplate to the header:                              */
/*                                                                    */
/*       Date, From, Organization, and Reply-To                       */
/*--------------------------------------------------------------------*/

   imprintf(imf,"%-10s by %s (%s %s)\n%-10s for %s; %s\n",
              "Received:",
              E_domain,
              compilep,
              compilev,
              " ",
              count > 1 ? "multiple addressees" : *address,
              arpadate());

/*--------------------------------------------------------------------*/
/*                       Generate a message-id                        */
/*--------------------------------------------------------------------*/

   sprintf(buf, "<%lx.%s@%s>", time(NULL), E_nodename, E_domain);
   PutHead("Message-ID:", buf, imf, KWFalse);
   PutHead(NULL, NULL, imf, KWFalse);

   PutHead("Date:", arpadate(), imf, KWFalse);

   if (bflag[F_BANG])
      sprintf(buf, "(%s) %s", fullName, sender->address);
   else {
      sprintf(buf, "\"%s\" <%s>", fullName, sender->address );
   }

   PutHead("From:", buf, imf, KWFalse);

   if (E_organization != NULL)
      PutHead("Organization:", E_organization, imf, KWFalse);

/*--------------------------------------------------------------------*/
/*                      Write the address out                         */
/*--------------------------------------------------------------------*/

   while((count-- > 0) && print)
   {
      token = *address++;

      if ((token[0] == '-') &&
            isalpha(token[1]) &&
          (token[2] == '\0') &&
            print)
      {
         if (token[1] == 'c')
         {
            header = cc;
            cc = "";
            *token = '\0';
         }
         else if (token[1] == 'b')
         {

            print = KWFalse;
            *token = '\0';
         }
         else
            printmsg(0,"rmail: Invalid flag \"%s\" ignored!", token);

      } /* if (token == '-') */
      else if (print)
      {
         if (strpbrk(token,"!@") == nil(char))
         {
            if (bflag[F_BANG])
               sprintf(buf, "%s!%s", E_nodename, token);
            else
               sprintf(buf, "%s@%s", token, E_fdomain);
            token = buf;
         }

         PutHead(header, token, imf, KWFalse);
         header = "";               /* Continue same field by default  */

      }

   } /* while((count-- > 0) && print) */

/*--------------------------------------------------------------------*/
/*                     Handle the subject, if any                     */
/*--------------------------------------------------------------------*/

   if (subject != NULL)
      PutHead("Subject:", subject, imf, KWFalse);

   PutHead(NULL, "", imf, KWFalse); /* Terminate the header line       */
   imputc('\n',imf);                /* Terminate the header            */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* DaemonMail */

/*--------------------------------------------------------------------*/
/*    P u t H e a d                                                   */
/*                                                                    */
/*    Write one line of an RFC-822 header                             */
/*--------------------------------------------------------------------*/

 static void PutHead(const char *label,
                      const char *operand,
                      IMFILE *imf,
                      const KWBoolean resent)
 {
   static KWBoolean terminate = KWTrue;

   if (label == NULL)               /* Terminate call?                 */
   {                                /* Yes --> Reset Flag and return   */
      if (! terminate)
      {
         imputc('\n', imf);         /* Terminate the current line      */
         terminate = KWTrue;
      }

      return;

   } /* if */

   if (strlen(label))               /* First line of a header?         */
   {

      if (!terminate)               /* Terminate previous line?        */
         imputc('\n', imf);

      if (resent)
         imprintf(imf, "Resent-%s %s", label, operand);
      else
         imprintf(imf, "%-10s %s", label, operand);

      terminate = KWFalse;          /* Flag that we did not end file   */

   } /* if */
   else                             /* Continuing line                 */
      imprintf(imf, ",\n%-10s %s", label, operand);

 } /* PutHead */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report how the program works                                    */
/*--------------------------------------------------------------------*/

 static void usage(void)
 {

   static char syntax[] =
      "Usage:\tRMAIL\t-t [-x debug] [-g GRADE] [-f | -F file]\n"
      "\t\t-w [-x debug] [-g GRADE] [-f | -F file] [-s subject] "
            "addr1 [-c] addr2  [-b] addr3 ...\n"
      "\t\t[-x debug] [-g GRADE | -q] [-f | -F file] addr1 addr2 addr3 ...\n";

   puts(syntax);
   exit(99);

 } /* usage */
