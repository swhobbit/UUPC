/*--------------------------------------------------------------------*/
/*       m a i l . c                                                  */
/*                                                                    */
/*       UUPC/extended Mailer User-Agent (MUA)                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mail.c 1.54 2001/03/12 13:55:29 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: mail.c $
 *    Revision 1.54  2001/03/12 13:55:29  ahd
 *    Annual copyright update
 *
 *    Revision 1.53  2000/05/12 12:35:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.52  1999/02/21 04:09:32  ahd
 *    Print better message router init fails
 *
 *    Revision 1.51  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.50  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.49  1998/11/04 01:59:55  ahd
 *    Prevent buffer overflows when processing UIDL lines
 *    in POP3 mail.
 *    Add internal sanity checks for various client structures
 *    Convert various files to CR/LF from LF terminated lines
 *
 * Revision 1.48  1998/03/16  07:47:40  ahd
 * Add IUDL to ignored header list
 *
 *    Revision 1.47  1998/03/03 03:53:54  ahd
 *    Routines to handle messages within a POP3 mailbox
 *
 *    Revision 1.46  1998/03/01 01:31:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.45  1998/01/03 05:24:17  ahd
 *    Force load of user table to allow use of passwd definition
 *    of home
 *
 *    Revision 1.44  1997/04/24 01:08:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.43  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.42  1996/01/20 12:58:08  ahd
 *    Specify test/binary when opening in-memory files
 *
 *    Revision 1.41  1996/01/04 04:00:46  ahd
 *    Use sorted list of boolean options with binary search and computed
 *    table size.
 *
 *    Revision 1.40  1996/01/01 23:50:26  ahd
 *    Don't scan nickname table for duplicate nicknames in linear fashion,
 *    merely check entire table for duplicates after sorting.
 *    Rename user functions previously known as 'user alias' to 'nickname',
 *    consistent with newer documentation.
 *
 *    Revision 1.39  1996/01/01 21:02:35  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.38  1995/03/11 22:27:46  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.37  1995/03/08 02:58:08  ahd
 *    Delete KW from literal boolean strings
 *
 *    Revision 1.36  1995/02/21 02:47:44  ahd
 *    The compiler warnings war never ends!
 *
 *    Revision 1.35  1995/02/20 17:38:19  ahd
 *    Determine OS/2 16 bit at compile time
 *
 *    Revision 1.34  1995/02/20 17:28:43  ahd
 *    in-memory file support, 16 bit compiler clean up
 *
 *    Revision 1.33  1995/01/07 16:26:58  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.32  1995/01/07 16:18:46  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 */

#include "uupcmoah.h"

RCSID("$Id: mail.c 1.54 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <io.h>
#include <limits.h>
#ifndef __32BIT__
#include <dos.h>
#endif
#include <direct.h>

#ifdef _Windows
#include <windows.h>
#endif

#ifndef BIT32ENV
#include <malloc.h>        /* _fmalloc, etc.                         */
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "address.h"
#include "nickname.h"
#include "dater.h"
#include "expath.h"
#include "getopt.h"
#include "mail.h"
#include "mailblib.h"
#include "maillib.h"
#include "mailsend.h"
#include "mailnewm.h"
#include "mlib.h"
#include "pushpop.h"
#include "stater.h"
#include "timestmp.h"
#include "arpadate.h"
#include "usertabl.h"

#include "title.h"

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

static char mfilename[FILENAME_MAX];
int letternum = 0;

static KWBoolean useto = KWFalse;

IMFILE *imBox;

#ifdef BIT32ENV
static size_t maxLetters = 1000;    /* Initial value only            */
#else
static size_t maxLetters = 100;     /* Initial value only            */
#endif

LDESC UUFAR  *letters;

/*--------------------------------------------------------------------*/
/*                       Local procedure names                        */
/*--------------------------------------------------------------------*/

static void    Cleanup(void);

static void Interactive_Mail( const KWBoolean PrintOnly,
                              const KWBoolean postoffice );

static void    PrintSubject(const int msgnum,
                            const int letternum);

static void    UpdateMailbox(int letternum, KWBoolean postoffice);

static int     CreateBox(FILE *rmailbox );

static void usage( void );

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char *defaultReplyToList[] =
  {
     "Resent-Reply-To:",
     "Resent-From:",
     "Reply-To:",
     "From:",
      NULL
   };

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
                               "Date:",
                               NULL} ;

