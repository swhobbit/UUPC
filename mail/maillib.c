/*--------------------------------------------------------------------*/
/*    m a i l l i b . c                                               */
/*                                                                    */
/*    Mail user agent subroutine library for UUPC/extended            */
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
 *    $Id: maillib.c 1.5 1993/07/31 16:26:01 ahd Exp $
 *
 *    $Log: maillib.c $
 * Revision 1.5  1993/07/31  16:26:01  ahd
 * Changes in support of Robert Denny's Windows support
 *
 * Revision 1.4  1993/06/13  14:06:00  ahd
 * Add precedence to the standard ignore list
 *
 * Revision 1.3  1993/04/11  00:33:05  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/27  14:36:10  ahd
 * Use scrsize() for screen size
 *
 *       3 May 90 Create from mail.c
 *       16 Jun 90:  Added support for mail (~) subcommands      pdm
 *                   chgd calling seq of Collect_Mail to support
 *                         above
 *                   chges to CopyMsg to support ~i subcmd
 *                   mods to SendMail to support autosign option
 *                   broke out signature append code to seperate fn
 *                   added support for alternate signature file
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "address.h"
#include "hlib.h"
#include "mlib.h"
#include "alias.h"
#include "mail.h"
#include "maillib.h"
#include "scrsize.h"

#define  INDENT "> "

/*--------------------------------------------------------------------*/
/*       Local variables                                              */
/*--------------------------------------------------------------------*/

static int PageCount = 0;

static char *ignorelist[] =  { "Message-ID:",
                        "Received:",
                        "Status: ",
                        "X-Mailer: ",
                        "From " ,
                        "Precedence: " ,
                        "Path: ",
                        "Lines: ",
                        "References: ",
                        "" };

currentfile();                /* Define current file for panic()     */

/*--------------------------------------------------------------------*/
/*    P a g e r                                                       */
/*                                                                    */
/*    Page through a message                                          */
/*                                                                    */
/*    There are hooks here to let the user use his/her own pager,     */
/*    like LIST, MORE, or LESS.  We just write the message out to     */
/*    a temporary file and invoke the appropriate external program    */
/*    to do the browsing.                                             */
/*--------------------------------------------------------------------*/

boolean Pager(const int msgnum,
              boolean external,
              copyopt received,
              const boolean reset)
{
   long nextloc;
   char *browse   = NULL;
   char buf[BUFSIZ];
   boolean exit  = FALSE;        /* Flag for PRE-MATURE exit   ahd   */
   FILE *fmailbag;

   if (msgnum == -1)
      return FALSE;

   if (bflag[F_PAGER])           /* User want pager option inverted? */
      external = ! external;     /* Yes --> Do the inversion         */

   if (letters[msgnum].status < M_READ)
      letters[msgnum].status = M_READ;

   if (external && (E_pager != nil(char)))
   {
      browse = mktempname( NULL,"TMP" );/* Get a temporary file name */

      if ((fmailbag = FOPEN(browse, "w",TEXT_MODE)) == nil(FILE))
      {
         printerr(browse);
         printmsg(0,"Cannot open browse file %s",browse);
         return FALSE;
      } /* if */
      CopyMsg(msgnum, fmailbag, received, FALSE);
      fclose(fmailbag);

      Invoke(E_pager, browse);
      remove(browse);
      free(browse);

   } /* if */
   else {
      fseek(fmailbox, letters[msgnum].adr , SEEK_SET);
      nextloc = letters[msgnum + 1].adr;

      if ( reset )
         ClearScreen();
      else
         PageLine("\n");

      sprintf(buf,"Mailbox item %d:\n",msgnum + 1);
      PageLine(buf);
      while (ftell(fmailbox) < nextloc && (!exit) &&
         fgets(buf, BUFSIZ, fmailbox) != nil(char))
      {
         boolean print = TRUE;

         switch(received)
         {
            case nocontinue:
               if ((*buf != '\n') && !isgraph(*buf)) {
                  print = FALSE;
                  break;
               }
               else
                  received = noreceived;
            case noreceived:
            {
               char entry = 0;
               while ( strlen(ignorelist[entry]) && print )
               {
                  if (equalni(ignorelist[entry],
                        buf,strlen(ignorelist[entry])))
                  {
                     print = FALSE;
                     received = nocontinue;
                  }
                  else
                     entry++;
               } /* while */
            } /* case noreceived */
         } /* switch */
         if (received != seperators)
            if (equal(buf,"\n"))
               received = seperators;

         if (print)
            if (PageLine(buf))         /* Exit if the user hits Q    */
               exit = TRUE;
      } /* while */

      if (equal(buf,"\n") && (!exit))                 /* ahd   */
         putchar('\n');                               /* ahd   */
   } /* else */

   return ! exit;
} /*Pager*/


