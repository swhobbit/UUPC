/*--------------------------------------------------------------------*/
/*    m a i l l i b . c                                               */
/*                                                                    */
/*    Mail user agent subroutine library for UUPC/extended            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: maillib.c 1.24 1996/01/01 23:50:26 ahd Exp $
 *
 *    $Log: maillib.c $
 *    Revision 1.24  1996/01/01 23:50:26  ahd
 *    Don't scan nickname table for duplicate nicknames in linear fashion,
 *    merely check entire table for duplicates after sorting.
 *    Rename user functions previously known as 'user alias' to 'nickname',
 *    consistent with newer documentation.
 *
 *    Revision 1.23  1996/01/01 21:03:50  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.22  1995/03/11 22:28:10  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.21  1995/02/20 17:28:43  ahd
 *    in-memory file support, 16 bit compiler clean up
 *
 *    Revision 1.20  1995/01/07 16:19:01  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.19  1994/12/22 00:19:27  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1994/08/07 21:28:54  ahd
 *    Clean up OS/2 processing to not use new sessions, but rather simply user
 *    command processor to allow firing off PM programs such as E and EPM.
 *
 * Revision 1.17  1994/06/05  01:57:26  ahd
 * Don't use exit as a variable name, scope errors are not trapped
 * Make sure exitNow properly set for all returns in Pager()
 *
 * Revision 1.16  1994/03/09  01:55:39  ahd
 * Error check the read which overshoots the buffer with memory files
 *
 * Revision 1.15  1994/03/07  06:09:51  ahd
 * Additional debugging messages controlled by UDEBUG
 * Shorten ReturnAddress line buffer to correct problem with length
 *
 * Revision 1.14  1994/02/28  01:02:06  ahd
 * Cosmetic formatting cleanups
 *
 * Revision 1.13  1994/02/25  03:17:43  ahd
 * Allow configurable ignore and reply to search lists
 * Insert X-previous- in front of aged resent- headers when doing
 * new resends.
 *
 * Revision 1.12  1994/02/19  04:18:12  ahd
 * Use standard first header
 *
 * Revision 1.12  1994/02/19  04:18:12  ahd
 * Use standard first header
 *
 * Revision 1.11  1994/01/01  19:12:47  ahd
 * Annual Copyright Update
 *
 * Revision 1.10  1993/12/23  03:16:03  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.9  1993/11/13  17:43:26  ahd
 * Noramalize external command processing
 *
 * Revision 1.8  1993/10/12  01:30:23  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.7  1993/10/09  20:16:12  rhg
 * ANSIfy the source
 *
 * Revision 1.6  1993/09/20  04:41:54  ahd
 * OS/2 2.x support
 *
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

#include "uupcmoah.h"

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "address.h"
#include "mlib.h"
#include "nickname.h"
#include "mail.h"
#include "maillib.h"
#include "scrsize.h"

#define  INDENT "> "

/*--------------------------------------------------------------------*/
/*       Local variables                                              */
/*--------------------------------------------------------------------*/

static int PageCount = 0;


static char *noResentList[] =
       {
         "Message-ID:",
         "Received:",
         "From ",
         "Precedence: ",
         "Path: ",
         "References: ",
         NULL
       };

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