static char *defaultIgnoreList[] =
      {
         "Message-ID:",
         "Received:",
         "Status: ",
         "X-Mailer: ",
         "From ",
         "Precedence: ",
         "Path: ",
         "Lines: ",
         "References: ",
         UIDL_HEADER,
         NULL,
       };

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
 { EMPTY_CMD,     M_EMPTY,    NODISPLAY | NO_OPERANDS | AUTOPRINT,
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
 { "copy",        M_COPY,     LETTER_OP | FILE_OP,
         "Copy item to file"},
 { "delete",      M_DELETE,   LETTER_OP | POSITION | AUTOPRINT,
         "Delete mail item"},
 { "debug",       M_DEBUG,    KEWSHORT_OP,
         "Enable debug output"},
 { "dquit",       M_DELETEQ,  LETTER_OP,
         "Delete then quit"},
 { "exit",        M_EXIT,     NO_OPERANDS,
         "Exit without updating mailbox"},
 { "forward",     M_FORWARD,  LETTER_OP | USER_OP,
         "Resend item to others"},
 { "go",          M_GOTO,     LETTER_OP | AUTOPRINT,
         "Go to item"},
 { "Headers",     M_HEADERS,  LETTER_OP | POSITION | NOAUTOHEADER,
         "Print specified item summary"},
 { "headers",     M_HEADERS,  NO_OPERANDS | NOAUTOHEADER,
         "Print all item summaries"},
 { "help",        M_HELP,     NO_OPERANDS,
         "Print long help text"},
 { "mail",        M_MAIL,     USER_OP,
         "Compose and send mail"},
 { "next",        M_DOWN,     KEWSHORT_OP | AUTOPRINT,
         "Move to next item"},
  {"print",       M_EXTPRINT, LETTER_OP | POSITION,
         "Print item (condensed)"},
  {"Print",       M_INTPRINT, LETTER_OP | POSITION,
         "Print item (condensed)"},
  {"previous",    M_UP,       KEWSHORT_OP | AUTOPRINT,
         "Move to previous item"},
  {"quit",        M_QUIT,     NO_OPERANDS,
         "Update mailbox, exit"},
  {"reply",       M_REPLY,    LETTER_OP | POSITION,
         "Reply to sender of item"},
  {"save",        M_SAVE,     LETTER_OP | FILE_OP | POSITION | AUTOPRINT,
         "Copy item, delete"},
  {"set",         M_SET,      STRING_OP,
         "Print/set boolean options"},
  {"status",     M_STATUS,  NO_OPERANDS,
         "Report version/status info"},
  {"type",        M_EXTTYPE,  LETTER_OP | POSITION,
         "Print item with all headers"},
  {"Type",        M_INTTYPE,  LETTER_OP | POSITION,
         "Print item with all headers"},
  {"undelete",    M_UNDELETE, LETTER_OP | POSITION | AUTOPRINT,
         "Rescue item after save/delete"},
  {"write",       M_WRITE,    LETTER_OP | FILE_OP | POSITION | AUTOPRINT,
         "Copy item w/o header, delete"},
  {"xit",         M_EXIT,     NO_OPERANDS,
         "alias for exit"},
  { NUMERIC_CMD,   M_GOTO,     NODISPLAY | KEWSHORT_OP | AUTOPRINT,
         NULL},
  { NULL,          M_INVALID,  NODISPLAY | STRING_OP,
         NULL }
         } ;

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