/*--------------------------------------------------------------------*/
/*    S u b _ P a g e r                                               */
/*       pager for the ~p mail subcommand                             */
/*       page through a mail message currently being entered          */
/*                                                                    */
/*    Clone of the Pager function                                     */
/*--------------------------------------------------------------------*/

void Sub_Pager(const char *tinput,
                     boolean external )
{
   boolean exit  = FALSE;        /* Flag for PRE-MATURE exit   ahd   */

   if (bflag[ F_PAGER ])
      external = ! external;

   if ( external && (E_pager != nil(char)) )
      Invoke(E_pager, tinput);
   else {
      FILE *finput;
      char buf[BUFSIZ];
      finput = FOPEN(tinput, "r",TEXT_MODE);
      if (finput == NULL) {
         printmsg(0,"Cannot open file %s for display",tinput);
         printerr(tinput);
         return;
      }
      PageReset();
      ClearScreen();
      while ( (!exit) && fgets(buf, BUFSIZ, finput) != nil(char))
      {
        if (PageLine(buf))         /* Exit if the user hits Q  */
           exit = TRUE;
      }
      fclose(finput);
   }

} /*Sub_Pager*/

/*--------------------------------------------------------------------*/
/*    P a g e R e s e t                                               */
/*                                                                    */
/*    Reset page function to top of page                              */
/*--------------------------------------------------------------------*/

void PageReset()
{
   PageCount = 0;
} /*PageReset*/

/*--------------------------------------------------------------------*/
/*    P a g e L i n e                                                 */
/*                                                                    */
/*    Print one line when paging through a file                       */
/*--------------------------------------------------------------------*/

boolean PageLine(char *line)
{

#ifdef _Windows
   short pagesize = scrsize() - 3;
#else
   short pagesize = scrsize() - 3;
#endif

   fputs(line, stdout);

   PageCount = PageCount + 1 + strlen(line) / 81; /* Handle long lines  */

   if (PageCount > (pagesize))
   {
      int c;
      fputs("More?", stdout);
      c = Get_One();

      switch (tolower(c))
      {
         case 'q':
         case '\003':
         case 'n':                        /* Because that's what I
                                             keep Pressing           */
         case 'x':
            fputs("\rAborted.\n", stdout);
            return TRUE;

         case 'd':
            PageCount = pagesize / 2;     /* Half a Page More */
            break;

         case '\r':
            PageCount = pagesize;         /* Only print one line  */
            break;

         default:
            PageCount = 0;                /* Print full screen    */
      }
      fputs("\r      \r",stdout);
   }

   return FALSE;

} /*PageLine*/

/*--------------------------------------------------------------------*/
/*    C o p y M s g                                                   */
/*                                                                    */
/*    Copy a message                                                  */
/*                                                                    */
/*    Allows copying message with one or more of the options          */
/*    specified in the copyopt data type.                             */
/*--------------------------------------------------------------------*/

