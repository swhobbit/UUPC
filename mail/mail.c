/*--------------------------------------------------------------------*/
/*       m a i l . c                                                  */
/*                                                                    */
/*       Mailer User-Agent (UA)                                       */
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
 *    $Id: mail.c 1.14 1993/10/31 21:32:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: mail.c $
 * Revision 1.14  1993/10/31  21:32:55  ahd
 * Don't print current header after headers command
 *
 * Revision 1.13  1993/10/31  19:04:03  ahd
 * Change "DOS" to "system" in short command help text
 *
 * Revision 1.12  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.11  1993/10/12  01:32:08  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.9  1993/10/09  20:16:12  rhg
 * ANSIfy the source
 *
 * Revision 1.8  1993/09/23  03:26:51  ahd
 * Use current version variables for Visual C++ under Windows NT
 *
 * Revision 1.7  1993/09/20  04:39:51  ahd
 * OS/2 2.x support
 *
 * Revision 1.6  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.5  1993/07/24  03:40:55  ahd
 * Change description of "-" command, previous command.
 *
 * version  1.0   Stuart Lynne
 * version 1.5 Samuel Lam <skl@van-bc.UUCP>  August/87
 *
 * version 1.6 Drew Derbyshire   May/89
 *             Support for single user aliases, -u option for reading
 *             alternate mailboxes, parsing addresses via external routine,
 *             parsing Resent- fields, suppressing Received: fields,
 *             automatic positioning to next message.                   ahd
 * 23 Sep 89   Version 1.07a
 *             Support lists in aliases                                 ahd
 *
 * 29 Sep 89   Version 1.07b
 *             Add prompting for subject in outgoing mail.              ahd
 * 01 Oct 89   Add additional function prototypes to catch bad calls    ahd
 * 02 Oct 89   Alter large strings/structures to use malloc()/free()    ahd
 * 12 Oct 89   Version 1.07d
 *             Correct free() of line in Send_Mail
 * 12 Dec 89   Version 1.07g
 *             Various spelling corrections
 * 18 Mar 90   Version 1.07i
 *             Add ~user support for save/write command
 *             Add ignore list for user
 *             Shorten lines printed by aborting from a print command   ahd
 * 30 Apr  90  Add autoedit support for sending mail                    ahd
 *  2 May  90  Add support for options= flags                           ahd
 *  3 May  90  Split selected subroutines into maillib.c                ahd
 *  4 May  90  Add 'save' option.                                       ahd
 *  8 May  90  Add 'pager' option                                       ahd
 * 10 May  90  Add 'purge' option                                       ahd
 * 13 May  90  Alter logging so that no numbers are printed on console  ahd
 * Additions for unofficial version 1.07k, Philip David Meese June 1990
 * 16 June 90
 *            -added mail command: Copy current (without delete)        pdm
 *            -altered calls to Collect_Mail to support mail subcmds    pdm
 *            -added handling of '+' to indicate "relative to home
 *                directory" for BSD like mail users.                   pdm
 * 12 Feb 91 rewrite parser a for more BSD like syntax
*/

 static const char rcsid[] =
      "$Id: mail.c 1.14 1993/10/31 21:32:55 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <dos.h>
#include <direct.h>

#ifdef _Windows
#include <windows.h>
#include <alloc.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "address.h"
#include "alias.h"                                            /* ahd */
#include "dater.h"
#include "expath.h"
#include "getopt.h"
#include "hlib.h"
#include "mail.h"
#include "mailblib.h"
#include "maillib.h"                                           /* ahd */
#include "mailsend.h"
#include "mlib.h"
#include "pushpop.h"
#include "stater.h"
#include "timestmp.h"
#include "arpadate.h"

#if defined(_Windows)
#include "winutil.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#ifdef _Windows
unsigned _stklen = 10 * 1024;
unsigned _heaplen = 30 * 1024;
#endif

currentfile();

static char *tmailbox;
static char mfilename[FILENAME_MAX];
int letternum = 0;

static boolean useto = FALSE;

FILE *fmailbox;

#define MAXLETTERS   100

static int maxletters = MAXLETTERS;

struct  ldesc *letters;

/*--------------------------------------------------------------------*/
/*                       Local procedure names                        */
/*--------------------------------------------------------------------*/

static void    Cleanup(void);

static void Interactive_Mail( const boolean PrintOnly,
                              const boolean postoffice );

static void    IncludeNew( const char *target, const char *user);

static void    PrintSubject(int msgnum, int letternum);

static void    UpdateMailbox(int letternum, boolean postoffice);

static int     CreateBox(FILE *rmailbox,
                         FILE *fmailbox,
                         const char *tmailbox);

static void usage( void );

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char *replytolist[] = { "Resent-Reply-To:",
                        "Resent-From:",
                        "Reply-To:",
                        "From:",
                         NULL };

static char *fromlist[] =    { "Resent-From:",
                               "From:",
                               NULL};

static char *tolist[] =    {   "Resent-To:",
                               "To:",
                               NULL};

static char *subjectlist[] = { "Resent-Subject:",
                               "Subject:",
                                NULL };

static char *datelist[]  =   { "Resent-Date:",
                               "Date:" ,
                               NULL} ;