main(int argc, char **argv)
{

   KWBoolean PrintOnly = KWFalse;
   KWBoolean postoffice = KWTrue;
   KWBoolean readmail  = KWFalse;
   KWBoolean sendmail  = KWFalse;
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
   {
      panic();    /* system configuration failed */
   }

   checkuser(E_mailbox);         /* Force load of user table         */

   PushDir(".");

/*--------------------------------------------------------------------*/
/*                       get mailbox file name                        */
/*--------------------------------------------------------------------*/

   strcpy( mfilename, E_mailbox );
   if ( strchr( mfilename,'.' ) == NULL )
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
         readmail = KWTrue;
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
             (expand_path( strcpy( oname, E_filesent),
                          E_homedir, E_homedir, E_mailext ) != NULL ) &&
             equali( oname, mfilename ))
                           /* Our outgoing filename?              */

            useto = (KWBoolean) ((! useto) ? KWTrue : KWFalse) ;
                                    /* Yes --> Automatically switch  */
         postoffice = KWFalse;
         break;

      case 'p':
         readmail = KWTrue;
         PrintOnly = KWTrue;
         break;

      case 'u':                  /* Read alternate mailbox?       */
         readmail = KWTrue;
         mkmailbox(mfilename, optarg);
         postoffice = KWFalse;
         break;

      case 'x':
         debuglevel = atoi(optarg);
         break;

      case 's':
         sendmail = KWTrue;
         subject = optarg;
         break;

      case 't':
         readmail = KWTrue;
         useto = (KWBoolean) ((! useto) ? KWTrue : KWFalse) ;
         break;

      case '?':
         usage();

      } /* switch */

   } /* while */

/*--------------------------------------------------------------------*/
/*                        Check for conflicts                         */
/*--------------------------------------------------------------------*/

   if (optind != argc)
      sendmail = KWTrue;

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

         Collect_Mail(stdin, argc+2, argv, -1, KWFalse);
      } /* if ( subject != NULL ) */
      else {
         Collect_Mail(stdin, argc, &argv[optind], -1, KWFalse);

#ifdef _Windows
         atexit ( CloseEasyWin );
#endif
      }

   } /* if (sendmail) */
   else  {
      if ( postoffice && bflag[ F_MULTITASK ] )
      {
         if ( !IncludeNew( mfilename, E_mailbox) )
            panic();
      }
      Interactive_Mail( PrintOnly, postoffice );
   }

   Cleanup();
   PopDir();

   return 0;

} /*main*/

/*--------------------------------------------------------------------*/
/*    C l e a n u p                                                   */
/*                                                                    */
/*    Remove temporary files when exiting                             */
/*--------------------------------------------------------------------*/

void Cleanup()
{

   if ( imBox != NULL )
   {
      imclose( imBox );
      imBox = NULL;
   }

} /*Cleanup*/

/*--------------------------------------------------------------------*/
/*       s t a t u s                                                  */
/*                                                                    */
/*       Report current mailbox status                                */
/*--------------------------------------------------------------------*/

static void
status ( void )
{

   printf("%s:\t%s created %s %s running under %s %d.%02d\n",
           compilep, compilev, compiled, compilet,

#ifdef WIN32
         "Windows NT",
         _winmajor,
         _winminor);
#elif defined(__OS2__) || defined(FAMILYAPI)
        "OS/2(R)",
        (int) _osmajor / 10,
          _osminor);
#else
        "DOS",
        _osmajor,
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
            E_name,
            E_mailbox,
            E_fdomain,
            E_domain,
            E_nodename );

   printf("Current File:\t%s\tNumber of items: %d\n"
          "File size:\t%ld bytes\tLast updated:\t%s",
            mfilename,
            letternum,
            mboxsize,
            ctime( & mboxage ) );

} /* status */

/*--------------------------------------------------------------------*/
/*       f a s t H e l p                                              */
/*                                                                    */
/*       Print internal command summary                               */
/*--------------------------------------------------------------------*/

static void fastHelp( void )
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
         fputc( '\n', stdout );
#else
         fputc( ( column++ % 2 ) ? ' ' : '\n', stdout );
#endif
         printf("%-9s%-30s",table[subscript].sym,
                           table[subscript].help );

      } /* if */

      subscript ++;

   } /* while */

   fputs("\n\nEnter \"help\" for additional information.\n",
            stdout);

} /* fastHelp */

/*--------------------------------------------------------------------*/
/*    I n t e r a c t i v e _ M a i l                                 */
/*                                                                    */
/*    main procedure for reading mail                                 */
/*--------------------------------------------------------------------*/