boolean CopyMsg(int msgnum, FILE *f, copyopt headers, boolean indent)
{
   long nextloc;
   boolean print;
   char buf[BUFSIZ];

/*--------------------------------------------------------------------*/
/*                 Write a separator line, if needed                  */
/*--------------------------------------------------------------------*/

   if (headers == seperators)
   {
      if (fputs(MESSAGESEP,f) == EOF)     /* Write out separator line   */
      {
         printerr("CopyMsg");
         panic();
      } /* if (fputs(MESSAGESEP,f) == EOF) */
   } /* if (headers == seperators) */

/*--------------------------------------------------------------------*/
/*             else add a one line from line, if desired              */
/*--------------------------------------------------------------------*/

   else if (headers == fromheader )
   {
      register char *sp = buf;
      headers = noheader;                 /* Do not print full header       */
      if (RetrieveLine(letters[msgnum].date, buf, LSIZE))
      {
         register char  *sp = buf;
         while (!isspace(*sp))
            sp++;
         while (isspace(*sp))
            sp++;
         fprintf(f,"On %s,", sp );
      } /* if */

      if (RetrieveLine(letters[msgnum].from, buf, BUFSIZ))
      {
         while (!isspace(*sp) && (*sp != '\0'))
            sp++;
         BuildAddress( buf, sp );
      } /* if */
      else
         strcpy(buf,"you");   /* Wimp out without admitting it       */

      fprintf(f, " %s wrote:\n", buf) ;
   } /* if (headers == fromheader ) */

/*--------------------------------------------------------------------*/
/*              Now position to the front of the letter               */
/*--------------------------------------------------------------------*/

   fseek(fmailbox, letters[msgnum].adr , SEEK_SET);
   nextloc = letters[msgnum + 1].adr;

   while (ftell(fmailbox) < nextloc &&
      fgets(buf, BUFSIZ, fmailbox) != nil(char)) {

/*--------------------------------------------------------------------*/
/*               Determine if we should write the line                */
/*--------------------------------------------------------------------*/

      print = TRUE;

      switch (headers)
      {
         case noheader:
            print = FALSE;
            break;

         case nocontinue:
            if ((*buf != '\n') && !isgraph(*buf)) {
               print = FALSE;
               break;
            }
            else
               headers = noreceived;
               /* Fall through ... */
         case noreceived:
         {
            char entry = 0;
            while ( strlen(ignorelist[entry]) && print )
            {
               if (equalni(ignorelist[entry],buf,strlen(ignorelist[entry])))
               {
                  print = FALSE;
                  headers = nocontinue;
               }
               else
                  entry++;
            }
         } /* case noreceived */
               /* Fall through */
         case noseperator:
         case seperators:
            break;

         default:
            printmsg(0,"CopyMsg: Bad header copy state of %d",headers);
            panic();
      } /* switch */

/*--------------------------------------------------------------------*/
/*                 If we should print the line, do so                 */
/*--------------------------------------------------------------------*/

      if (print)
      {
         if (indent)
         {
            if ( fputs(INDENT , f ) == EOF )
            {
               printerr( "CopyMsg" );
               panic();
            } /* if ( fputs(INDENT , f ) == EOF ) */
         } /* if (indent) */

         if ( fputs(buf , f ) == EOF )
         {
            printerr( "CopyMsg" );
            panic();
         } /* if ( fputs(buf , f ) == EOF ) */

      } /* if (print) */

/*--------------------------------------------------------------------*/
/*  If end of the header, print all data until the end of the input   */
/*--------------------------------------------------------------------*/

      if ( (headers != seperators) && equal(buf, "\n") )
         headers = seperators;

   } /*while*/

   return TRUE;
} /*CopyMsg*/

/*--------------------------------------------------------------------*/
/*    N u m e r i c                                                   */
/*                                                                    */
/*    Determine if a string is numeric.  Returns TRUE if string is    */
/*    numeric, else FALSE.                                            */
/*--------------------------------------------------------------------*/

 boolean Numeric( const char *number)
 {
   char *column = (char *) number;

   if (*column == '\0')
      return FALSE;

   while( isdigit(*column) )  /* Scan to string end or 1st non-digit */
      column++;

   return *column == '\0';    /* Success if whole string was made of
                                 digits                              */
 } /* Numeric */

