/*--------------------------------------------------------------------*/
/*    m a i l s e n d . c                                             */
/*                                                                    */
/*    Subroutines for sending mail for UUPC/extended                  */
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
 *    $Id: mailsend.c 1.9 1993/10/31 19:04:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: mailsend.c $
 * Revision 1.9  1993/10/31  19:04:03  ahd
 * Trap no subject after subject (-s) flag when sending mail
 *
 * Revision 1.8  1993/10/12  01:32:08  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.6  1993/10/04  03:57:20  ahd
 * Clarify error message
 *
 * Revision 1.5  1993/08/02  03:24:59  ahd
 * Further changes in support of Robert Denny's Windows 3.x support
 *
 * Revision 1.4  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

#include "arpadate.h"
#include "expath.h"
#include "execute.h"
#include "hlib.h"
#include "mlib.h"
#include "alias.h"
#include "mail.h"
#include "maillib.h"
#include "mailblib.h"
#include "mailsend.h"
#include "safeio.h"
#include "address.h"

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

 static char *ExplodeAlias(char *header ,
                      const char *alias,
                      FILE *stream,
                      const boolean resent);


 static void PutHead( const char *label,
                      const char *operand,
                      FILE *stream,
                      const boolean resent);

 static boolean Append_Signature(FILE *mailbag,
                     const boolean alternate);

 static void Prompt_Input( char *tmailbag,
                          FILE *fmailbag,
                           char *subject,
                          const int current);

 static boolean Subcommand( char *buf,
                           FILE *fmailbag,
                           char *tmailbag,
                           char *subject,
                           const int current_msg);

 static void CopyOut( const char* input);

 static void filter( char *tmailbag, char *command);

 static char *GetString( char *input);

currentfile();                /* Define current file for panic()     */

/*--------------------------------------------------------------------*/
/*    E x p l o d e A l i a s                                         */
/*                                                                    */
/*    Resolves an alias, exploding it into a list if needed.          */
/*--------------------------------------------------------------------*/

 static char *ExplodeAlias(char *header ,
                      const char *alias,
                      FILE *stream,
                      const boolean resent)
{
   char *fullname;
   char buffer[LSIZE];

   if ((alias == NULL) || (strlen(alias) == 0))
   {
      printmsg(0,"ExplodeAlias: NULL or empty string for argument");
      panic();
   }

   fullname = AliasByNick(alias);

   printmsg(4,"Processing alias '%s', result '%s'", alias,
      (fullname == NULL) ? alias : fullname);

   if (fullname == NULL)            /* No alias found for user?     */
   {                                /* No --> Try node lookup       */
      char user[MAXADDR];
      char node[MAXADDR];
      char path[MAXADDR];
      char bucket[MAXADDR];

      ExtractAddress(bucket, (char *) alias, FALSE);
      user_at_node(bucket, path, node, user);
      fullname = AliasByAddr( node, user);

      if (fullname == NULL)         /* Did we come up empty?         */
      {
         char *hisuser, *hisnode;

         hisuser = strtok( bucket, "@");
         hisnode = strtok( NULL, "@");
         if ((*bucket != '@') &&
             equal(hisuser, user ) && (hisnode != NULL) &&
             equal(hisnode, node ) && (strchr( node, '.') == NULL))
         {
            if (equal(hisnode, E_nodename))
               strcpy(node, E_fdomain);
            else {
               strcat(node,".");
               strcat(node,E_localdomain);
            }

            ExtractAddress(path, (char *) alias, TRUE);
            if (strlen( path ) == 0)
               sprintf(buffer,"%s@%s", hisuser, node );
            else
               sprintf(buffer,"\"%s\" <%s@%s>", path, hisuser, node);
            fullname = buffer;
         }
         else
            fullname = (char *) alias; /* Use original information     */
      }
   }
   else {
      ExtractAddress(buffer,fullname,TRUE);
      if (strlen(buffer) == 0)      /* A list of users?              */
      {                             /* Yes --> Do recursive call     */
         char *current = buffer;    /* Current token being processed */
         char *next = NULL;         /* Next token to process         */

         strcpy(buffer,fullname);

         do {
            current = strtok(current,",\t "); /* Get next alias to process */
            next    = strtok(NULL,"");    /* Also save rest of list    */
            header  = ExplodeAlias( header , current, stream, resent);
                                          /* Get alias, including sub-list */
            current  = next;
         } while ( next != NULL );        /* Until no more tokens exist */

         return header;                   /* Have written header, return  */

      } /* if */
   } /* else */

   if (strpbrk(fullname,"!@") == nil(char))
   {
      sprintf(buffer,"%s@%s", fullname , E_fdomain);
                              /* Local address                    */
      fullname = buffer;      /* Write out the formatted address  */
   }

   PutHead(header, fullname, stream, resent);
                              /* Remote address                   */
   return "";                 /* Make header empty string for
                                 next caller                      */
} /* ExplodeAlias */