static void Interactive_Mail( const KWBoolean PrintOnly,
                              const KWBoolean postoffice )
{
   char resp[LSIZE];
   int current = 0;
   KWBoolean done     = KWFalse;
   KWBoolean modified = KWFalse;
   FILE *rmailbox;

/*--------------------------------------------------------------------*/
/*                      Handle special defaults                       */
/*--------------------------------------------------------------------*/

   if ( E_ignoreList == NULL )
      E_ignoreList = defaultIgnoreList;

   if ( E_replyToList == NULL )
      E_replyToList = defaultReplyToList;

/*--------------------------------------------------------------------*/
/*               Open real and temporary mailbox files                */
/*--------------------------------------------------------------------*/

   if ((rmailbox = FOPEN(mfilename, "r",TEXT_MODE)) == nil(FILE))
   {
      printf("No mail in %s\n", mfilename);
      return;
   }

   if (setvbuf(rmailbox, NULL, _IOFBF, 8192))
   {
      printerr( mfilename );
      panic();
   }

   mboxage = stater( mfilename, &mboxsize );
                              /* Remember mailbox information        */

   imBox  = imopen( mboxsize, IMAGE_MODE );

   if ( imBox == NULL )
   {
      printerr("imopen" );
      return;
   }

   letters = (LDESC UUFAR*) MALLOC( maxLetters * sizeof *letters);

   checkref(letters);

/*--------------------------------------------------------------------*/
/*                 Copy real mailbox to temporary one                 */
/*--------------------------------------------------------------------*/

   letternum = CreateBox( rmailbox );

   rmailbox = NULL;

   if (letternum < 1)            /* Did we find any mail in the box? */
   {                             /* No --> Return to caller          */
      if (letternum == 0)
         printf("No mail in %s\n", mfilename);
      return;
   }

/*--------------------------------------------------------------------*/
/*        Shrink mailbox status array to what we actually need        */
/*--------------------------------------------------------------------*/

   letters = (LDESC UUFAR *) REALLOC( letters, (size_t) (letternum + 1) * sizeof *letters);

   checkref(letters);

   if ( postoffice && (!PrintOnly))
      modified = KWTrue;

   if (PrintOnly)
   {
      int j = 0;
      while (j < letternum)
      {
         Pager(j,
               KWTrue,
               ignoresome,
               (KWBoolean) ((!j) ? KWTrue : KWFalse ));
         j++ ;
      }
      return;
   }

   setTitle("%s (user %s)", mfilename, E_mailbox );
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
      size_t length;
      int    integer;
      KWBoolean firstPass = KWTrue;
      int previous = current;
      struct CommandTable *cmd_ptr = table;
      KWBoolean success = KWTrue;
      KWBoolean crlf   = KWFalse;     /* crlf after delete command?    */

      printf("%d%s",current + 1,
               (letters[current].status == M_DELETED) ? "*" : " ");

      if (!Console_fgets(resp, LSIZE, "? ")) /* End of file?         */
      {
         done = KWTrue;
         continue;            /* Yes --> Exit loop                   */
      }

      PageReset();

/*--------------------------------------------------------------------*/
/*                     Locate command to execute                      */
/*--------------------------------------------------------------------*/

      length = strlen( resp );

      if (length && ( resp[ length - 1 ] == '\n'))
         resp[ length - 1 ] = '\0';   /* Trim newline, if any       */

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
         operand = strtok( NULL, "");
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
             Get_Operand( &integer, &operand, cmd_ptr->bits, firstPass) )
      {
         switch( cmd_ptr->verb )
         {
            case M_ALIAS:
               ShowAlias( operand );
               break;

            case M_COPY:
               success = SaveItem( integer,
                         KWFalse,       /* Do not delete */
                         seperators,   /* Do save headers */
                         (operand == NULL) ? "PRN:" : operand,
                         cmd_ptr->verb );
               break;

            case M_DEBUG:
               debuglevel = integer;
               printmsg(0,"Debug set to %d",debuglevel);
               break;

            case M_DELETEQ:
               done = KWTrue;
               /* Fall through */

            case M_DELETE:
               if (letters[integer].status < M_DELETED)
               {
                  letters[integer].status = M_DELETED;

                  if ( ! crlf )
                     printf("Deleting item(s) %d",integer + 1 );
                  else
                     printf(" %d", integer + 1 );

                  crlf  = modified = KWTrue;
               }

               break;

            case M_DOWN:
               current = Position( 0, integer, current );
               break;

            case M_EMPTY:
               if ( bflag[F_DOSKEY] && !bflag[F_EXPERT] )
               {
                  printf("DOSKEY active, empty line ignored\n");
                  PrintSubject( current, letternum );
                  success = KWFalse;
               }
               else if (letters[current].status == M_UNREAD)
                  success = Pager( current, KWTrue, ignoresome, firstPass);
               else
                  current = Position( 0, 1, current );

               break;

            case M_EXIT:
               modified = KWFalse;
               done     = KWTrue;
               break;

            case M_EXTPRINT:
               success = Pager( integer, KWTrue, ignoresome, firstPass);
               break;

            case M_EXTTYPE:
               success = Pager( integer, KWTrue, noseperator, firstPass);
               break;

            case M_FASTHELP:
               fastHelp();
               break;

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
               Sub_Pager(filename, KWTrue );
               break;
            }

            case M_INTPRINT:
               success = Pager( integer, KWFalse, ignoresome, firstPass);
               break;

            case M_INTTYPE:
               success = Pager( integer, KWFalse, noseperator, firstPass);
               break;

            case M_INVALID:
               printf("Invalid command \"%s\".  Enter \"?\" for help.\n",
                        command);
               break;

            case M_MAIL:
               success = DeliverMail( operand, current);
               break;

            case M_NOOP:
               break;

            case M_REPLY:
               success = Reply( integer );
               break;

            case M_QUIT:
               done = KWTrue;
               break;

            case M_SAVE:
               success = SaveItem( integer,
                         KWTrue,        /* Do delete */
                         seperators,   /* Do save headers */
                         operand,
                         cmd_ptr->verb );
               modified = KWTrue;
               break;

            case M_SET:
               if (operand == NULL)
                  sayOptions( configFlags, configFlagsSize );
               else
                  options(operand,
                          USER_CONFIG,
                          configFlags,
                          bflag,
                          configFlagsSize );
               break;

            case M_SYSTEM:
               subshell( operand );
               break;

            case M_UNDELETE:
               letters[integer].status = M_UNREAD;
               break;

            case M_UP:
               current = Position( 0, - integer, current );
               break;

            case M_STATUS:
               status();
               break;

            case M_WRITE:
               success = SaveItem( integer,
                         KWTrue,     /* Do delete */
                         noheader,  /* Do not save headers */
                         operand,
                         cmd_ptr->verb );

               modified = KWTrue;

         } /* switch */

         firstPass = KWFalse;