/*--------------------------------------------------------------------*/
/*                  Information on existing mailbox                   */
/*--------------------------------------------------------------------*/

static long    mboxsize = 0;
static time_t  mboxage  = 0;

/*--------------------------------------------------------------------*/
/*                       Command parsing table                        */
/*--------------------------------------------------------------------*/

#define NUMERIC_CMD "9999"
#define EMPTY_CMD   ""

static struct CommandTable {
   char *sym;
   ACTION verb;
   unsigned int bits;
   char *help;
} table[] = {
 { EMPTY_CMD,     M_EMPTY,    NODISPLAY | NO_OPERANDS | AUTOPRINT ,
         NULL},
 { "!",           M_SYSTEM,   STRING_OP,
         "Execute system command"},
 { "+",           M_DOWN,     KEWSHORT_OP | AUTOPRINT,
         "Alias for next"},
 { "-",           M_UP,       KEWSHORT_OP | AUTOPRINT,
         "Alias for previous"},
 { "?",           M_FASTHELP, NO_OPERANDS,
         "Print this help"},
 { "alias",       M_ALIAS,    TOKEN_OP,
         "Print user alias"},
 { "copy",        M_COPY,     LETTER_OP | FILE_OP ,
         "Copy item to file"},
 { "delete",      M_DELETE,   LETTER_OP | POSITION | AUTOPRINT ,
         "Delete mail item"},
 { "debug",       M_DEBUG,    KEWSHORT_OP,
         "Enable debug output"},
 { "dquit",       M_DELETEQ,  LETTER_OP ,
         "Delete then quit"},
 { "exit",        M_EXIT,     NO_OPERANDS,
         "Exit without updating mailbox"},
 { "forward",     M_FORWARD,  LETTER_OP | USER_OP,
         "Resend item to others"},
 { "go",          M_GOTO,     LETTER_OP | AUTOPRINT ,
         "Go to item"},
 { "Headers",     M_HEADERS,  LETTER_OP | POSITION | NOAUTOHEADER,
         "Print specified item summary"},
 { "headers",     M_HEADERS,  NO_OPERANDS | NOAUTOHEADER,
         "Print all item summaries"},
 { "help",        M_HELP,     NO_OPERANDS,
         "Print long help text"},
 { "mail",        M_MAIL,     USER_OP,
         "Compose and send mail"},
 { "next",        M_DOWN,     KEWSHORT_OP | AUTOPRINT ,
         "Move to next item"},
  {"print",       M_EXTPRINT, LETTER_OP | POSITION ,
         "Print item (condensed)"},
  {"Print",       M_INTPRINT, LETTER_OP | POSITION ,
         "Print item (condensed)"},
  {"previous",    M_UP,       KEWSHORT_OP | AUTOPRINT ,
         "Move to previous item"},
  {"quit",        M_QUIT,     NO_OPERANDS,
         "Update mailbox, exit"},
  {"reply",       M_REPLY,    LETTER_OP | POSITION ,
         "Reply to sender of item"},
  {"save",        M_SAVE,     LETTER_OP | FILE_OP | POSITION | AUTOPRINT ,
         "Copy item, delete"},
  {"set",         M_SET,      STRING_OP,
         "Print/set boolean options"},
  {"status",     M_STATUS,  NO_OPERANDS,
         "Report version/status info"},
  {"type",        M_EXTTYPE,  LETTER_OP | POSITION,
         "Print item with all headers"},
  {"Type",        M_INTTYPE,  LETTER_OP | POSITION,
         "Print item with all headers"},
  {"undelete",    M_UNDELETE, LETTER_OP | POSITION | AUTOPRINT ,
         "Rescue item after save/delete"},
  {"write",       M_WRITE,    LETTER_OP | FILE_OP | POSITION | AUTOPRINT ,
         "Copy item w/o header, delete"},
  {"xit",         M_EXIT,     NO_OPERANDS,
         "alias for exit"},
  { NUMERIC_CMD,   M_GOTO,     NODISPLAY | KEWSHORT_OP | AUTOPRINT ,
         NULL} ,
  { NULL,          M_INVALID,  NODISPLAY | STRING_OP,
         NULL }
         } ;

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