/*--------------------------------------------------------------------*/
/*    R e t r i e v e L i n e                                         */
/*                                                                    */
/*    Read a line from a mail header, if available                    */
/*--------------------------------------------------------------------*/

boolean RetrieveLine(long adr, char *line, const size_t len)
{
   char *cp = line;
   size_t count;

   *line = '\0';              /* Insure nothing to find              */
   if (adr == MISSING)        /* No information to read?             */
      return FALSE;           /* Report this to caller               */

   if (fseek(fmailbox, adr, SEEK_SET)) /* Position to data           */
   {                          /* Have a problem?                     */
      printerr("mailbox");    /* Yes --> Report and return           */
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                     Actually read the data in                      */
/*--------------------------------------------------------------------*/

   count = fread(line, sizeof *line, len-1, fmailbox);

   if ((count < (len-1)) && ferror( fmailbox ))
   {
      printerr( "RetrieveLine");
      return FALSE;
   }

   line[count] = '\0';        /* Terminate the string read           */

/*--------------------------------------------------------------------*/
/*    A field continues until a new field begins in column of the     */
/*    next line or the header ends (an empty line); find the end      */
/*    of the field, trimming extra white space from the beginning     */
/*    of each line as we go                                           */
/*--------------------------------------------------------------------*/

   while( (cp = strchr(cp , '\n')) != NULL )
   {
      if ((cp[1] == '\n') || !isspace(cp[1]))   /* End of field?     */
         *cp = '\0';          /* Yes --> Terminate string            */
      else {
         char *next;

         *cp++ = ' ';         /* Convert line break to whitespace    */
         next = ++cp;         /* Get first position of new line      */
         while( isspace( *next ) )  /* Ignore leading white space    */
            next++;

         memmove( cp , next , strlen(next) + 1 );
                              /* Trim leading white space            */
      } /* else */
   } /* while */

   return TRUE;

} /*RetrieveLine*/

/*--------------------------------------------------------------------*/
/*    R e t u r n A d d r e s s                                       */
/*                                                                    */
/*    Returns the user name (if available and requested) or           */
/*    E-mail address of the user                                      */
/*                                                                    */
/*    Written by ahd 15 July 1989                                     */
/*--------------------------------------------------------------------*/

void ReturnAddress(char *line, struct ldesc *ld)
{
   char buffer[BUFSIZ];

   if (!RetrieveLine(ld->from, buffer, BUFSIZ))
                                          /* From: line available?   */
      strcpy(line,"-- Unknown --");       /* No --> Return error     */
   else {
      char *begin = buffer;
      while (!isspace(*begin) && (*begin != '\0'))
         begin++;
      if (strlen(begin))
         ExtractName(line,begin);         /* Yes --> Return name     */
      else
         strcpy(line,"-- Invalid From: line --");
   }

   return;

} /*ReturnAddress*/

/*--------------------------------------------------------------------*/
/*    s a y o p t i o n s                                             */
/*                                                                    */
/*    Announce user options in effect                                 */
/*--------------------------------------------------------------------*/

void sayoptions( FLAGTABLE *flags)
{

   size_t subscript;
   size_t used = 0;

   printf("\nThe following options are set:\n");

   for (subscript = 0; flags[subscript].sym != NULL; subscript++)
   {
         size_t width;

         if (flags[subscript].bits & B_GLOBAL)
            continue;               /* Don't print system options */

         width = 1 + strlen( flags[subscript].sym ) +
                 ( bflag[ flags[subscript].position ] ? 0 : 2 );

         used += width;
         if ( subscript > 0 )
         {
            if ( used > 79 )
            {
               putchar('\n');
               used = width;
            } /* if ( used > 79 ) */
            else
               putchar(' ');
         } /* if ( subscript > 0 ) */

         printf("%s%s",
               bflag[ flags[subscript].position ] ? "" : "no" ,
               flags[subscript].sym );

   } /* for */

   putchar('\n');

} /* sayoptions */