#ifdef UDEBUG
         printmsg( 2, "success %s, integer %d",
                     success ? "True" : "False",
                     integer );
#endif

      } /* while */

      if ( firstPass )
         success = KWFalse;    /* If firstPass not run, then
                                  Get_Operand failed                  */

      if ( crlf )
         fputc('\n', stdout);

      if ( success && !done )
      {
         if (cmd_ptr->bits & POSITION)
            current = Position( 0, 0, integer );

         if ( current != previous )
         {

            if ( (cmd_ptr->bits & AUTOPRINT ) &&
                  bflag[F_AUTOPRINT] &&
                  (letters[current].status != M_DELETED) )
               Pager( current, KWTrue, ignoresome, KWTrue);
            else if ( !(cmd_ptr->bits & NOAUTOHEADER ) )
               PrintSubject( current, letternum );

         } /* if */

      } /* if */

   } /* while */

/*--------------------------------------------------------------------*/
/*                       End main command loop                        */
/*--------------------------------------------------------------------*/

   if (modified)
      UpdateMailbox(letternum, postoffice);

} /*Interactive_Mail*/

/*--------------------------------------------------------------------*/
/*    C r e a t e B o x                                               */
/*                                                                    */
/*    Creates the temporary mailbox and related tables                */
/*--------------------------------------------------------------------*/