void main(int argc, char **argv)
{

   boolean PrintOnly = FALSE;
   boolean postoffice = TRUE;
   boolean readmail   = FALSE;
   boolean sendmail   = FALSE;
   int option;
   char    *subject = NULL;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   banner( argv );

   if (!configure( B_MUA ))
      exit(1);    /* system configuration failed */

   if (!InitRouter())
      exit(1);    /* system configuration failed */

   tmailbox = mktempname(NULL, "TMP");
   PushDir(".");

/*--------------------------------------------------------------------*/
/*                       get mailbox file name                        */
/*--------------------------------------------------------------------*/

   strcpy( mfilename, E_mailbox );
   if ( strchr( mfilename ,'.' ) == NULL )
      mfilename[8] = '\0';       /* Prevent OS/2 filename length
                                    overrun                          */

   if ( bflag[ F_MULTITASK ] )
   {
      if (expand_path( mfilename, E_homedir, E_homedir, E_mailext ) == NULL )
         panic();
   }
   else
      mkmailbox(mfilename, E_mailbox );

/*--------------------------------------------------------------------*/
/*                          parse arguments                           */
/*--------------------------------------------------------------------*/

   while ((option = getopt(argc, argv, "f:ps:tu:x:")) != EOF)
   {
      char oname[FILENAME_MAX];

      switch (option)
      {
      case 'f':
         readmail = TRUE;
         strcpy( mfilename, optarg );
         if (expand_path( mfilename, NULL, E_homedir, E_mailext ) == NULL )
            usage();

/*--------------------------------------------------------------------*/
/*    This next one is a little tricky ...  If we log outgoing        */
/*    mail, we copy the name of the outgoing mail file into a         */
/*    temporary buffer, and expand the name of the file to include    */
/*    the path name.  If this name is the same as the current         */
/*    file, flip-flip the useto flag which says use the To:           */
/*    related fields when scanning headers, not the From:  related    */
/*    fields.                                                         */
/*--------------------------------------------------------------------*/

         if (( E_filesent != NULL ) &&
             (expand_path( strcpy( oname, E_filesent) ,
                          E_homedir, E_homedir , E_mailext ) != NULL ) &&
             equali( oname , mfilename ))
                           /* Our outgoing filename?              */
            useto = ! useto;  /* Yes --> Automatically switch     */
         postoffice = FALSE;
         break;

      case 'p':
         readmail = TRUE;
         PrintOnly = TRUE;
         break;

      case 'u':                  /* Read alternate mailbox?       */
         readmail = TRUE;
         mkmailbox(mfilename, optarg);
         postoffice = FALSE;
         break;

      case 'x':
         debuglevel = atoi(optarg);
         break;

      case 's':
         sendmail = TRUE;
         subject = optarg;
         break;

      case 't':
         readmail = TRUE;
         useto = ! useto;
         break;

      case '?':
         usage();

      } /* switch */

   } /* while */

/*--------------------------------------------------------------------*/
/*                        Check for conflicts                         */
/*--------------------------------------------------------------------*/

   sendmail |= (optind != argc);

   if ( sendmail && readmail )
   {
      puts("Conflicting options specified");
      usage();
   }

   if ((optind == argc) && sendmail)
   {
      puts("Missing addresses for sending mail");
      usage();
   }

/*--------------------------------------------------------------------*/
/*        We have the options, now decide how to process them         */
/*--------------------------------------------------------------------*/

   if (sendmail)
   {
      argc -= optind;

      if ( subject != NULL )
      {

         argv    = &argv[optind-2];
         argv[0] = "-s";
         argv[1] = subject;

         Collect_Mail(stdin, argc+2 , argv , -1, FALSE);
      } /* if ( subject != NULL ) */
      else {
         Collect_Mail(stdin, argc, &argv[optind], -1, FALSE);

#ifdef _Windows
         atexit ( CloseEasyWin );
#endif
      }

   } /* if (sendmail) */
   else  {
      if ( postoffice && bflag[ F_MULTITASK ] )
         IncludeNew( mfilename, E_mailbox);
      Interactive_Mail( PrintOnly , postoffice );
   }

   Cleanup();
   PopDir();
   exit(0);

} /*main*/

/*--------------------------------------------------------------------*/
/*    C l e a n u p                                                   */
/*                                                                    */
/*    Remove temporary files when exiting                             */
/*--------------------------------------------------------------------*/

void Cleanup()
{

   printmsg(2,"Deleting temporary mailbox %s", tmailbox);

   if ( fmailbox != NULL )
   {
      fclose(fmailbox);
      fmailbox = NULL;
   }

   unlink(tmailbox);

} /*Cleanup*/


/*--------------------------------------------------------------------*/
/*    I n t e r a c t i v e _ M a i l                                 */
/*                                                                    */
/*    main procedure for reading mail                                 */
/*--------------------------------------------------------------------*/