/*--------------------------------------------------------------------*/
/*    A p p e n d _ S i g n a t u r e                                 */
/*                                                                    */
/*    Append the signature file to the specified mailbag file         */
/*                                                                    */
/*    [Broke this code out from Send_Mail to support the ~a mail      */
/*    subcommand]                                                     */
/*                                                                    */
/*    Returns:  0 on success, 1 if signature file not found           */
/*--------------------------------------------------------------------*/

static boolean Append_Signature(FILE *mailbag_fp ,
                     const boolean alternate)
{
   FILE *sigfp;
   char *sig;
   char sigfile[FILENAME_MAX];
   char buf[BUFSIZ];

   sig = alternate ? E_altsignature : E_signature;

   if(sig != nil(char)) {
      mkfilename(sigfile, E_homedir, sig);
      printmsg(4, "Append_Signature: signature file %s", sigfile);
      if ((sigfp = FOPEN(sigfile, "r",TEXT_MODE)) != nil(FILE)) {
         fputs("-- \n", mailbag_fp);
         while (fgets(buf, BUFSIZ, sigfp) != nil(char))
            fputs(buf, mailbag_fp);
         fclose(sigfp);
         return(0);
      }
      else {
         printmsg(0, "Signature file \"%s\" doesn't exist!\n", sigfile);
         return(1);
      }
   }
   return(0);
}  /* Append_Signature */

/*--------------------------------------------------------------------*/
/*    S e n d _ M a i l                                               */
/*                                                                    */
/*    Send text in a mailbag file to address(es) specified by line.   */
/*--------------------------------------------------------------------*/