KWBoolean Pager(const int msgnum,
              KWBoolean external,
              copyopt received,
              const KWBoolean reset)
{
   KWBoolean normalExit = KWTrue;

   if (msgnum == -1)
      return KWFalse;

   if (bflag[F_PAGER])           /* User want pager option inverted? */
      external = (KWBoolean) ((! external) ? KWTrue : KWFalse );
                                 /* Yes --> Do the inversion         */

   if (letters[msgnum].status < M_READ)
      letters[msgnum].status = M_READ;

   if (external && (E_pager != nil(char)))
   {
      char browse[FILENAME_MAX];
      FILE *fmailbag;

      mktempname( browse,"tmp" );      /* Get a temporary file name */

      if ((fmailbag = FOPEN(browse, "w",TEXT_MODE)) == nil(FILE))
      {
         printerr(browse);
         printmsg(0,"Cannot open browse file %s",browse);
         return KWFalse;
      } /* if */

      CopyMsg(msgnum, fmailbag, received, KWFalse);
      fclose(fmailbag);

      Invoke(E_pager, browse );
      REMOVE(browse);

   } /* if (external && (E_pager != nil(char))) */
   else {

      char buf[BUFSIZ];
      long nextloc = letters[msgnum + 1].adr;

      imseek(imBox, letters[msgnum].adr, SEEK_SET);

      if ( reset )
         ClearScreen();
      else
         PageLine("\n");

      sprintf(buf,"Mailbox item %d:\n",msgnum + 1);
      PageLine(buf);

      while ((imtell(imBox) < nextloc) &&
             ( normalExit ) &&
             (imgets(buf, sizeof buf, imBox ) != nil(char)))
      {
         KWBoolean print = KWTrue;

         switch(received)
         {
            case nocontinue:
               if ((*buf != '\n') && !isgraph(*buf))
               {
                  print = KWFalse;
                  break;
               }
               else
                  received = ignoresome;
               /* Fall through */

            case ignoresome:
            {
               char entry = 0;

               while ( E_ignoreList[entry] && print )
               {
                  if (equalni(E_ignoreList[entry],
                        buf,
                        strlen(E_ignoreList[entry])))
                  {
                     print = KWFalse;
                     received = nocontinue;
                  }
                  else
                     entry++;

               } /* while */

            } /* case ignoresome */

         } /* switch */

         if ((received != seperators) && equal(buf,"\n"))
               received = seperators;

         if (print && PageLine(buf))   /* Exit if the user hits Q    */
            normalExit = KWFalse;

      } /* while */

      if (equal(buf,"\n") && ( normalExit ))
         fputc('\n', stdout);

   } /* else */

   return normalExit;

} /*Pager*/

/*--------------------------------------------------------------------*/
/*    S u b _ P a g e r                                               */
/*       pager for the ~p mail subcommand                             */
/*       page through a mail message currently being entered          */
/*                                                                    */
/*    Clone of the Pager function                                     */
/*--------------------------------------------------------------------*/