static void Interactive_Mail( const boolean PrintOnly,
                              const boolean postoffice )
{
   char resp[LSIZE];
   int current = 0;                                               /* ahd  */
   boolean done      = FALSE;                                     /* ahd  */
   boolean modified;
   FILE *rmailbox;

/*--------------------------------------------------------------------*/
/*               Open real and temporary mailbox files                */
/*--------------------------------------------------------------------*/

   if ((rmailbox = FOPEN(mfilename, "r",TEXT_MODE)) == nil(FILE)) {
      printf("No mail in %s\n", mfilename);
      return;
   }

   mboxage = stater( mfilename, &mboxsize );
                              /* Remember mailbox information        */

   if ((fmailbox = FOPEN(tmailbox, "w", BINARY_MODE)) == nil(FILE)) {
      printerr(tmailbox);
      return;
   }

   letters = calloc(maxletters,sizeof(letters[0]));
   checkref(letters);

/*--------------------------------------------------------------------*/
/*                 Copy real mailbox to temporary one                 */
/*--------------------------------------------------------------------*/

   setvbuf(rmailbox, NULL, _IOFBF, 8192);
   setvbuf(fmailbox, NULL, _IOFBF, 8192);

   letternum = CreateBox(rmailbox, fmailbox, tmailbox);

   fclose(rmailbox);
   fclose(fmailbox);

   rmailbox = fmailbox = NULL;

   if (letternum < 1)            /* Did we find any mail in the box? */
   {                             /* No --> Return to caller          */
      if (letternum == 0)
         printf("No mail in %s\n", mfilename);
      return;
   }

/*--------------------------------------------------------------------*/
/*        Shrink mailbox status array to what we actually need        */
/*--------------------------------------------------------------------*/

   letters = realloc( letters, (letternum + 1) *  sizeof(letters[0]));
   checkref(letters);

   fmailbox = FOPEN(tmailbox, "r", BINARY_MODE);

   if (fmailbox == NULL)
   {
      printerr(tmailbox);
      panic();
   } /* if */
   setvbuf(fmailbox, NULL, _IOFBF, 8192);

   modified = postoffice && (!PrintOnly);

   if (PrintOnly) {
      int j = 0;
      while (j < letternum)
      {
         Pager(j, TRUE, noreceived, !j );
         j++ ;
      }
      return;
   }

   PrintSubject(-1,letternum); /* print all subjects */

/*--------------------------------------------------------------------*/
/*               Determine first letter in to prompt at               */
/*--------------------------------------------------------------------*/

      if (letternum == 0)
         current = -1;
      else
         current = 0;

/*--------------------------------------------------------------------*/
/*            Begin main command loop for reading the mail            */
/*--------------------------------------------------------------------*/

   if (!bflag[F_EXPERT])
      printf("Enter \"?\" for short help or \"help\" for long help.\n");

#ifdef _Windows
   atexit ( CloseEasyWin );
#endif

   while( ! done )
   {
      char *command, *operand;
      int integer;
      boolean first_pass = TRUE;
      int previous = current;
      struct CommandTable *cmd_ptr = table;
      boolean success = TRUE;
      boolean crlf    = FALSE;      /* crlf after delete command?    */

      printf("%d%s",current + 1,
               (letters[current].status == M_DELETED) ? "*" : " ");
      if (!Console_fgets(resp, LSIZE, "? ")) /* End of file?         */
      {
         done = TRUE;
         continue;            /* Yes --> Exit loop                   */
      }
      PageReset();

/*--------------------------------------------------------------------*/
/*                     Locate command to execute                      */
/*--------------------------------------------------------------------*/

      integer = strlen( resp );
      if (integer && ( resp[ integer - 1 ] == '\n'))
         resp[ integer - 1 ] = '\0';   /* Trim newline, if any       */

      operand = command = strtok( resp, WHITESPACE );
      if ( command == NULL )
         command = EMPTY_CMD;
      else if (Numeric(command))
         command = NUMERIC_CMD;

      while( cmd_ptr->sym != NULL)
      {
         if (equaln(command, cmd_ptr->sym, strlen(command)))
            break;            /* Exit if we have a hit               */
         cmd_ptr++;           /* Examine next command                */
      } /* while */

/*--------------------------------------------------------------------*/
/*     Get rest of command line, and trim leading spaces from it      */
/*--------------------------------------------------------------------*/

      if (!equal(command, NUMERIC_CMD) && (operand != NULL))
      {
         operand = strtok( NULL , "");
                              /* Save rest of string for later       */
         if ( operand != NULL )
         {
            while( isspace( *operand ))
               operand++ ;

            if (*operand == '\0')
               operand = NULL ;
         } /* if */
      }

/*--------------------------------------------------------------------*/
/*        Parse items to be selected from mailbox for command         */
/*--------------------------------------------------------------------*/

      if (cmd_ptr->bits & (LETTER_OP) )
         success = SelectItems( &operand, current, cmd_ptr->bits);

/*--------------------------------------------------------------------*/
/*                  Process the operands in the list                  */
/*--------------------------------------------------------------------*/

      while( success &&
             Get_Operand( &integer, &operand, cmd_ptr->bits, first_pass) )
      {
         switch( cmd_ptr->verb )
         {
            case M_ALIAS:
               ShowAlias( operand );
               break;

            case M_COPY:
               success = SaveItem( integer,
                         FALSE,        /* Do not delete */
                         seperators,   /* Do save headers */
                         (operand == NULL) ? "PRN" : operand ,
                         cmd_ptr->verb );
               break;

            case M_DEBUG:
               debuglevel = integer;
               printmsg(0,"Debug set to %d",debuglevel);
               break;

            case M_DELETEQ:
               done = TRUE;
            case M_DELETE:
               if (letters[integer].status < M_DELETED)
               {
                  letters[integer].status = M_DELETED;
                  if ( ! crlf )
                     printf("Deleting item(s) %d",integer + 1 );
                  else
                     printf(" %d",integer + 1 );
                  crlf  = modified = TRUE;
               }
               break;

            case M_DOWN:
               current = Position( 0 , integer , current );
               break;

            case M_EMPTY:
               if ( bflag[F_DOSKEY] && !bflag[F_EXPERT] )
               {
                  printf("DOSKEY active, empty line ignored\n");
                  PrintSubject( current , letternum );
                  success = FALSE;
               }
               else if (letters[current].status == M_UNREAD)
                  success = Pager( current , TRUE, noreceived, first_pass);
               else
                  current = Position( 0 , 1 , current );
               break;

            case M_EXIT:
               modified = FALSE;
               done     = TRUE;
               break;

            case M_EXTPRINT:
               success = Pager( integer , TRUE, noreceived, first_pass);
               break;

            case M_EXTTYPE:
               success = Pager( integer , TRUE, noseperator, first_pass);
               break;

            case M_FASTHELP:
            {
               size_t subscript = 0;
#ifndef _Windows
               size_t column    = 0;
#endif
               fputs("Valid commands are:\n",stdout);
               while( table[subscript].sym != NULL)
               {
                  if ( !(table[subscript].bits & NODISPLAY ))
                  {
#ifdef _Windows
                     fputc( '\n' , stdout );
#else
                     fputc( ( column++ % 2 ) ? ' ' : '\n' , stdout );
#endif
                     printf("%-9s%-30s",table[subscript].sym,
                                       table[subscript].help );
                  } /* if */
                  subscript ++;
               } /* while */
               fputs("\n\nEnter \"help\" for additional information.\n",
                        stdout);
               break;
            } /* case */

            case M_FORWARD:
               success = ForwardItem( integer, operand);
               break;

            case M_GOTO:
               current = Position( integer, 0, current );
               break;

            case M_HEADERS:
               PrintSubject( (cmd_ptr->bits & NO_OPERANDS) ?
                                 -1 : integer, letternum );
               break;

            case M_HELP:
            {
               char filename[FILENAME_MAX];
               mkfilename(filename, E_confdir, "mail.hlp");
               Sub_Pager(filename, TRUE );
               break;
            }

            case M_INTPRINT:
               success = Pager( integer , FALSE, noreceived, first_pass);
               break;

            case M_INTTYPE:
               success = Pager( integer , FALSE, noseperator, first_pass);
               break;

            case M_INVALID:
               printf("Invalid command \"%s\".  Enter \"?\" for help.\n",
                        command);
               break;

            case M_MAIL:
               success = DeliverMail( operand , current);
               break;

            case M_NOOP:
               break;

            case M_REPLY:
               success = Reply( integer );
               break;

            case M_QUIT:
               done = TRUE;
               break;

            case M_SAVE:
               success = SaveItem( integer,
                         TRUE,         /* Do delete */
                         seperators,   /* Do save headers */
                         operand ,
                         cmd_ptr->verb );
               modified = TRUE;
               break;

            case M_SET:
               if (operand == NULL)
                  sayoptions( configFlags);
               else
                  options(operand, USER_CONFIG, configFlags, bflag);
               break;

            case M_SYSTEM:
               subshell( operand );
               break;

            case M_UNDELETE:
               letters[integer].status = M_UNREAD;
               break;

            case M_UP:
               current = Position( 0 , - integer , current );
               break;

            case M_STATUS:
               printf("%s:\t%s created %s %s running under %s %d.%02d\n",
                       compilep, compilev, compiled, compilet,

#ifdef WIN32
                     "Windows NT",
                     _winmajor,
                     _winminor);
#elif defined(__OS2__)
                    "OS/2(R)" ,
                    (int) _osmajor / 10,
                      _osminor);
#elif defined(__TURBOC__)
                    "DOS",
                    _osmajor,
                    _osminor);
#else
                    (_osmode == DOS_MODE) ? "DOS" : "OS/2(R)" ,
                    (_osmode == DOS_MODE) ? _osmajor : ((int) _osmajor / 10 ),
                     _osminor);
#endif
#ifdef _Windows
               printf("Windows version: %s\t", compilew );
#endif
               printf("Magic Word:\t%s\tCurrent time:\t%s\n",
                     "flarp",
                     arpadate() );
               printf("Return address:\t\"%s\" <%s@%s>\n"
                      "Domain name:\t%s\tNodename:\t%s\n",
                        E_name, E_mailbox, E_fdomain, E_domain, E_nodename );
               printf("Current File:\t%s\tNumber of items: %d\n"
                      "File size:\t%ld bytes\tLast updated:\t%s",
                        mfilename, letternum + 1 , mboxsize ,
                        ctime( & mboxage ) );
               break;

            case M_WRITE:
               success = SaveItem( integer,
                         TRUE,      /* Do delete */
                         noheader,  /* Do not save headers */
                         operand,
                         cmd_ptr->verb );
               modified = TRUE;
         } /* switch */
         first_pass = FALSE;
      } /* while */

      success = ! first_pass; /* If first_pass not run, then
                                 Get_Operand failed                  */

      if ( crlf )
         putchar('\n');

      if ( success && !done )
      {
         if (cmd_ptr->bits & POSITION)
            current = Position( 0 , 0 , integer );

         if ( current != previous )
         {
            if ( (cmd_ptr->bits & AUTOPRINT ) &&
                  bflag[F_AUTOPRINT] &&
                  (letters[current].status != M_DELETED) )
               Pager( current , TRUE, noreceived, TRUE);
            else if ( !(cmd_ptr->bits & NOAUTOHEADER ) )
               PrintSubject( current , letternum );
         } /* if */
      } /* if */
   } /* while */

