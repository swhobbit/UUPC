/*--------------------------------------------------------------------*/
/*       r m a i l . c                                                */
/*                                                                    */
/*       Delivery agent for UUPC/extended                             */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: rmail.c 1.30 1994/10/23 23:29:44 ahd Exp $
 *
 *    $Log: rmail.c $
 * Revision 1.30  1994/10/23  23:29:44  ahd
 * Start logging BEFORE we process the options
 *
 * Revision 1.29  1994/08/07  21:28:54  ahd
 * Make To: line optional for processing such items as news
 * via mail.
 *
 * Revision 1.28  1994/03/15  03:02:26  ahd
 * Delete obsolete comments about format restrictions in RFC-822 mode
 *
 * Revision 1.27  1994/02/28  01:02:06  ahd
 * Don't close input file, redirect it to nul, in order to insure the
 * 0 file handle is not reused by other processing
 *
 * Revision 1.26  1994/02/25  03:17:43  ahd
 * Perform more precise check for headers when in Resent- mode of
 * Parse822
 *
 * Revision 1.25  1994/02/23  04:20:27  ahd
 * Don't eat first line in the body of the message
 * Don't emit the Date: field in RFC-822 mode when one exists!
 *
 * Revision 1.24  1994/02/21  16:38:58  ahd
 * Add routine name to failed to parse address message
 *
 * Revision 1.23  1994/02/20  19:07:38  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.22  1994/01/01  19:13:14  ahd
 * Annual Copyright Update
 *
 * Revision 1.21  1993/12/23  03:16:03  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.20  1993/12/13  03:09:13  ahd
 * Print error before panic() when cannot open temp file
 *
 * Revision 1.19  1993/12/07  04:57:53  ahd
 * Make ParseFrom perform heroics to pick up from FromUser whenever
 * possible
 *
 * Revision 1.18  1993/11/13  17:43:26  ahd
 * Use additional sources for From information
 *
 * Revision 1.17  1993/11/06  17:54:55  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.16  1993/11/06  13:04:13  ahd
 * Add For to Received: lines ... but is it backwards?
 *
 * Revision 1.15  1993/10/28  00:18:10  ahd
 * Correct initialize of arpadate to after implied tzset()
 *
 * Revision 1.14  1993/10/12  01:30:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.13  1993/09/20  04:39:51  ahd
 * OS/2 2.x support
 *
 * Revision 1.12  1993/07/31  16:22:16  ahd
 * Changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.11  1993/07/24  03:40:55  ahd
 * Make usage() return code unique
 *
 * Revision 1.10  1993/06/13  14:06:00  ahd
 * Save invoked program name and use it for recursive calls
 *
 * Revision 1.9  1993/05/09  03:41:47  ahd
 * Don't use debuglevel -1, it suppresses important configuration errors
 *
 * Revision 1.8  1993/04/15  03:17:21  ahd
 * Correct conditions under which name in userp structure used
 *
 * Revision 1.7  1993/04/13  02:26:30  ahd
 * Make return codes more unique
 *
 * Revision 1.6  1993/04/11  00:33:05  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.5  1992/12/05  23:38:43  ahd
 * Let logger close the log, not rmail
 *
 * Revision 1.4  1992/12/04  01:00:27  ahd
 * Add copyright messages
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

#ifdef _Windows
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#define  MOPLEN      10          /* Length of formatted header lines */
#define  UUCPFROM    "From "     /* Length of UUCP incoming mail     */

/*--------------------------------------------------------------------*/
/*                   Prototypes for internal files                    */
/*--------------------------------------------------------------------*/

static boolean CopyTemp( void );

static void ParseFrom( const char *forwho);

static char **Parse822( boolean *header,
                        size_t *count);

static void Terminate( const int rc);

 static void PutHead( const char *label,
                      const char *operand,
                      FILE *stream,
                      const boolean resent);

static boolean DaemonMail( const char *subject,
                           char **address,
                           int count );

 static void usage( void );

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

 currentfile();               /* Declare file name for checkref()    */
 char *tempname = NULL;       /* Pointer to temporary input file     */
 char *namein = CONSOLE;
 FILE *datain = NULL;         /* Handle for reading input mail       */
 FILE *dataout = NULL;        /* Handle for the output of mail       */
 char fromUser[MAXADDR] = ""; /* User id of originator               */
 char fromNode[MAXADDR] = ""; /* Node id of originator               */
 char *myProgramName = NULL;  /* Name for recursive invocation       */
 char grade  = 'C';           /* Grade for mail sent                 */

 static char received[] = "Received:";
 static char receivedlen = sizeof( received) - 1;