void Sub_Pager(const char *tinput,
                     KWBoolean external )
{
   KWBoolean exitNow  = KWFalse;   /* Flag for PRE-MATURE exit   ahd   */

   if (bflag[ F_PAGER ])
      external = (KWBoolean) ((! external) ? KWTrue : KWFalse);

   if ( external && (E_pager != nil(char)) )
      Invoke(E_pager, tinput );
   else {
      FILE *finput;
      char buf[BUFSIZ];

      finput = FOPEN(tinput, "r",TEXT_MODE);
      if (finput == NULL)
      {
         printmsg(0,"Cannot open file %s for display",tinput);
         printerr(tinput);
         return;
      }

      PageReset();
      ClearScreen();

      while ( (!exitNow) && fgets(buf, sizeof buf, finput) != nil(char))
      {
        if (PageLine(buf))         /* Exit if the user hits Q  */
           exitNow = KWTrue;
      }

      fclose(finput);

   } /* else */

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

KWBoolean PageLine(char *line)
{

   int pagesize = scrsize() - 3;

   fputs(line, stdout);

   PageCount = PageCount + 1 + (int) strlen(line) / 81;
                                    /* Handle long lines             */

   if (PageCount > pagesize)
   {
      int c;
      fputs("More?", stdout);
      fflush(stdout);
      c = Get_One();

      switch (tolower(c))
      {
         case 'q':
         case '\003':
         case 'n':                        /* Because that's what I
                                             keep Pressing           */
         case 'x':
            fputs("\rAborted.\n", stdout);
            return KWTrue;

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

   return KWFalse;

} /* PageLine */

/*--------------------------------------------------------------------*/
/*    C o p y M s g                                                   */
/*                                                                    */
/*    Copy a message                                                  */
/*                                                                    */
/*    Allows copying message with one or more of the options          */
/*    specified in the copyopt data type.                             */
/*--------------------------------------------------------------------*/

KWBoolean CopyMsg(const int msgnum,
                FILE *f,
                const copyopt headerFlag,
                const KWBoolean indent)
{
   long nextloc;
   KWBoolean print;                  /* Header line should be printed */
   KWBoolean printX;                 /* Header line gets X- prefix    */
   char buf[BUFSIZ];
   copyopt headers = headerFlag;
   char **ignoreList = (headerFlag == ignoresome) ?
                        E_ignoreList : noResentList;

/*--------------------------------------------------------------------*/
/*                 Write a separator line, if needed                  */
/*--------------------------------------------------------------------*/

   if (headers == seperators)
   {
      if (fputs(MESSAGESEP,f) == EOF)     /* Write out separator line  */
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
      headers = noheader;                 /* Do not print full header */
      if (RetrieveLine(letters[msgnum].date, buf, sizeof buf))
      {
         register char  *sp = buf;
         while (!isspace(*sp))
            sp++;

         while (isspace(*sp))
            sp++;

         fprintf(f,"On %s,", sp );
      } /* if */

      if (RetrieveLine(letters[msgnum].from, buf, sizeof buf))
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

   imseek(imBox, letters[msgnum].adr, SEEK_SET);
   nextloc = letters[msgnum + 1].adr;

   while (imtell(imBox) < nextloc &&
      imgets(buf, sizeof buf, imBox) != nil(char))
   {

/*--------------------------------------------------------------------*/
/*               Determine if we should write the line                */
/*--------------------------------------------------------------------*/

      print = KWTrue;
      printX = KWFalse;

      switch (headers)
      {
         case noheader:
            print = KWFalse;
            break;

         case nocontinue:
            if ((*buf != '\n') && !isgraph(*buf))
            {
               print = KWFalse;
               break;
            }
            else
               headers = headerFlag;
               /* Fall through ... */

         case autoresent:
            if (( headerFlag == autoresent) &&  /* Allow Fall Through   */
                ( equalni(buf, "Resent-", 7)))
                     printX = KWTrue;

            /* Fall through again ... */

         case ignoresome:
            {
               char entry = 0;
               while ( ignoreList[entry] && print )
               {
                  if (equalni(ignoreList[entry],
                              buf,
                              strlen(ignoreList[entry])))
                  {
                     print = KWFalse;
                     headers = nocontinue;
                  }
                  else
                     entry++;
               }
            } /* case ignoresome */
            break;

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

         if ( printX )
         {
            if ( fputs("X-Previous-", f ) == EOF )
            {
               printerr( "CopyMsg" );
               panic();
            } /* if ( fputs(INDENT, f ) == EOF ) */

         } /* if ( printX ) */
         else if (indent)
         {
            if ( fputs(INDENT, f ) == EOF )
            {
               printerr( "CopyMsg" );
               panic();
            } /* if ( fputs(INDENT, f ) == EOF ) */
         } /* if (indent) */

         if ( fputs(buf, f ) == EOF )
         {
            printerr( "CopyMsg" );
            panic();
         } /* if ( fputs(buf, f ) == EOF ) */

      } /* if (print) */

/*--------------------------------------------------------------------*/
/*  If end of the header, print all data until the end of the input   */
/*--------------------------------------------------------------------*/

      if ( (headers != seperators) && equal(buf, "\n") )
         headers = seperators;

   } /*while*/

   return KWTrue;
} /*CopyMsg*/

/*--------------------------------------------------------------------*/
/*    N u m e r i c                                                   */
/*                                                                    */
/*    Determine if a string is numeric.  Returns KWTrue if string is   */
/*    numeric, else KWFalse.                                           */
/*--------------------------------------------------------------------*/

 KWBoolean Numeric( const char *number)
 {
   char *column = (char *) number;

   if (*column == '\0')
      return KWFalse;

   while( isdigit(*column) )  /* Scan to string end or 1st non-digit */
      column++;
   if ( *column == '\0' )
      return KWTrue;          /* Success if whole string was made of
                                 digits                              */
   else
      return KWFalse;

 } /* Numeric */

/*--------------------------------------------------------------------*/
/*    R e t r i e v e L i n e                                         */
/*                                                                    */
/*    Read a line from a mail header, if available                    */
/*--------------------------------------------------------------------*/

KWBoolean RetrieveLine(const long adr,
                     char *line,
                     const size_t len)
{
   char *cp = line;
   size_t count;

   *line = '\0';              /* Insure nothing to find              */

   if (adr == MISSING)        /* No information to read?             */
      return KWFalse;         /* Report this to caller               */

   if (imseek(imBox, adr, SEEK_SET)) /* Position to data           */
   {                          /* Have a problem?                     */
      printmsg(0,"Failure seeking to %ld offset in mailbox ...",
               adr );

      printerr("mailbox");    /* Yes --> Report and return           */
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                     Actually read the data in                      */
/*--------------------------------------------------------------------*/

   count = imread(line, sizeof *line, len-1, imBox);

   if ((count < (len-1)) && imerror( imBox ))
   {
      printerr( "imread");
      return KWFalse;
   }

   line[count] = '\0';        /* Terminate the string read           */

/*--------------------------------------------------------------------*/
/*    A field continues until a new field begins in column of the     */
/*    next line or the header ends (an empty line); find the end      */
/*    of the field, trimming extra white space from the beginning     */
/*    of each line as we go                                           */
/*--------------------------------------------------------------------*/

   while( (cp = strchr(cp, '\n')) != NULL )
   {
      if ((cp[1] == '\n') || !isspace(cp[1]))   /* End of field?     */
         *cp = '\0';          /* Yes --> Terminate string            */
      else {
         char *next;

         *cp++ = ' ';         /* Convert line break to whitespace    */
         next = cp;           /* Get first position of new line      */

         while( isspace( *next ) )  /* Ignore leading white space    */
            next++;

         memmove( cp, next, strlen(next) + 1 );
                              /* Trim leading white space            */
      } /* else */
   } /* while */

   return KWTrue;

} /* RetrieveLine */

/*--------------------------------------------------------------------*/
/*    R e t u r n A d d r e s s                                       */
/*                                                                    */
/*    Returns the user name (if available and requested) or           */
/*    E-mail address of the user                                      */
/*--------------------------------------------------------------------*/

void ReturnAddress(char *result, const long adr )
{
   char buffer[LSIZE];

#ifdef UDEBUG
   printmsg(15,"ReturnAddress: Return address is %p", result );
#endif

   if (RetrieveLine(adr, buffer, sizeof buffer))
   {
      char *begin = buffer;

#ifdef UDEBUG
   printmsg(15,"ReturnAddress: Return address is %p", result );
#endif

      while (isgraph(*begin))    /* Find end of header name          */
         begin++;

      printmsg(4,"ReturnAddress: Input buffer: %s",buffer);

      if (strlen(begin))
         ExtractName(result,begin);       /* Yes --> Return name     */
      else
         strcpy(result,"-- Invalid From: line --");

   } /* else */
   else
      strcpy(result,"-- Unknown --");     /* No --> Return error     */

} /* ReturnAddress */

/*--------------------------------------------------------------------*/
/*    s a y O p t i o n s                                             */
/*                                                                    */
/*    Announce user options in effect                                 */
/*--------------------------------------------------------------------*/

void sayOptions( FLAGTABLE *flags, const size_t flagsSize )
{

   size_t subscript;
   size_t used = 0;

   printf("\nThe following options are set:\n");

   for (subscript = 0; subscript < flagsSize ; subscript++)
   {
         size_t width;

         if (flags[subscript].bits & B_GLOBAL)
            continue;               /* Don't print system options */

         width = 1 + strlen( flags[subscript].sym ) +
                 ( bflag[ flags[subscript].position ] ? 0 : 2 );


         if ( used )
         {
            if ( (used + width ) > 79 )
            {

               fputc( '\n', stdout);
               used = 0;

            } /* if ( used > 79 ) */
            else
               fputc( ' ', stdout);


         } /* if ( subscript > 0 ) */

         used += width;

         printf("%s%s",
               bflag[ flags[subscript].position ] ? "" : "no",
               flags[subscript].sym );

   } /* for */

   fputc( '\n', stdout);

} /* sayOptions */