/*--------------------------------------------------------------------*/
/*                       End main command loop                        */
/*--------------------------------------------------------------------*/

   if (modified)
      UpdateMailbox(letternum, postoffice);

   free(letters);

} /*Interactive_Mail*/

/*--------------------------------------------------------------------*/
/*    I n c l u d e N e w                                             */
/*                                                                    */
/*    Includes mail from the system box into the user's local         */
/*    mailbox                                                         */
/*--------------------------------------------------------------------*/

static void IncludeNew( const char *target, const char *user)
{
   time_t age;
   long size;
   FILE *stream_in;
   FILE *stream_out;
   int  bytes;

   char sysbox[FILENAME_MAX];
   char buf[BUFSIZ];

   mkmailbox(sysbox, user);

/*--------------------------------------------------------------------*/
/*      Return semi-quietly if we can't open the system mailbox       */
/*--------------------------------------------------------------------*/

   stream_in   = FOPEN( sysbox, "r", BINARY_MODE);
   if ( stream_in == NULL )
   {
      if ( debuglevel > 1 )
         printerr( sysbox );
      return;
   }

/*--------------------------------------------------------------------*/
/*      Determine if we have new mail, returning quietly if not       */
/*--------------------------------------------------------------------*/

   age = stater( sysbox , &size );

   if ( age == (time_t) -1L)
      panic();

   printmsg( 1, "Including mail from %s through %.24s",
            sysbox,
            ctime(&age));

/*--------------------------------------------------------------------*/
/*                    Now open up the output file                     */
/*--------------------------------------------------------------------*/

   stream_out  = FOPEN( target, "a+", BINARY_MODE);

   if ( stream_out == NULL )
   {
      printerr( target );
      panic();
   }

/*--------------------------------------------------------------------*/
/*                       Loop to read the data                        */
/*--------------------------------------------------------------------*/

   while ((bytes = fread(buf,sizeof(char), sizeof buf, stream_in)) > 0)
   {
      if ((int) fwrite(buf, sizeof(char), bytes, stream_out) != bytes)
      {
         printmsg(0, "Error including new mail into %s", target );
         printerr( target );
         fclose( stream_in );
         fclose( stream_out );
         panic();
      }
   } /* while */

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   if ( ferror( stream_in ))
   {
      printerr( sysbox );
      panic();
   }

   fclose( stream_in  );
   fclose( stream_out );

   filebkup( sysbox );
   unlink(sysbox);

} /* IncludeNew */