int CreateBox(FILE *rmailbox )
{

/*--------------------------------------------------------------------*/
/*          Copy real mailbox file to temporary mailbox file          */
/*--------------------------------------------------------------------*/

   int letternum = 0;
   KWBoolean inHeader = KWFalse;
   char line[LSIZE];
   char **list;
   size_t replyprior = 0;
   size_t dateprior = 0;
   size_t subjectprior = 0;
   size_t fromprior = 0;
   int current = INT_MAX;

   while ((fgets(line, LSIZE, rmailbox) != nil(char)) )
   {

      if (inHeader)
      {
         if (*line == '\n')
            inHeader = KWFalse;
      }  /* inHeader */
      else {               /* Determine if starting new message   */

         if (equal(line,MESSAGESEP) ||
            (bflag[F_FROMSEP] && equaln(line, "From ", 5)))
         {
             long position;

             while (equal(line,MESSAGESEP))
             if (fgets(line, LSIZE, rmailbox) == NULL)
             {
               printerr(mfilename);
               panic();
             } /* if */

/*--------------------------------------------------------------------*/
/*               Make the mailbox bigger if we need to                */
/*--------------------------------------------------------------------*/

            current = letternum++;

             position = imtell(imBox);
             if ( letternum == (int) maxLetters )
             {
               maxLetters = (maxLetters * mboxsize) / position;

               if ( maxLetters < (size_t) (letternum * 11) / 10 )
                  maxLetters = (size_t) (letternum * 11) / 10;

               printmsg(2,"\nReallocating mailbox array from %d to %d entries",
                     current,
                     maxLetters );

               letters = (LDESC UUFAR *) REALLOC( letters,
                                                  maxLetters * sizeof *letters);
               checkref( letters );
             }

/*--------------------------------------------------------------------*/
/*             Initialize this entry in the mailbox array             */
/*--------------------------------------------------------------------*/

             fromprior = subjectprior = replyprior = dateprior = INT_MAX;
             letters[current].from    = MISSING;
             letters[current].subject = MISSING;
             letters[current].date    = MISSING;
             letters[current].replyto = MISSING;
             letters[current].adr     = position;
             letters[current].status  = M_UNREAD;
             letters[current].lines   = 0L;
             inHeader = KWTrue;

             printf("Reading message %d (%d%% done)\r",
                        letternum,
                        (int) (position * 100 / mboxsize));
         }
         else {
            if( ! letternum )    /* Did we find first letter?     */
            {                    /* No --> Abort with message     */
               fprintf(stderr,"%s  %s\n\a",
                  "This mailbox is not in UUPC/extended format!",
                  bflag[F_FROMSEP] ?
                  "Messages must be seperated by From lines!" :
                  "(Try \"options=fromsep\" in your configuration file)");
               panic();
            } /* if */

            letters[current].lines++;
         } /* else */
      } /* else */

      if (inHeader)
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
               letters[current].date = imtell( imBox );
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
               letters[current].subject = imtell( imBox );
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
               letters[current].from = imtell( imBox );
               fromprior = priority;
            }
            priority++;
         } /* while */

/*--------------------------------------------------------------------*/
/*             Search for the best Reply-To related field             */
/*--------------------------------------------------------------------*/

         priority = 0;
         while ( (replyprior > priority) &&
                 (E_replyToList[priority] != NULL ))
         {
            if (equalni(line, E_replyToList[priority],
                             strlen(E_replyToList[priority]) ) )
            {
               letters[current].replyto = imtell( imBox );
               replyprior = priority;
            } /* if */
            priority++;
         }  /* while */
      } /* inHeader */

      if (imputs(line, imBox) == EOF )
      {
         printerr("imputs");
         panic();
      } /* if */

   } /* while */

   letters[letternum].adr    = imtell( imBox );
   letters[letternum].status = M_DELETED;

   fclose(rmailbox);

   return letternum;

} /* CreateBox */

/*--------------------------------------------------------------------*/
/*    P r i n t S u j e c t                                           */
/*                                                                    */
/*    Print the subject line of one or all messages in the mailbox    */
/*--------------------------------------------------------------------*/