boolean Send_Mail(FILE *datain,
               int argc,
               char *argv[],
               char *subject,
               const boolean resent)
{
   int argx = 0;
   char buf[LSIZE];
   char *header    = "To:";
   char *CcHeader  = "Cc:";
   char *BccHeader = "Bcc:";
   char *pipename  = mktempname(NULL, "TMP");
   FILE *stream = FOPEN(pipename , "w",TEXT_MODE);
   int status;

/*--------------------------------------------------------------------*/
/*                     Verify our workfile opened                     */
/*--------------------------------------------------------------------*/

   if ( stream == NULL )
   {
      printerr(pipename);
      free(pipename);
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*    Add the boilerplate the front:                                  */
/*                                                                    */
/*       Date, From, Organization, and Reply-To                       */
/*--------------------------------------------------------------------*/

   PutHead("Date:", arpadate() , stream, resent);

   if (bflag[F_BANG])
      sprintf(buf, "(%s) %s!%s", E_name, E_fdomain, E_mailbox );
   else
      sprintf(buf, "\"%s\" <%s@%s>", E_name, E_mailbox, E_fdomain );
   PutHead("From:", buf, stream , resent);

   if (E_organization != NULL )
      PutHead("Organization:", E_organization, stream, resent );

   if (E_replyto != NULL )
   {
      if (strpbrk(E_replyto,"!@") == nil(char))
         sprintf(buf,"\"%s\" <%s@%s>", E_name, E_replyto , E_fdomain);
      else
         sprintf(buf,"\"%s\" <%s>", E_name, E_replyto);
      PutHead("Reply-To:", buf, stream, resent );
   }

/*--------------------------------------------------------------------*/
/*                      Write the addressees out                      */
/*--------------------------------------------------------------------*/

   for (argx = 0 ; argx < argc; argx++ )
   {
      if (equal(argv[argx],"-c"))
      {
         header = CcHeader;
         CcHeader = "";
      } /* if */
      else if (equal(argv[argx],"-b"))
      {
         header = BccHeader;
         CcHeader = BccHeader = "";
      } /* if else */
      else
         header = ExplodeAlias( header , argv[argx], stream, resent);
   } /* for */

/*--------------------------------------------------------------------*/
/*                  Prompt for carbon copies, if any                  */
/*--------------------------------------------------------------------*/

   if ( bflag[F_ASKCC] && Is_Console(stdin) &&
        Console_fgets(buf,LSIZE,"Cc: "))
   {
      char *current = buf;
      header = CcHeader;
      CcHeader = "";
      printmsg(4,"CC buffer: %s",current);

      while ((current != NULL) &&
             (current = strtok(current,",\t\n ")) != NULL)
      {
         char *next  =  strtok(NULL,"");
         if (equal(current,"-b"))
         {
            header = BccHeader;
            CcHeader = BccHeader = "";
         } /* if */
         else
            header = ExplodeAlias( header, current, stream, resent);
         current = next;
      } /* while */
   }  /* If Console_fgets() */

/*--------------------------------------------------------------------*/
/*                     Handle the subject, if any                     */
/*--------------------------------------------------------------------*/

   if (subject != NULL)
      PutHead("Subject:",subject, stream, resent);
   PutHead(NULL, "", stream, resent);  /* Terminate the header file   */

/*--------------------------------------------------------------------*/
/*                    Copy the body of the message                    */
/*--------------------------------------------------------------------*/

   while (fgets(buf, LSIZE, datain) != nil(char))
   {
      int result = fputs(buf, stream );
      if (result == EOF)
      {
         printerr( pipename );
         panic();
      } /* if */
      if (buf[strlen(buf)-1] != '\n')
            fputc('\n', stream);
   } /* while */

   if (!feof(datain))
   {
      printerr("Send_Mail:");
      panic();
   } /* if */

   if (datain != stdin)
      fclose(datain);

/*--------------------------------------------------------------------*/
/*    Append user's primary signature file, if autosign option on     */
/*--------------------------------------------------------------------*/

   if ( bflag[F_AUTOSIGN] )
      Append_Signature(stream, FALSE);

   fclose(stream);

/*--------------------------------------------------------------------*/
/*                  Invoke the mail delivery program                  */
/*--------------------------------------------------------------------*/


   sprintf(buf, "-t -f %s", pipename);
   status = execute(RMAIL, buf, NULL, NULL, TRUE, FALSE );

   if ( status < 0 )
   {
      printerr( RMAIL );
      printmsg(0,"Unable to execute rmail; mail not delivered.");
   }
   else if ( status > 0 )
      printmsg(0,
         "rmail returned non-zero status; delivery may be incomplete.");

/*--------------------------------------------------------------------*/
/*               Log a copy of the mail for the sender                */
/*--------------------------------------------------------------------*/

   if (bflag[F_SAVERESENT] || ! resent)
      CopyOut(pipename);

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   remove(pipename);
   free(pipename);
   return (status == 0 );

} /*Send_Mail*/

/*--------------------------------------------------------------------*/
/*    C o p y O u t                                                   */
/*                                                                    */
/*    Save copy of outgoing mail, if desired                          */
/*--------------------------------------------------------------------*/

static void CopyOut( const char* input)
{
   FILE *datain;
   FILE *dataout;
   char buf[BUFSIZ];
   char outbox[FILENAME_MAX];

   if (E_filesent == NULL)
      return;

   strcpy( outbox, E_filesent);
   expand_path( outbox, E_homedir, E_homedir, E_mailext );

   datain = FOPEN( input, "r",TEXT_MODE);

   if (datain == NULL )
   {
      printerr(input);
      panic();
   } /* if */

   dataout = FOPEN( outbox, "a",TEXT_MODE);
   if (dataout == NULL )
   {
      printerr( outbox );
      panic();
   } /* if */

   fputs(MESSAGESEP,dataout);

   while (fgets(buf, BUFSIZ, datain) != nil(char))
   {
      int result = fputs(buf, dataout);
      if (result == EOF)
      {
         printerr( outbox );
         panic();
      } /* if */
   } /* while */

   if (!feof(datain))
   {
      printerr(input);
      panic();
   } /* if */

   fclose(datain);
   fclose(dataout);
} /* CopyOut */

/*--------------------------------------------------------------------*/
/* P u t H e a d                                                      */
/*                                                                    */
/* Write one line of an RFC-822 header                                */
/*--------------------------------------------------------------------*/

 static void PutHead( const char *label,
                      const char *operand,
                      FILE *stream,
                      const boolean resent)
 {
   static boolean terminate = TRUE;
                              /* If previous line was terminated     */

   if (label == NULL )        /* Terminate call?                     */
   {                          /* Yes --> Reset Flag and return       */
      fputc('\n', stream);    /* Terminate the current line          */
      if (!resent)
         fputc('\n', stream); /* Terminate the header file           */
      terminate = TRUE;
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
/*    C o l l e c t _ M a i l                                         */
/*                                                                    */
/*    Create mailbox file for delivery                                */
/*--------------------------------------------------------------------*/

boolean Collect_Mail(FILE *stream,
                  int argc,
                  char **argv,
                  const int current_msg,
                  const boolean reply)
{
   boolean editonly = FALSE;
   char Subuffer[LSIZE];
   char *subject = NULL;
   char *tmailbag;
   int  c;                    /* Really a 'char', but who cares?     */
   boolean done = FALSE;
   FILE  *fmailbag;

   if ( argc < (equal(argv[0], "-s" ) ? 3 : 1 ))
   {
      printmsg(0,"Cannot send mail, no addressees provided" );
      return FALSE;           /* Actually doesn't mean anything      */
   } /* if */

/*--------------------------------------------------------------------*/
/*      Determine if we are running interactively; if not, just       */
/*         pass the input stream to Send_Mail for processing          */
/*--------------------------------------------------------------------*/

   if (!Is_Console(stream))
   {
      if ( equal(argv[0], "-s" ) )
         return Send_Mail( stream, argc-2, argv+2, argv[1], FALSE);
      else
         return Send_Mail( stream , argc , argv, NULL , FALSE);
   } /* if */

/*--------------------------------------------------------------------*/
/*    We are running interactively; create a determine the name of    */
/*                        our work file to be.                        */
/*--------------------------------------------------------------------*/

   *Subuffer = '\0';          /* Assume no subject */
   tmailbag = mktempname( NULL, "TXT");

/*--------------------------------------------------------------------*/
/*         Determine if we should go straight into the editor         */
/*--------------------------------------------------------------------*/

   editonly = bflag[F_AUTOEDIT] && (E_editor != NULL);

   if ( equal(argv[0],"-s"))     /* Any subject specified?           */
   {
      strcpy( Subuffer , argv[1] ); /* Save the subject for later    */
      argv += 2;                 /* Skip over it in argument list    */
      argc -= 2;                 /* Giving us fewer args left        */
   }
   else {                        /* No --> Prompt for one            */
      if(Console_fgets(Subuffer,LSIZE,"Subject: "))
      {
         if (Subuffer[strlen(Subuffer) - 1 ] == '\n')
            Subuffer[strlen(Subuffer)-1] = '\0';   /* End the subject */
      }  /* If Console_fgets() */
   } /* if ( equal(argv[0],"-s")) */

/*--------------------------------------------------------------------*/
/*      Copy a message from the original input to temporary file      */
/*--------------------------------------------------------------------*/

   fmailbag = FOPEN(tmailbag, "w",TEXT_MODE);
   if (fmailbag == NULL )
   {
      printerr( tmailbag );
      panic();
   }

/*--------------------------------------------------------------------*/
/*               Include incoming message if requested                */
/*--------------------------------------------------------------------*/

   if ( bflag[F_AUTOINCLUDE] && reply)
   {
       CopyMsg(current_msg, fmailbag, fromheader , TRUE);
       fprintf(stdout, "Message %d Included\n", current_msg+1);
   } /* if ( bflag[F_AUTOINCLUDE] && reply) */

/*--------------------------------------------------------------------*/
/*                     Edit the file if requested                     */
/*--------------------------------------------------------------------*/

   if (editonly)              /* Enter editor immediately?     ahd   */
   {                          /* Yes --> Go to it                    */
      fclose(fmailbag);
      Invoke(E_editor, tmailbag, bflag[F_NEWEDITORSESSION] );
   } /* if */
   else {                     /* No  --> prompt for data       ahd   */
      Prompt_Input( tmailbag , fmailbag , Subuffer, current_msg );
      fclose(fmailbag);
   } /*else */


   do {
      fputs("\nAbort, Continue, Edit, List, or Send? ",stdout);
      c     = Get_One();             /* adaptation for QuickC */  /* pdm */
      switch (tolower( c ))
      {
         case 'c':
            puts("Continue");
            fmailbag = FOPEN(tmailbag, "a",TEXT_MODE);
            Prompt_Input( tmailbag , fmailbag , Subuffer, current_msg );

#if defined(_Windows)

/*--------------------------------------------------------------------*/
/*            Prompt_Input() comes back with EOF on stdin!            */
/*--------------------------------------------------------------------*/

            rewind(stdin);
#endif
            fclose(fmailbag);
            break;

         case 'l':
            puts("List");
            Sub_Pager(tmailbag, islower(c) );
            break;

         case 's':
            puts("Send");
            fmailbag = FOPEN(tmailbag, "r",TEXT_MODE);
            if (fmailbag == NULL )
            {
               printerr(tmailbag);
               panic();
            }
            if ( strlen( Subuffer ))
               subject = Subuffer;
            Send_Mail(fmailbag, argc, argv, subject, FALSE);
            done = TRUE;
            break;

         case 'e':
            puts("Edit");
            Invoke(E_editor, tmailbag, bflag[F_NEWEDITORSESSION] );
            break;

         case 'a':
            fputs("Abort\nAre you sure? ", stdout);
            safeflush();
            c = Get_One();             /* for QuickC */          /* pdm */
            switch (tolower(c)) {      /* unravel these two calls */
            case 'y':
               puts("Yes");
               done = TRUE;
               break;
            default:
               puts("No");
            } /*switch*/
            break;

         default:
            puts("\n\aEnter A, C, E, L, or S.");
            safeflush();
            done = FALSE;
      } /*switch*/
   } while (!done);

   remove(tmailbag);
   free(tmailbag);

   return TRUE;
} /*Collect_Mail*/


/*--------------------------------------------------------------------*/
/*    P r o m p t _ I n p u t                                         */
/*                                                                    */
/*    Prompt for mail entry interactively.                            */
/*--------------------------------------------------------------------*/

static void Prompt_Input( char *tmailbag,
            FILE *fmailbag,
            char *subject,
            const int current_msg)
{
   char buf[LSIZE];

   printf("\nEnter message.  Enter ~? for help.  End input with %s\n",
          bflag[ F_DOT ] ?  "a period (.)" :
         "end-of-file (Control-Z)");
   for ( ; ; )
   {
      if (Console_fgets(buf, LSIZE, "? "))
      {
         if (bflag[F_DOT] && equal(buf,".\n"))
            break;
         else if (Subcommand( buf, fmailbag, tmailbag, subject, current_msg) )
            continue;      /*Don't write line out if subcommand   */
      } /* if */
      else
         break;            /* Exit loop if end of file            */

      if (fputs(buf, fmailbag) == EOF )
      {
         printerr( tmailbag );
         panic();
      } /* if (fputs(buf, fmailbag) == EOF ) */

      if (buf[strlen(buf)-1] != '\n')
         fputc('\n', fmailbag);
   } /* for */
} /* Prompt_Input */

/*--------------------------------------------------------------------*/
/*    S u b c o m m a n d                                             */
/*                                                                    */
/*    Handle tilde (~) subcommands for Interactive mail               */
/*--------------------------------------------------------------------*/

static boolean Subcommand( char *buf,
                           FILE *fmailbag,
                           char *tmailbag,
                           char *subject,
                           const int current_msg)
{
   int message;
   char fname[FILENAME_MAX];
   char *token;
   FILE *stream;

   if(*buf == '~')        /* Handle mail subcommands  pdm */
   {
      switch(buf[1])
      {

/*--------------------------------------------------------------------*/
/*                     Treat as normal data line                      */
/*--------------------------------------------------------------------*/

         case '~':
            memmove( buf, buf + 1, strlen( buf + 1 ));
            return FALSE;        /* Treat as normal line              */

/*--------------------------------------------------------------------*/
/*              Put signature file into current message               */
/*--------------------------------------------------------------------*/

         case 'a':
         case 'A':
            Append_Signature(fmailbag, isupper( buf[1] ));
            fputs("(continue)\n", stdout);
            break;

/*--------------------------------------------------------------------*/
/*                       Edit outgoing message                        */
/*--------------------------------------------------------------------*/

         case 'v':            /* UNIX allows 'v'isual editor         */
         case 'e':
            /* invoke editor with current msg */
            fclose(fmailbag);
            Invoke(E_editor, tmailbag, bflag[F_NEWPAGERSESSION] );
            fmailbag = FOPEN(tmailbag, "a",TEXT_MODE);
            fputs("(continue)\n", stdout);
            break;

/*--------------------------------------------------------------------*/
/*                 Include any letter in this message                 */
/*--------------------------------------------------------------------*/

         case 'f':
         case 'F':
         case 'i':
         case 'I':
         case 'm':
         case 'M':
            if (fmailbox == NULL)
               puts("Mailbox not accessible!");
            else {
               int *item_list;
               int next_item = PushItemList( &item_list );
               boolean first_pass = TRUE;

               token = GetString( &buf[2] );

               if (SelectItems( &token, current_msg, LETTER_OP ))
               while( Get_Operand( &message, &token, LETTER_OP, first_pass))
               {
                  CopyMsg( message , fmailbag,
                           islower(buf[1]) ? fromheader : noseperator ,
                           tolower(buf[1]) != 'f');
                  fprintf(stdout, "Message %d included\n", message + 1);
                  first_pass = FALSE;
               } /* while */

               PopItemList( item_list, next_item );
            } /* else */
            break;

/*--------------------------------------------------------------------*/
/*                       Print current message                        */
/*--------------------------------------------------------------------*/

         case 'p':
         case 'P':
            fclose(fmailbag);
            Sub_Pager(tmailbag, islower(buf[1]) );
            fmailbag = FOPEN(tmailbag, "a",TEXT_MODE);
            fputs("(continue)\n", stdout);
            break;

/*--------------------------------------------------------------------*/
/*                           Include a file                           */
/*--------------------------------------------------------------------*/

         case 'r':
            token = strtok( &buf[2], " \t\n");
            if ( token == NULL )
            {
               printf("Need a file name for this command!\n");
               break;
            }
            strcpy( fname, token );
            if ( expand_path( fname, NULL, E_homedir , NULL) == NULL )
               break;
            stream = FOPEN( fname, "r",TEXT_MODE);
            if (stream == NULL )
            {
               printerr(fname);
               break;
            }
            else while( fgets( buf, LSIZE, stream ))
            {
               fputs( buf, fmailbag);
               if ferror( fmailbag )
               {
                  printerr( tmailbag);
                  break;
               } /* if */
            } /* else while */
            if (ferror( stream ) )
            {
               printerr( fname );
               clearerr( stream );
            } /* if */
            fclose( stream );
            fputs("(continue)\n", stdout);
            break;


/*--------------------------------------------------------------------*/
/*                        Change mail subject                         */
/*--------------------------------------------------------------------*/

         case 's':
            token = GetString( &buf[2] );
            if ( token != NULL )
            {
               strcpy( subject, token );
            }
            else
               printf("No new subject, command ignored\n");
            printf("Subject: %s\n",subject);
            break;

/*--------------------------------------------------------------------*/
/*                                Help                                */
/*--------------------------------------------------------------------*/

         case '?':
         {
            mkfilename(fname, E_confdir, "tilde.hlp");
            Sub_Pager( fname, TRUE );
            break;
         }

/*--------------------------------------------------------------------*/
/*                             A subshell                             */
/*--------------------------------------------------------------------*/

         case '!':
            token = strtok( &buf[2], "\n");
            subshell( token );
            break;

/*--------------------------------------------------------------------*/
/*                     Pipe mail through a filter                     */
/*--------------------------------------------------------------------*/

         case '|':
            fclose( fmailbag );
            filter( tmailbag, &buf[2] );
            fmailbag = FOPEN(tmailbag, "a",TEXT_MODE);
            fputs("(continue)\n", stdout);
            break;

/*--------------------------------------------------------------------*/
/*                          Invalid command                           */
/*--------------------------------------------------------------------*/

         default:
            fputs("Unknown mail subcommand, ~? for help.\n",
                  stdout);
            break;
      } /* switch */

      return TRUE;
   } /* if */
   else
      return FALSE;           /* It wasn't a sub-command             */

} /*SubCommand*/

/*--------------------------------------------------------------------*/
/*    f i l t e r                                                     */
/*                                                                    */
/*    Filter the next of an outgoing program into the output mail     */
/*--------------------------------------------------------------------*/

static void filter( char *tmailbag, char *command)
{

   char pipename[FILENAME_MAX];
   struct stat statbuf;
   int    result = 0;

   command = GetString( command );

   if ( command == NULL )
   {
      printf("No command given for filter");
      return;
   }

/*--------------------------------------------------------------------*/
/*   Set up our standard input and standard output for the command    */
/*--------------------------------------------------------------------*/

   mktempname(pipename, "TMP");

/*--------------------------------------------------------------------*/
/*                          Run the command                           */
/*--------------------------------------------------------------------*/

   result = executeCommand( command, tmailbag, pipename, TRUE, TRUE );

   if (result == -1)       /* Did spawn fail?            */
         ;                 /* No operation               */
   else if( stat( pipename, &statbuf) <0 )   /* Create output?    */
   {
      printf(0,"Cannot determine status of output %s",pipename);
      printerr( pipename );
   }
   else if( statbuf.st_size == 0 )  /* Anything in the file?      */
      printf("Output file %s is empty!\n", pipename);
   else {                  /* Good output, replace input file     */
      remove( tmailbag );
      if (rename( pipename, tmailbag ))
         printerr( pipename );
   } /* else */

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   remove( pipename );

} /* filter */

/*--------------------------------------------------------------------*/
/*    G e t S t r i n g                                               */
/*                                                                    */
/*    Get non-whitespace in a string                                  */
/*--------------------------------------------------------------------*/

static char *GetString( char *input)
{
   char *end;

/*--------------------------------------------------------------------*/
/*                   Look for first data in string                    */
/*--------------------------------------------------------------------*/

   while( *input && !isgraph( *input ))
      input++ ;

/*--------------------------------------------------------------------*/
/*   If no input or all blanks, return NULL to denote empty string    */
/*--------------------------------------------------------------------*/

   if (*input == '\0')
      return NULL ;

/*--------------------------------------------------------------------*/
/*                Delete whitespace from end of string                */
/*--------------------------------------------------------------------*/

   end = input + strlen( input ) - 1;

   while (!isgraph(*end))
      end--;

   end[1] = '\0';

/*--------------------------------------------------------------------*/
/*                 Return beginning of string to caller               */
/*--------------------------------------------------------------------*/

   return input;

} /* GetString */