/*--------------------------------------------------------------------*/
/*    C r e a t e B o x                                               */
/*                                                                    */
/*    Creates the temporary mailbox and related tables                */
/*--------------------------------------------------------------------*/

int CreateBox(FILE *rmailbox, FILE *fmailbox , const char *tmailbox)
{

/*--------------------------------------------------------------------*/
/*          Copy real mailbox file to temporary mailbox file          */
/*--------------------------------------------------------------------*/

   int letternum = 0;
   boolean inheader = FALSE;
   long position;
   char line[LSIZE];
   char **list;
   size_t replyprior = 0;
   size_t dateprior = 0;
   size_t subjectprior = 0;
   size_t fromprior = 0;

   struct ldesc *letter = NULL;

   while ((fgets(line, LSIZE, rmailbox) != nil(char)) ){

      if (inheader)
      {
         if (*line == '\n')
            inheader = FALSE;
      }  /* inheader */
      else {               /* Determine if starting new message   */
         if (equal(line,MESSAGESEP) ||
            (bflag[F_FROMSEP] && equaln(line, "From ", 5)))
         {
             while (equal(line,MESSAGESEP))
             if (fgets(line, LSIZE, rmailbox) == NULL)
             {
               printerr(mfilename);
               panic();
             } /* if */

/*--------------------------------------------------------------------*/
/*               Make the mailbox bigger if we need to                */
/*--------------------------------------------------------------------*/

             position = ftell(fmailbox);
             if ( (letternum+1) == maxletters )
             {
               maxletters = max((int) ((maxletters * mboxsize) / position),
                                 (letternum * 11) / 10 );
               printmsg(2,"Reallocating mailbox array from %d to %d entries",
                     letternum+1, maxletters );
               letters = realloc( letters, maxletters *  sizeof(letters[0]));
               checkref( letters );
             }

/*--------------------------------------------------------------------*/
/*             Initialize this entry in th mailbox array              */
/*--------------------------------------------------------------------*/

             letter = &letters[letternum++];

             fromprior = subjectprior = replyprior = dateprior = INT_MAX;
             letter->from = letter->subject = letter->date =
                  letter->replyto = MISSING;
             letter->adr = position;
             letter->status = M_UNREAD;
             letter->lines = 0L;
             inheader = TRUE;
             printf("Reading message %d (%d%% done)\r",letternum,
                        (int) (position * 100 / mboxsize));
         }
         else
         {
            if(letter == NULL)   /* Did we find first letter?     */
            {                    /* No --> Abort with message     */
               fprintf(stderr,"%s  %s\n\a",
                  "This mailbox is not in UUPC/extended format!",
                  bflag[F_FROMSEP] ?
                  "Messages must be seperated by From lines!" :
                  "(Try \"options=fromsep\" in your configuration file)");
               panic();
            } /* if */

            letter->lines++;
         } /* else */
      } /* else */

      if (inheader)
      {
         size_t priority = 0;

/*--------------------------------------------------------------------*/
/*              Search for the best Date: related field               */
/*--------------------------------------------------------------------*/

         while ( (dateprior > priority) && (datelist[priority] != NULL ))
         {
            if (equalni(line, datelist[priority],
                             strlen(datelist[priority]) ) )
            {
               letter->date = ftell(fmailbox);
               dateprior = priority;
            }
            priority++;
         }

/*--------------------------------------------------------------------*/
/*             Search for the best Subject: related field             */
/*--------------------------------------------------------------------*/

         priority = 0;
         while ( (subjectprior > priority) &&
                 (subjectlist[priority] != NULL ))
         {
            if (equalni(line, subjectlist[priority],
                             strlen(subjectlist[priority]) ) )
            {
               letter->subject = ftell(fmailbox);
               subjectprior = priority;
            }
            priority++;
         }

/*--------------------------------------------------------------------*/
/*           Search for the best From: header related field           */
/*--------------------------------------------------------------------*/

         list = (useto) ? tolist : fromlist;
         priority = 0;
         while ( (fromprior > priority) && (list[priority] != NULL ))
         {
            if (equalni(line, list[priority],
                             strlen(list[priority]) ) )
            {
               letter->from = ftell(fmailbox);
               fromprior = priority;
            }
            priority++;
         } /* while */

/*--------------------------------------------------------------------*/
/*             Search for the best Reply-To related field             */
/*--------------------------------------------------------------------*/

         priority = 0;
         while ( (replyprior > priority) &&
                 (replytolist[priority] != NULL ))
         {
            if (equalni(line, replytolist[priority],
                             strlen(replytolist[priority]) ) )
            {
               letter->replyto = ftell(fmailbox);
               replyprior = priority;
            } /* if */
            priority++;
         }  /* while */
      } /* inheader */

      if (fputs(line, fmailbox) == EOF )
      {
         printerr(tmailbox);
         panic();
      } /* if */


   } /* while */

   letters[letternum].adr = ftell(fmailbox);
   letters[letternum].status = M_DELETED;


   fclose(rmailbox);
   fclose(fmailbox);

   return letternum;

} /* CreateBox */