void PrintSubject(const int msgnum,
                  const int letternum)
{
   char from[MAXADDR];
   char subject[LSIZE];
   char date[LSIZE];
   char line[LSIZE];

   int current, mink, maxk;

   if (msgnum == -1)
   {                                         /* print all of them? */
      sprintf(line," %d messages in file %s.\n",
              letternum,
              mfilename);
      PageLine(line);
      mink = 0;
      maxk = letternum - 1;
   }
   else
      mink = maxk = msgnum;

   for (current = mink ; current <= maxk ; current++)
   {

      if ((letters[current].status == M_DELETED) && (msgnum == -1))
         continue;

      memset(from,'?',sizeof from );

      ReturnAddress(from, letters[current].from);

      /* Date: Wed May 13 23:59:53 1987 */
      *date = '\0';  /* default date to null */

      if (RetrieveLine(letters[current].date, date, LSIZE))
      {
         sscanf(date, "%*s %*s %s %s", line, subject);
         sprintf(date, "%s %s", line, subject);
      }

      strcpy(subject, "--- no subject ---");

      if (RetrieveLine(letters[current].subject, line, LSIZE))
      {
         register char  *sp = line;

         while (*sp && !isspace(*sp))
            sp++;

         while (isspace(*sp))
            sp++;

         strcpy(subject, sp);
      }

/*--------------------------------------------------------------------*/
/*                    Format the line and print it                    */
/*--------------------------------------------------------------------*/

      sprintf(line, "%3d%c %6.6s  %-25.25s  %-30.30s  (%5ld)\n",
         current + 1,
         ((letters[current].status == M_DELETED) ? '*' : ' '),
         date,
         from,
         subject,
         letters[current].lines);

      if (PageLine(line))
         break;

   } /* for (current = mink ; k <= maxk ; current++) */

} /* PrintSubject */

/*--------------------------------------------------------------------*/
/*    U p d a t e  M a i l b o x                                      */
/*                                                                    */
/*    Update the permanent mailbox for the user                       */
/*--------------------------------------------------------------------*/

void UpdateMailbox(int letternum, KWBoolean postoffice)
{
   int current;
   KWBoolean changed = KWFalse;
   KWBoolean problem = KWFalse;
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

   if ( ! bflag[F_SAVE] )
   postoffice = KWFalse;

/*--------------------------------------------------------------------*/
/*     Determine if anything was actually changed in the mailbox      */
/*--------------------------------------------------------------------*/

   for (current = 0;
        (current < letternum) && (! changed);
        current++)
   {
      if (letters[current].status == M_DELETED)
         changed = KWTrue;

      if (postoffice && (letters[current].status != M_UNREAD))
         changed = KWTrue;
   }

   if (!changed)
      return;

/*--------------------------------------------------------------------*/
/*    Determine if the mailbox has changed since we built our         */
/*    temporary file                                                  */
/*--------------------------------------------------------------------*/

   newage = stater( mfilename, &newsize );

   if ( mboxsize != newsize )
   {
      printf("%s size has changed from %ld to %ld bytes\n",
            mfilename, mboxsize, newsize );
      problem = KWTrue;
   }

   if ( mboxage != newage )
   {
      char mboxbuf[DATEBUF];
      char newbuf[DATEBUF];
      printf("%s date stamp has changed from %s to %s\n",
            mfilename, dater(mboxage, mboxbuf), dater(newage, newbuf) );
      problem = KWTrue;
   }

   while ( problem )
   {
      int c;

      printf("WARNING! File %s has changed, data may be lost if updated!\n",
            mfilename);
      fputs("Update anyway? ",stdout);
      fflush(stdout);

      c     = Get_One();

      switch (tolower( c ))
      {
         case 'y':
            puts("Yes");
            problem = KWFalse;
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

   if ((fmailbag = FOPEN(mfilename, "w", TEXT_MODE)) == nil(FILE))
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
         fflush(stdout);
      }
      else if (postoffice && (letters[current].status != M_UNREAD))
      {

         if ( mbox == NULL )  /* Mailbox already open?               */
         {                    /* No --> Do so now                    */
            mbox = FOPEN(mboxname, "a",TEXT_MODE);
            if (mbox == NULL) /* Open fail?                          */
            {                  /* Yes --> Disable postoffice autosave*/
               printf("\nUpdateMailbox: can't append to %s.\n", mboxname);
               postoffice = KWFalse;
               current--;     /* Process this entry again            */
            } /* if */
            else
               setvbuf(mbox, NULL, _IOFBF, 8192);
         } /* if ( mbox == NULL ) */

         if ( mbox != NULL )
         {
            fputc('+', stdout);
            fflush(stdout);
            CopyMsg(current, mbox, seperators, KWFalse);
            msave++;
         } /* mbox */
      }
      else {
         fputc('*', stdout);
         fflush(stdout);
         CopyMsg(current, fmailbag, seperators, KWFalse);
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
               (msave > 1) ? "s" : "",
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
            psave,
            (psave > 1) ? "s" : "", mfilename);
   else if (bflag[F_PURGE] )
   {
      REMOVE(mfilename);
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

}  /* usage */