/*--------------------------------------------------------------------*/
/*                            main program                            */
/*--------------------------------------------------------------------*/

void main(int argc, char **argv)
{
   boolean ReadHeader = FALSE;   /* TRUE = Parse RFC-822 headers      */

   int  option;                  /* For parsing option list           */
   int  tempHandle;              /* For redirecting stdin             */
   char **address;               /* Pointer to list of target
                                    addresses                         */
   char *token;
   size_t addressees;            /* Number of targets in address      */
   size_t count;                 /* Loop variable for delivery        */
   size_t delivered = 0;         /* Count of successful deliveries    */
   int user_debug  = -1;
   boolean header = TRUE;        /* Assume terminated header         */
   boolean DeleteInput = FALSE;

   boolean daemon = FALSE;

   char *subject = NULL;
   char *logname = NULL;
   myProgramName = newstr( argv[0] );   /* Copy before banner() mangles it  */

/*--------------------------------------------------------------------*/
/*    Make a copy of the Borland copyright for debugging purposes     */
/*--------------------------------------------------------------------*/

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   logfile = stderr;             /* Prevent redirection of error      */
                                 /* messages during configuration     */

   banner( argv );

   debuglevel =  0;

/*--------------------------------------------------------------------*/
/*       Load the UUPC/extended configuration file, and exit if       */
/*       any errors                                                   */
/*--------------------------------------------------------------------*/

   if (!configure(B_MTA))
      Terminate(3);

   datain = stdin;

/*--------------------------------------------------------------------*/
/*                       Begin logging messages                       */
/*--------------------------------------------------------------------*/

   openlog( logname );

/*--------------------------------------------------------------------*/
/*                      Parse our operand flags                       */
/*--------------------------------------------------------------------*/

   while ((option = getopt(argc, argv, "g:ws:tF:f:l:x:")) != EOF)
   {
      switch (option)
      {

      case 'F':
         DeleteInput = TRUE;
         /* Fall through to regular file name choice */

      case 'f':
         namein = optarg;
         datain = FOPEN(namein , "r",TEXT_MODE);
         break;

      case 'g':
         if ( isalnum(*optarg) && ( strlen( optarg) == 1 ))
            grade = *optarg;
         else {
            printmsg(0,"Invalid grade for mail: %s", optarg );
            usage();
         }
         break;

      case 'l':
         openlog( optarg );
         break;

      case 's':
         subject = optarg;
         daemon = TRUE;
         break;

      case 't':
         ReadHeader = TRUE;
         break;

      case 'w':
         daemon = TRUE;
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

    if( signal( SIGINT, ctrlchandler ) == SIG_ERR )
    {
        printmsg( 0, "Couldn't set SIGINT\n" );
        panic();
    }

   if ( debuglevel > 1 )
   {
      for ( count = 1; (int) count < argc; count ++)
         printmsg(4,"rmail argv[%d] = \"%s\"", count, argv[count] );
   } /* if ( debuglevel > 4 ) */

   if ((optind == argc) != ReadHeader)
   {
      printmsg(0,"Missing/extra parameter(s) at end.");
      usage();
   }

   checkname( E_nodename );  /* Force loading of the E_fdomain name */

#if defined(_Windows)
   atexit( CloseEasyWin );               /* Auto-close EasyWin on exit  */
#endif

   remoteMail = ! (ReadHeader || daemon);
                              /* If not reading headers, must be in
                                 normal rmail mode ...               */

/*--------------------------------------------------------------------*/
/*    If in local mode and the user doesn't want output, suppress     */
/*    routine delivery messages                                       */
/*--------------------------------------------------------------------*/

   if (( user_debug == -1 ) && (debuglevel == 0))
   {
      if (remoteMail)
         debuglevel = 1;
      else
         debuglevel = (int) bflag[F_VERBOSE];
   }

/*--------------------------------------------------------------------*/
/*               Verify we have input stream available                */
/*--------------------------------------------------------------------*/

   if (datain == NULL )
   {
      printerr(namein);
      Terminate(6);
   } /* if */

/*--------------------------------------------------------------------*/
/*                   Open up the output data stream                   */
/*--------------------------------------------------------------------*/

   tempname = mktempname( NULL , "tmp");

   dataout = FOPEN(tempname, "w",TEXT_MODE);

   if (dataout == NULL)
   {
      printerr( tempname );
      printmsg(0,"Cannot open temporary file \"%s\" for output",
            tempname);
      Terminate(5);
   } /* if */

/*--------------------------------------------------------------------*/
/*   If in local mail mode, make up a list of addresses to mail to    */
/*--------------------------------------------------------------------*/

   if ( daemon )
   {
      addressees = argc - optind;
      address = &argv[optind];
      DaemonMail( subject, address, addressees );
      header = FALSE;
   }
   else if (ReadHeader)
      address = Parse822( &header, &addressees );
   else {
      addressees = argc - optind;
      address = &argv[optind];
      ParseFrom( addressees > 1 ? "multiple addressees" : *address );
                                 /* Copy remote header instead       */
   } /* if */

   if ( !address || ! addressees )  /* Can we deliver mail?          */
   {
      printmsg(0, "No addressees to deliver to!");
      Terminate( 2 );            /* No --> Execute punt formation    */
   }

/*--------------------------------------------------------------------*/
/*       Copy the rest of the input file into our holding tank        */
/*--------------------------------------------------------------------*/

   header = CopyTemp( ) && header ;
   if (header)                   /* Was the header ever terminated?  */
   {
      printmsg(0,"rmail: Improper header, adding trailing newline");
      fputc('\n', dataout);      /* If not, it is now ...            */
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

   if ( stdin != datain )           /* Insure input is closed too    */
      fclose(datain);

   if (DeleteInput)                 /* Make room for data on disk    */
      remove(namein);

   if ((tempHandle = open(BIT_BUCKET, O_RDONLY | O_BINARY)) == -1)
   {
     printerr(BIT_BUCKET);          /* Aw, heck, bit bucket is full? */
     panic();                       /* If we can't open it, kick it  */
   }
   else if (dup2(tempHandle, 0))    /* Swap stdin to empty input     */
   {
       printerr( "dup2" );
       panic();
   }

   close(tempHandle);               /* Don't need original handle    */

/*--------------------------------------------------------------------*/
/*              Also close output file before we reread it            */
/*--------------------------------------------------------------------*/

   fclose(dataout);

/*--------------------------------------------------------------------*/
/*                    Perform delivery of the mail                    */
/*--------------------------------------------------------------------*/

   while ((token = strpbrk(tempname ,"/")) != NULL)
      *token = '\\';

   for ( count = 0; count < addressees; count++)
         if ( *address[count] == '-')
            delivered ++;     /* Ignore option flags on delivery     */
         else
            delivered += Deliver(tempname, address[count], TRUE);

/*--------------------------------------------------------------------*/
/*                       Terminate the program                        */
/*--------------------------------------------------------------------*/

   printmsg(8,"rmail: %d addressees, delivered to %d mailboxes",
            addressees, delivered);

   if ( delivered >= addressees )
      Terminate( 0 );         /* All mail delivered                  */
   else if ( delivered == 0 )
      Terminate( 2 );         /* No mail delivered                   */
   else
      Terminate (1 );         /* Some mail delivered                 */

} /* main */

/*--------------------------------------------------------------------*/
/*    T e r m i n a t e                                               */
/*                                                                    */
/*    Cleanup open files and return to operating system               */
/*--------------------------------------------------------------------*/

static void Terminate( const int rc)
{
   if (tempname != NULL)         /* Did temporary file get named?    */
   {
      if (datain != stdin)       /* Non-standard input?              */
        fclose(stdin);           /* Yes --> Close it                 */
      remove(tempname);          /* Purge temporary file, if exists  */
   } /* if */

   exit( rc );                   /* Return to operating systems      */
}  /* Terminate */

/*--------------------------------------------------------------------*/
/*    P a r s e F r o m                                               */
/*                                                                    */
/*    Read the from address of incoming data from UUCP                */
/*--------------------------------------------------------------------*/

static void ParseFrom( const char *forwho)
{
   static const char from[] = "From ";
   static const char remote[] = "remote from ";
   static const size_t remotelen = sizeof remote - 1;
   static const size_t fromlen = sizeof from - 1;

   char *token;
   char buf[BUFSIZ];
   boolean hit;

   uuser = "uucp";            /* Effective id is always our daemon   */
   *fromUser = '\0';          /* Initialize for later tests          */
   *fromNode = '\0';          /* Initialize for later tests          */

/*--------------------------------------------------------------------*/
/*           Use UUXQT Information for nodename, if available         */
/*--------------------------------------------------------------------*/

   token = getenv( UU_MACHINE );
   if ( token != NULL )
   {
      strncpy( fromNode, token , sizeof fromNode );
      fromNode[ sizeof fromNode - 1 ] = '\0';
   }

/*--------------------------------------------------------------------*/
/*            Now look at the UUCP From line, if it exists            */
/*--------------------------------------------------------------------*/

   if (fgets(buf, BUFSIZ , datain) == NULL )
   {
      printmsg(0,"ParseFrom: Input file is empty!");
      panic();
   }

   hit = equaln(buf, from, fromlen );  /* true = UUCP From line   */

/*--------------------------------------------------------------------*/
/*       It's a From line.  Grab the initial address and then look    */
/*       for the final node at the end.                               */
/*--------------------------------------------------------------------*/

   if (hit)
   {
      int nodelen = strlen( fromNode ) + 1; /* Plus ! */
      char *s;
      token = strtok( &buf[ fromlen ], " ");
                                       /* Get second token on line
                                          with user's address        */
      s = strtok( NULL, "\n");         /* Get first non-blank of third
                                          token on the line          */

      if (strlen( token ) + nodelen >= MAXADDR) /* overlength addr?  */
      {                                /* Reduce address to length
                                          that we can handle         */
         char *next;
         token = strtok( token, "!" );

         while ((next = strtok( NULL , "!")) != NULL )
         {
            token = next;
            if (strlen( next ) + nodelen < MAXADDR)
               break;
         } /* while */
      } /* if */

      strncpy( fromUser, token , sizeof fromUser );
      fromUser[ sizeof fromUser - nodelen ] = '\0';

/*--------------------------------------------------------------------*/
/*       Make more heroic efforts to determine the fromNode.  This    */
/*       is done so programs like v-mail server which don't put on    */
/*       a remote on the From line -or- set any enironment variables  */
/*       still get valid information generated by deliver()           */
/*--------------------------------------------------------------------*/

      if (*fromNode == '\0')
      {
         s = strstr( s, remote );

         if (s != NULL )         /* Do we have a remote on line?     */
         {                       /* Yes --> Use as fromNode          */
            strncpy( fromNode, s + remotelen,  sizeof fromNode );

            fromNode[ sizeof fromNode -1 ] = '\0';
                                 /* Insure string is terminated      */
         }
         else {                  /* No --> look at fromUuser string  */
            token = strtok( fromUser, "!");  /* Find end of the node */

            if ( token != NULL )
            {
               strcpy( fromNode, token ); /* Take first part as node */
               token = strtok( NULL, ""); /* Get next part of user   */
               memmove( fromUser, token, strlen( token ) + 1 );
                                          /* Move to front of buffer */
            } /* if ( token != NULL ) */

         } /* else */

      } /* if (*fromNode == '\0') */

   } /* if */

/*--------------------------------------------------------------------*/
/*             Generate required "Received" header lines              */
/*--------------------------------------------------------------------*/

   fprintf(dataout,"%-10s from %s by %s (%s %s) with UUCP\n%-10s for %s; %s\n",
            "Received:", fromNode, E_domain, compilep, compilev,
            " ", forwho, arpadate());

/*--------------------------------------------------------------------*/
/*       If what we read wasn't a From line, write it into the new    */
/*       file after the generated Received: line                      */
/*--------------------------------------------------------------------*/

   if (!hit)
   {
      fputs(buf, dataout);

      if (ferror(dataout))
      {
         printerr(tempname);
         Terminate(6);
      } /* if */

   } /* if */

/*--------------------------------------------------------------------*/
/*              Determine the requestor user id and node              */
/*--------------------------------------------------------------------*/

   token = getenv( UU_USER ); /* Get exactly what remote told us     */

   if ( token != NULL )
   {                     /* Use exactly what remote told us     */
      ruser = strtok( token , WHITESPACE );
      if (( ruser != NULL ) && (fromUser == '\0'))
         strcpy(fromUser, ruser);

      rnode = strtok( NULL  , WHITESPACE );

   } /* else */

   if ((rnode == NULL) || (strchr(rnode,'.') == NULL ))
                              /* Did it tell us the domain?          */
   {                          /* No --> Use from information         */
      char node[MAXADDR];
      char user[MAXADDR];

      if (( fromNode != '\0' ) && (fromUser != '\0' ))
      {
         sprintf(buf ,"%s!%s", fromNode, fromUser);
         user_at_node(buf , buf, node, user);
         ruser = newstr( user );
         rnode = newstr( node );
      } /* if */
   }

   if ( fromNode == NULL )

/*--------------------------------------------------------------------*/
/*                    Provide defaults if no input                    */
/*--------------------------------------------------------------------*/

   if ( *fromNode == '\0' )
      strcpy(fromNode, rnode == NULL ? "somewhere" : rnode );

   if ( *fromUser == '\0' )
      strcpy(fromUser, "unknown");

   if ( rnode == NULL )
      rnode = fromNode;

   if ( ruser == NULL )
      ruser = fromUser;

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

static char **Parse822( boolean *header,
                        size_t *count)
{

/*--------------------------------------------------------------------*/
/*  Define the headers we will be examining and variables for their   */
/*                              lengths                               */
/*--------------------------------------------------------------------*/

   static const char resent[] = "Resent-";
   static const size_t resentLen =  sizeof resent - 1;

   size_t offset = 0;         /* Subscript for examining headers,
                                 which allows us to ignore Resent-   */

   size_t allocated = 5;      /* Reasonable first size for address   */
                              /* Note: MUST BE AT LEAST 2 because we
                                       add 50% below!                */

   char **addrlist = calloc( sizeof *addrlist , allocated);
   char buf[BUFSIZ];          /* Input buffer for reading header     */
   char outputBuffer[BUFSIZ+MAXADDR]; /* Output buffer for addresses */
   char address[MAXADDR];     /* Buffer for parsed address           */
   char path[MAXADDR];
   int senderID = -1, dateID = -1, fromID = -1;
   struct HostTable *hostp;

   typedef struct _HEADERS
   {
      const char *text;
      char  *address;
      const boolean blind;
      const boolean required;
      const boolean output;
      boolean found;
   } HEADERS;

   boolean blind = FALSE;
   boolean output = FALSE;

   char sender[MAXADDR];
   char from[MAXADDR];

   static HEADERS headerTable[] =
   {
      { "From:",   NULL,  FALSE, TRUE,  FALSE, FALSE },
      { "Sender:", NULL,  FALSE, FALSE, FALSE, FALSE },
      { "To:",     NULL,  FALSE, FALSE, TRUE,  FALSE },
      { "Cc:",     NULL,  FALSE, FALSE, TRUE,  FALSE },
      { "Bcc:",    NULL,  TRUE,  FALSE, TRUE,  FALSE },
      { "Date:",   NULL,  FALSE, FALSE, FALSE, FALSE },
      { NULL }
   };

   size_t subscript;

/*--------------------------------------------------------------------*/
/*               Determine selected subscript information             */
/*--------------------------------------------------------------------*/

   for ( subscript = 0;
         headerTable[subscript].text != NULL;
         subscript++ )
   {
      if ( equal( headerTable[subscript].text, "Date:" ))
         dateID = subscript;
      else if ( equal( headerTable[subscript].text, "From:" ))
         fromID = subscript;
      else if ( equal( headerTable[subscript].text, "Sender:" ))
         senderID = subscript;

      headerTable[subscript].found = FALSE;

   } /* for */

/*--------------------------------------------------------------------*/
/*                   Initialized string information                   */
/*--------------------------------------------------------------------*/

   if (( dateID < 0 ) || (fromID < 0) || (senderID < 0))
      panic();

   headerTable[fromID].address   = from;
   headerTable[senderID].address = sender;

   *outputBuffer = '\0';            /* Flag no addresses to send yet */

/*--------------------------------------------------------------------*/
/*                          Begin processing                          */
/*--------------------------------------------------------------------*/

   *count = 0;                /* No addresses discovered yet         */
   checkref(addrlist);        /* Verify we had room for the list     */

   fprintf(dataout,"%-10s by %s (%s %s);\n%-10s %s\n",
              "Received:",E_domain,compilep, compilev,
              " ", arpadate() );

/*--------------------------------------------------------------------*/
/*                       Generate a message-id                        */
/*--------------------------------------------------------------------*/

   sprintf(buf, "<%lx.%s@%s>", time( NULL ) , E_nodename, E_domain);
   PutHead("Message-ID:", buf, dataout , offset != 0 );
   PutHead(NULL, NULL, dataout , FALSE ); /* Terminate header        */

/*--------------------------------------------------------------------*/
/*                        Find the From: line                         */
/*--------------------------------------------------------------------*/

   while( *header && (fgets( buf, BUFSIZ, datain) != NULL) )
   {
      char *startAddress = buf;

      if ( *buf == '\n')         /* end of the header?               */
      {
         output = *header = FALSE;  /* Yes --> reset all our flags   */
         blind = TRUE;           /* We'll print terminator later     */
      }

/*--------------------------------------------------------------------*/
/*              Set flags whenever we find a new header               */
/*--------------------------------------------------------------------*/

      else if ( isgraph( *buf )) /* Start of a new header?           */
      {
         blind = output = FALSE; /* Reset processing flags for this
                                    header                           */

         if ( equalni( buf, resent, resentLen ))   /* Msg a resend?  */
            offset = resentLen;  /* Yes --> Only use Resent- hdrs    */

/*--------------------------------------------------------------------*/
/*         Loop to find the header in our table, if possible          */
/*--------------------------------------------------------------------*/

         for ( subscript = 0;
               headerTable[subscript].text != NULL;
               subscript++ )
         {
            size_t headerLen = strlen( headerTable[subscript].text );

            if (( ! offset || equalni(buf, resent, offset )) &&
                  equalni( buf + offset,
                          headerTable[subscript].text,
                          headerLen ))
            {
               if ( headerTable[subscript].found ) /* Been here before? */
               {
                  printmsg(0,"Parse822: Error: Duplicate header: %s",
                              buf );
                  return NULL;
               }

               headerTable[subscript].found = TRUE;
               blind  = headerTable[subscript].blind;
               output = headerTable[subscript].output;

               startAddress = buf + offset + headerLen;

               if ( isgraph( *startAddress ))   /* Non-blank?     */
               {                       /* I'm SO confused         */
                  printmsg(0,"Parse822: Invalid header, cannot continue: %s",
                              buf );
                  return NULL;
               }

/*--------------------------------------------------------------------*/
/*                 Save address if table requested it                 */
/*--------------------------------------------------------------------*/

               if ( headerTable[subscript].address != NULL )
               {
                  ExtractAddress( headerTable[subscript].address,
                                  startAddress,
                                  FALSE );
               }

/*--------------------------------------------------------------------*/
/*            Insert separator between addresses if needed            */
/*--------------------------------------------------------------------*/

               if ( *outputBuffer && output )
                  strcat( outputBuffer, "," );   /* Sep addresses  */

               break;               /* drop out of the for ( ;; ) */

            } /* if equalni() */

         } /* for */

      } /* else if */

/*--------------------------------------------------------------------*/
/*              Write the line out unless a blind header              */
/*--------------------------------------------------------------------*/

      if ( ! blind )
         fputs(buf, dataout );

/*--------------------------------------------------------------------*/
/*                       Save output addresses                        */
/*--------------------------------------------------------------------*/

      if ( output )
      {
         while( *startAddress && !isgraph( *startAddress ))
            startAddress++;         /* Step past while space in addr */

         printmsg(10,"Adding address [%s] to [%s]",
               startAddress,
               outputBuffer );
         strcat( outputBuffer, startAddress );
      }

/*--------------------------------------------------------------------*/
/*       Spit out any addresses if we may overflow the buffer or      */
/*       we're done collecting them.                                  */
/*--------------------------------------------------------------------*/

      while( *outputBuffer &&
             (! output || (strlen(outputBuffer) > MAXADDR )))
      {
         char *next = ExtractAddress( address, outputBuffer, FALSE );
                                 /* Get address to add to list    */

         if (allocated == (*count+1))  /* Do we have room for addr?  */
         {
            allocated += allocated / 2;   /* Choose larger array     */
            addrlist = realloc( addrlist ,
                                allocated * sizeof( *addrlist ));
            checkref(addrlist);  /* Verify the allocation worked     */
         } /* if */

         if (!strlen(address))
         {
            printmsg(0,"Parse822: Could not locate expected address in header");
            return NULL;
         } /* if */
         else {
            addrlist[*count] = newstr( address );
                                 /* Save permanent copy of address      */
            printmsg(4,"address[%d]= \"%s\"",*count, address);
            *count += 1;         /* Flag we got the address             */
         } /* else */

         if ( next )
            memmove( outputBuffer, next, strlen(next) + 1);
                                 /* Shift buffer up to recover space
                                    used by now parsed address          */
         else
            *outputBuffer = '\0'; /* End of addresses                   */

      } /* while */

   } /* while( (fgets( buf, BUFSIZ, datain) != NULL) && *header ) */

/*--------------------------------------------------------------------*/
/*               Now validate the information we received             */
/*--------------------------------------------------------------------*/

   if ( *header )             /* Did we receive a proper header?     */
   {                          /* No --> Very bad news                */
      printmsg(0,"Parse822: Premature end of input file, message rejected");
      return NULL;
   }

   for ( subscript = 0;
         headerTable[subscript].text != NULL;
         subscript++ )
   {
      if ( headerTable[subscript].required &&
           ! headerTable[subscript].found )
      {
         printmsg(0, "Parse822: Missing header \"%s\", cannot continue.",
                      headerTable[subscript].text );
         return NULL;
      }

   } /* for (;;) */

/*--------------------------------------------------------------------*/
/*                Fill in the sender field, if needed                 */
/*--------------------------------------------------------------------*/

   user_at_node( headerTable[senderID].found ? sender : from,
                 path,
                 fromNode,
                 fromUser);      /* Separate portions of the address */

/*--------------------------------------------------------------------*/
/*               Generate a Sender: line if we need it                */
/*--------------------------------------------------------------------*/

   if (equal(fromNode,HostAlias(E_fdomain))) /* Same as hidden site? */
      strcpy(fromNode, E_nodename);/* Yes --> Declare as local system */

   hostp = checkname( fromNode );   /* Look up real system name      */

   if (!equal(fromUser,E_mailbox) ||
       (hostp == BADHOST) || (hostp->status.hstatus != localhost))
   {
      sprintf(buf, "%s <%s@%s>", E_name, E_mailbox, E_fdomain );
      PutHead("Sender:", buf, dataout , offset != 0 );
   } /* if */

/*--------------------------------------------------------------------*/
/*                     Insert a date field if needed                  */
/*--------------------------------------------------------------------*/

   if ( ! headerTable[dateID].found )
      PutHead("Date:", arpadate() , dataout , offset != 0 );

/*--------------------------------------------------------------------*/
/*      Set UUCP requestor name while we've got the information       */
/*--------------------------------------------------------------------*/

   if ((hostp != BADHOST) && (hostp->status.hstatus == localhost))
      rnode = bflag[F_BANG] ? E_nodename : E_fdomain;
                              /* Use full domain address, if possible */
   else
      rnode = fromNode;

   uuser = ruser = fromUser;  /* User and requestor always the same
                                 for locally generated mail          */

/*--------------------------------------------------------------------*/
/*                        Terminate the header                        */
/*--------------------------------------------------------------------*/

   PutHead(NULL, NULL, dataout , FALSE ); /* End the headers         */
   fputc('\n', dataout );

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

static boolean CopyTemp( void )
{
   boolean header = TRUE;
   char buf[BUFSIZ];
   boolean newline = TRUE;

   while (fgets(buf, BUFSIZ, datain) != NULL)
   {
      if (header)
      {
         if (*buf == '\n')
            header = FALSE;
         else if (equalni(received, buf, receivedlen))
            hops++;
      }

      newline = buf[ strlen( buf ) - 1 ] == '\n';

      if (fputs(buf, dataout) == EOF)  /* I/O error?                 */
      {
         printerr(tempname);
         printmsg(0,"I/O error on \"%s\"", tempname);
         fclose(dataout);
         return FALSE;
      } /* if */
   } /* while */

   if (ferror(datain))        /* Clean end of file on input?         */
   {
      printerr(namein);
      Terminate(7);
   }

   if ( !newline )            /* Is the file terminated properly?    */
   {
      printmsg(0, "rmail: Improperly formed message, adding final newline!");
      fputc( '\n', dataout );
   }

   return header;
}  /* CopyTemp */

/*--------------------------------------------------------------------*/
/*    D a e m o n M a i l                                             */
/*                                                                    */
/*    Send text in a mailbag file to address(es) specified by address */
/*--------------------------------------------------------------------*/

static boolean DaemonMail( const char *subject,
                          char **address,
                          int count )
{
   char buf[BUFSIZ];
   char *username;
   char *token;
   char *moi = NULL;
   struct UserTable *userp;
   char *header = "To:";
   char *cc     = "Cc:";
   boolean print = TRUE;

/*--------------------------------------------------------------------*/
/*                         Validate the input                         */
/*--------------------------------------------------------------------*/

   if ( count == 0 )
   {
      printmsg(0,"rmail: No addresseses to deliver to!");
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                       Determine our user id                        */
/*--------------------------------------------------------------------*/

   username = getenv( LOGNAME );
   if ( username == NULL )
      username = E_mailbox;

/*--------------------------------------------------------------------*/
/*              Get the name of the user, or make one up              */
/*--------------------------------------------------------------------*/

   userp = checkuser(username);  /* Locate user id in host table     */

   if ( (userp != BADUSER) &&
        (userp->realname != NULL) &&
         !equal(userp->realname, EMPTY_GCOS ))
      moi = userp->realname;
   else if ( equali(username, E_postmaster) ||
             equali(username, POSTMASTER))
      moi = "Postmaster";
   else if ( equali( username, "uucp" ))
      moi = "Unix to Unix Copy";
   else
      moi = username;         /* Dummy to ease formatting From: line  */

/*--------------------------------------------------------------------*/
/*    Add the boilerplate the front:                                  */
/*                                                                    */
/*       Date, From, Organization, and Reply-To                       */
/*--------------------------------------------------------------------*/

   fprintf(dataout,"%-10s by %s (%s %s)\n%-10s for %s; %s\n",
              "Received:",
              E_domain,
              compilep,
              compilev,
              " ",
              count > 1 ? "multiple addressees" : *address,
              arpadate() );

/*--------------------------------------------------------------------*/
/*                       Generate a message-id                        */
/*--------------------------------------------------------------------*/

   sprintf(buf, "<%lx.%s@%s>", time( NULL ) , E_nodename, E_domain);
   PutHead("Message-ID:", buf, dataout , FALSE );
   PutHead(NULL, NULL, dataout , FALSE );

   PutHead("Date:", arpadate() , dataout, FALSE);

   if (bflag[F_BANG])
      sprintf(buf, "(%s) %s!%s", moi, E_nodename, username );
   else {
      sprintf(buf, "\"%s\" <%s@%s>", moi, username , E_fdomain );
   }

   PutHead("From:", buf, dataout, FALSE );

   if (E_organization != NULL )
      PutHead("Organization:", E_organization, dataout, FALSE);

/*--------------------------------------------------------------------*/
/*                      Write the address out                         */
/*--------------------------------------------------------------------*/

   while( (count-- > 0) && print )
   {
      token = *address++;
      if ( *token == '-')  /* Option flag?                        */
      {
         if (token[1] == 'c')
         {
            header = cc;
            cc = "";
         }
         else if (token[1] == 'b')
            print = FALSE;
         else
            printmsg(0,"rmail: Invalid flag \"%s\" ignored!", token);
      } /* if ( token == '-') */
      else if ( print )
      {
         if (strpbrk(token,"!@") == nil(char))
         {
            if (bflag[F_BANG])
               sprintf(buf, "%s!%s", E_nodename, token );
            else
               sprintf(buf, "%s@%s", token , E_fdomain );
            token = buf;
         }

         PutHead(header , token, dataout, FALSE);
         header = "";         /* Continue same field by default      */
      }
   } /* while( (count-- > 0) && print ) */

/*--------------------------------------------------------------------*/
/*                     Handle the subject, if any                     */
/*--------------------------------------------------------------------*/

   if (subject != NULL)
      PutHead("Subject:", subject, dataout, FALSE);

   PutHead(NULL, "", dataout, FALSE);  /* Terminate the header line   */
   fputc('\n',dataout );               /* Terminate the header        */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   uuser = ruser = strncpy(fromUser, username, sizeof fromUser);
                              /* Define user for UUCP From line      */
   fromUser[ sizeof fromUser - 1 ] = '\0';
   rnode = bflag[F_BANG] ? E_nodename : E_fdomain;
                              /* Use full domain address, if possible */

   strcpy(fromNode, E_nodename);/* Declare as local system           */
   return TRUE;

} /*DaemonMail*/

/*--------------------------------------------------------------------*/
/*    P u t H e a d                                                   */
/*                                                                    */
/*    Write one line of an RFC-822 header                             */
/*--------------------------------------------------------------------*/

 static void PutHead( const char *label,
                      const char *operand,
                      FILE *stream,
                      const boolean resent)
 {
   static boolean terminate = TRUE;

   if (label == NULL )        /* Terminate call?                     */
   {                          /* Yes --> Reset Flag and return       */
      if ( ! terminate )
      {
         fputc('\n', stream); /* Terminate the current line          */
         terminate = TRUE;
      }
      return;
   } /* if */

   if (strlen(label))         /* First line of a header?             */
   {
      if (!terminate)         /* Terminate previous line?            */
         fputc('\n', stream);

      if (resent)
         fprintf(stream,"Resent-%s %s",label, operand);
      else
         fprintf(stream,"%-10s %s",label, operand);
      terminate = FALSE;          /* Flag that we did not end file   */
   } /* if */
   else                       /* Continuing line                     */
      fprintf(stream,",\n%-10s %s",label, operand);
 } /* PutHead */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report how the program works                                    */
/*--------------------------------------------------------------------*/

 static void usage( void )
 {

   static char syntax[] =
      "Usage:\tRMAIL\t-t [-x debug] [-g GRADE] [-f | -F file]\n"
      "\t\t-w [-x debug] [-g GRADE] [-f | -F file] [-s subject] addr1 [-c] addr2  [-b] addr3 ...\n"
      "\t\t[-x debug] [-g GRADE] [-f | -F file] addr1 addr2 addr3 ...\n";

   puts( syntax );
   exit(99);
 }