/*--------------------------------------------------------------------*/
/*    P r i n t S u j e c t                                           */
/*                                                                    */
/*    Print the subject line of one or all messages in the mailbox    */
/*--------------------------------------------------------------------*/

void PrintSubject(int msgnum,int letternum)
{
   struct ldesc *ld;
   char from[LSIZE];
   char subject[LSIZE];
   char date[LSIZE];
   char line[LSIZE];

   int k, mink, maxk;

   if (msgnum == -1)
   {                                         /* print all of them? */
      sprintf(line," %d messages in file %s.\n",letternum,mfilename);
      PageLine(line);
      mink = 0;
      maxk = letternum - 1;
   } else
      mink = maxk = msgnum;

   for (k = mink ; k <= maxk ; k++) {

      ld = &letters[k];
      if ((ld->status == M_DELETED) && (msgnum == -1))
         continue;

      ReturnAddress(from,ld);       /* Get return address for letter */

      /* Date: Wed May 13 23:59:53 1987 */
      *date = '\0';  /* default date to null */
      if (RetrieveLine(ld->date, date, LSIZE)) {
         sscanf(date, "%*s %*s %s %s", line, subject);
         sprintf(date, "%s %s", line, subject);
      }

      strcpy(subject, "--- no subject ---");
      if (RetrieveLine(ld->subject, line, LSIZE)) {
         register char  *sp;
         sp = line;
         while (!isspace(*sp))
            sp++;
         while (isspace(*sp))
            sp++;
         strcpy(subject, sp);
      }

      /* make sure the fields aren't too long */

      from[25] = '\0';
      date[6] = '\0';
      subject[30] = '\0';

      sprintf(line, "%3d%c %6s  %-25s  %-30s  (%5ld)\n", k + 1,
         ((ld->status == M_DELETED) ? '*' : ' '),
            date, from, subject, ld->lines);

      if (PageLine(line))
         break;

   }

} /*PrintSubject*/


/*--------------------------------------------------------------------*/
/*    U p d a t e  M a i l b o x                                      */
/*                                                                    */
/*    Update the permanent mailbox for the user                       */
/*--------------------------------------------------------------------*/

void UpdateMailbox(int letternum, boolean postoffice)
{
   int current;
   boolean changed = FALSE;
   boolean problem = FALSE;
   FILE *fmailbag;
   FILE *mbox = NULL;
   char *mboxname = NULL;
   long newsize;
   time_t newage;
   size_t msave = 0;
   size_t psave = 0;

/*--------------------------------------------------------------------*/
/*    Auto save into user's home directory mailbox if we were         */
/*    reading the system mailbox and the user specified the           */
/*    'save' option.                                                  */
/*--------------------------------------------------------------------*/

   postoffice = postoffice && bflag[F_SAVE];

/*--------------------------------------------------------------------*/
/*     Determine if anything was actually changed in the mailbox      */
/*--------------------------------------------------------------------*/

   for (current = 0;
        (current < letternum) && (! changed);
        current++)
   {
      if (letters[current].status == M_DELETED)
         changed = TRUE;

      if (postoffice && (letters[current].status != M_UNREAD))
         changed = TRUE;
   }

   if (!changed)
      return;

/*--------------------------------------------------------------------*/
/*    Determine if the mailbox has changed since we built our         */
/*    temporary file                                                  */
/*--------------------------------------------------------------------*/

   newage = stater( mfilename , &newsize );

   if ( mboxsize != newsize )
   {
      printf("%s size has changed from %ld to %ld bytes\n",
            mfilename, mboxsize, newsize );
      problem = TRUE;
   }

   if ( mboxage != newage )
   {
      char mboxbuf[DATEBUF];
      char newbuf[DATEBUF];
      printf("%s date stamp has changed from %s to %s\n",
            mfilename, dater(mboxage, mboxbuf), dater(newage, newbuf) );
      problem = TRUE;
   }

   while ( problem )
   {
      int c;

      printf("WARNING! File %s has changed, data may be lost if updated!\n",
            mfilename);
      fputs("Update anyway? ",stdout);

      c     = Get_One();

      switch (tolower( c ))
      {
         case 'y':
            puts("Yes");
            problem = FALSE;
            break;

         case 'n':
            printf("No\nUpdate aborted, %s left unchanged.\n",
                     mfilename);
            return;

         default:
            printf("%c - Invalid Response\n",c);
            break;
      } /* switch */
   } /* while ( problem ) */

/*--------------------------------------------------------------------*/
/*                Allocate auto save related variables                */
/*--------------------------------------------------------------------*/

   if (postoffice)
   {
      mboxname = malloc(FILENAME_MAX);
      checkref(mboxname);
      strcpy( mboxname, "mbox" );
      expand_path( mboxname, E_homedir, E_homedir, E_mailext );
   } /* if (postoffice) */

/*--------------------------------------------------------------------*/
/*                   Create a backup file if needed                   */
/*--------------------------------------------------------------------*/

   if ( bflag[F_BACKUP] )
      filebkup( mfilename );

/*--------------------------------------------------------------------*/
/*                    Begin re-writing the mailbox                    */
/*--------------------------------------------------------------------*/

   if ((fmailbag = FOPEN(mfilename, "w",TEXT_MODE)) == nil(FILE))
   {
      printf("UpdateMailbox: can't rewrite %s.\n", mfilename);
      Cleanup();
   } /* if */

   setvbuf(fmailbag, NULL, _IOFBF, 8192);

/*--------------------------------------------------------------------*/
/*    We got the files open, now actually loop through copying        */
/*    data from our temporary mailbox back into the permenent one,    */
/*    or in the user's mbox if he read the message and the post       */
/*    office is open.                                                 */
/*--------------------------------------------------------------------*/

   printf("Cleaning up ", current+ 1);

   for (current = 0; current < letternum;  current++)
   {
      if (letters[current].status == M_DELETED)
      {
         /* No operation */
         fputc('.', stdout);
      }
      else if (postoffice && (letters[current].status != M_UNREAD))
      {

         if ( mbox == NULL )  /* Mailbox already open?               */
         {                    /* No --> Do so now                    */
            mbox = FOPEN(mboxname, "a",TEXT_MODE);
            if (mbox == NULL) /* Open fail?                          */
            {                  /* Yes --> Disable postoffice autosave*/
               printf("\nUpdateMailbox: can't append to %s.\n", mboxname);
               postoffice = FALSE;
               current--;     /* Process this entry again            */
            } /* if */
            else
               setvbuf(mbox, NULL, _IOFBF, 8192);
         } /* if ( mbox == NULL ) */

         if ( mbox != NULL )
         {
            fputc('+', stdout);
            CopyMsg(current, mbox, seperators, FALSE);
            msave++;
         } /* mbox */
      }
      else {
         fputc('*', stdout);
         CopyMsg(current, fmailbag, seperators, FALSE);
         psave ++;
      } /* else */

   } /* for */
   fputs(" done!\n", stdout);

/*--------------------------------------------------------------------*/
/*    Close the post office.  We close the 'mbox' in the user's       */
/*    home directory, report if any data was saved in it, and         */
/*    then free the storage associated with the postoffice processing.*/
/*--------------------------------------------------------------------*/

   if ( postoffice )
   {

      if (msave > 0)          /* Did we write data into mailbox?  */
      {                       /* Yes --> Report it                */
         fclose(mbox);
         printf("%d letter%s saved in %s%s",
               msave,
               (msave > 1) ? "s" : "" ,
               mboxname,
               (psave > 0) ? ", " : ".\n");
      }

      free(mboxname);
   } /* if (postoffice) */

/*--------------------------------------------------------------------*/
/*    Now, clean up after the input mailbox.  We close it, and        */
/*    report when anything was saved into it.  If nothing was         */
/*    saved, we delete the file if the 'purge' option is active.      */
/*--------------------------------------------------------------------*/

   fclose(fmailbag);

   if (psave > 0)
      printf("%d letter%s held in %s.\n",
            psave ,
            (psave > 1) ? "s" : "" , mfilename);
   else if (bflag[F_PURGE] )
   {
      remove(mfilename);
      printf("Empty mail box %s has been deleted.\n", mfilename);
   }

} /* UpdateMailbox */

/*--------------------------------------------------------------------*/
/*    U s a g e                                                       */
/*                                                                    */
/*    Report command line syntax                                      */
/*--------------------------------------------------------------------*/

static void usage( void )
{
   puts("\nUsage:\tmail [-s subject] recipient ... "
         "[-c recipient ...] [-b receipient ...]\n"
         "\tmail [-f mailbox] [-u user] [-t] [-p] [-x debug]");
   exit(1);
}
