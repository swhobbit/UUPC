/*--------------------------------------------------------------------*/
/*       s m t p n e t w . c                                          */
/*                                                                    */
/*       SMTP generic support for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.26 1999/01/22 01:25:46 ahd Exp $
 *
 *    $Log: smtpnetw.c $
 *    Revision 1.26  1999/01/22 01:25:46  ahd
 *    Correct SMTP logic error message to include client
 *
 *    Revision 1.25  1999/01/17 17:19:16  ahd
 *    Give priority to accepting new connections
 *    Make initialization of slave and master connections more consistent
 *
 *    Revision 1.24  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.23  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.22  1998/11/04 01:59:55  ahd
 *    Prevent buffer overflows when processing UIDL lines
 *    in POP3 mail.
 *    Add internal sanity checks for various client structures
 *    Convert various files to CR/LF from LF terminated lines
 *
 * Revision 1.21  1998/05/11  13:55:28  ahd
 * Reduce buffer size to handle OS/2 builds
 *
 *    Revision 1.20  1998/05/11 01:20:48  ahd
 *    Length line of trace messages in log
 *
 *    Revision 1.19  1998/04/27 01:45:15  ahd
 *    Revamp bufferring for larger buffers
 *
 *    Revision 1.18  1998/04/24 03:30:13  ahd
 *    Use local buffers, not client->transmit.buffer, for output
 *    Rename receive buffer, use pointer into buffer rather than
 *         moving buffered data to front of buffer every line
 *    Restructure main processing loop to give more priority
 *         to client processing data already buffered
 *    Add flag bits to client structure
 *    Add flag bits to verb tables
 *
 *    Revision 1.17  1998/04/22 01:19:54  ahd
 *    Performance improvements for SMTPD data mode
 *
 *    Revision 1.16  1998/04/19 15:30:08  ahd
 *    Improved error messages for network errors
 *
 *    Revision 1.15  1998/04/08 11:35:35  ahd
 *    CHange error processing for bad sockets
 *
 *    Revision 1.14  1998/03/16 07:49:07  ahd
 *    Make NETSCAPE send CR/LF
 *
 *    Revision 1.13  1998/03/16 06:42:49  ahd
 *    Allow larger receive buffers
 *
 *    Revision 1.12  1998/03/08 23:10:20  ahd
 *    Allow raw message transmission for POP messages
 *    Make all receive errors fatal
 *
 *    Revision 1.11  1998/03/08 04:50:04  ahd
 *    Close socket after read errors
 *
 *    Revision 1.10  1998/03/06 06:51:28  ahd
 *    Add commands to make Netscape happy
 *
 *    Revision 1.9  1998/03/03 03:51:53  ahd
 *    Routines to handle messages within a POP3 mailbox
 *
 *    Revision 1.8  1998/03/01 19:42:17  ahd
 *    First compiling POP3 server which accepts user id/password
 *
 *    Revision 1.7  1997/11/29 13:03:13  ahd
 *    Clean up single client (hot handle) mode for OS/2, including correct
 *    network initialization, use unique client id (pid), and invoke all
 *    routines needed in main client loop.
 *
 */

/*--------------------------------------------------------------------*/
/*                         Standard includes                          */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>
#include <ctype.h>

#include "catcher.h"
#include "memstr.h"

#include "smtpnett.h"
#include "smtpnetw.h"
#include "smtpverb.h"

/*--------------------------------------------------------------------*/
/*                      Global defines/variables                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: smtpnetw.c 1.26 1999/01/22 01:25:46 ahd Exp $");

static const char crlf[] = "\r\n";

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static void
SMTPBurpBuffer(SMTPClient *client);

/*--------------------------------------------------------------------*/
/*       g e t L i n e B r e a k                                      */
/*                                                                    */
/*       Determine where end of next line is                          */
/*--------------------------------------------------------------------*/

static char
*getLineBreak( SMTPBuffer *sb )
{
   size_t offset;
   static const char mName[] = "getLineBreak";

   if ( sb->next == NULL)
      return NULL;

   offset = sb->next - sb->buffer;

#ifdef UDEBUG
   if (offset > sb->used)
   {
      printmsg(0,"%s: Attempted to scan %ld bytes in a "
                  "buffer only %ld bytes long",
                  mName,
                  (long) offset,
                  (long) sb->used);
      panic();
   }
#endif

   return memstr(sb->next, crlf, sb->used - offset);

} /* lineBreak */

/*--------------------------------------------------------------------*/
/*       S M T P G e t L i n e                                        */
/*                                                                    */
/*       Read an SMTP command from our internal buffer                */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPGetLine(SMTPClient *client)
{
   static const char mName[] = "SMTPGetLine";
   char *lineBreak;

#ifdef UDEBUG2

#define ignoredBytes(x) (((x).next == NULL) ? 0 : (x).next - (x).buffer)

   if ( debuglevel >= 8 )
      printmsg(8, "%s: Client %d in mode 0x%04x "
                   "with %d of possible %d bytes buffered (%d ignored)",
                   mName,
                   getClientSequence(client),
                   getClientMode(client),
                   client->receive.used,
                   client->receive.allocated,
                   ignored(client->receive));
#endif

/*--------------------------------------------------------------------*/
/*                   Handle previously signaled EOF                   */
/*--------------------------------------------------------------------*/

   if (isClientEOF(client) && (client->receive.next == NULL))
   {
      printmsg(0, "%s: client %d is out of data (EOF)",
                   mName,
                   getClientSequence(client));

      client->receive.line = client->receive.buffer;
      client->receive.line[ 0 ] = '\0';
      setClientMode(client, SM_ABORT);
      return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Read more data if we have some and need it,                  */
/*--------------------------------------------------------------------*/

   lineBreak = getLineBreak(&client->receive);

   if (getClientReady(client) && (lineBreak == NULL))
   {
      if (client->stalledReads)     /* Improve response time ...     */
         client->stalledReads--;

      SMTPBurpBuffer(client);
      SMTPRead(client);
      lineBreak = getLineBreak(&client->receive);
   }

/*--------------------------------------------------------------------*/
/*              If no data available, return immediately              */
/*--------------------------------------------------------------------*/

   if (lineBreak == NULL)
   {

      if (isClientEOF(client))
      {
         static const char quit[] = "quit";
         static const size_t quitLength = sizeof quit - 1;

         if ((client->receive.next == client->receive.buffer) &&
             (client->receive.used == quitLength) &&
             equalni(client->receive.buffer, "quit", quitLength))
         {
            printmsg(8,"%s: Applying CR/LF after Netscape %s/EOF",
                     mName,
                     quit);
            lineBreak = client->receive.buffer + quitLength;
            client->receive.used += 2;
         }
         else {
            printmsg(0, "%s: Client %d Terminated unexpectedly without QUIT",
                       mName,
                       getClientSequence(client));
            client->receive.line = NULL;

            /* Abort client immediately */
            setClientMode(client, SM_ABORT);
            return KWTrue;
         }
      }
      else if (client->receive.used < client->receive.allocated)
      {
         printmsg(2, "%s: Client %d Input buffer "
                      "(%d bytes) waiting for data.",
                      mName,
                      getClientSequence(client),
                      client->receive.used);

         /* Sleep client for few secs  */
         setClientIgnore(client, (time_t) ++client->stalledReads);

         /* Don't process command yet  */
         return KWFalse;

      } /* if (client->receive.used < client->receive.allocated) */
      else {

        printmsg(0, "%d <<< %.125s",
                     getClientSequence(client),
                     client->receive.next);
        printmsg(0, "%s: Client %d Input buffer (%d bytes) overrun.",
                     mName,
                     getClientSequence(client),
                     client->receive.used);

        client->receive.lineLength = client->receive.used - 1;

        /* Don't run off end of the buffer */
        client->receive.buffer[client->receive.used - 1] = '\0';

        /* Abort client immediately     */
        setClientMode(client, SM_ABORT);
        return KWTrue;

      } /* else */

   } /* if (lineBreak == NULL) */

   client->receive.line = client->receive.next;
   client->receive.lineLength = lineBreak - client->receive.line;

   /* Terminate the command line and step past the CR/LF */
   *(lineBreak++) = '\0';
   *(lineBreak++) = '\0';

   /* Remember where our next line starts, if any */
   client->receive.next = lineBreak;

   if (client->receive.next == (client->receive.buffer +
                                client->receive.used))
   {
      client->receive.next = NULL;

#ifdef UDEBUG2
      if (debuglevel >= 6)
         printmsg(6,"%s: Last line in buffer %p (%d bytes) at %p: %.125s",
                  mName,
                  client->receive.buffer,
                  client->receive.allocated,
                  client->receive.line,
                  client->receive.line );
#endif
   }

/*--------------------------------------------------------------------*/
/*           Locate start of input line if not in data mode           */
/*--------------------------------------------------------------------*/

   if (isClientFlag(client, SF_NO_TOKENIZE))
   {
      clearClientFlag(client, SF_NO_TOKENIZE);

      printmsg(5,"%d <<< %.125s",
               getClientSequence(client),
               client->receive.line );
   }
   else {

      while(isspace(*(client->receive.line)))
         client->receive.line++;

      if (*(client->receive.line) == '\0')
      {
         printmsg(0, "%d <<<   (empty line with %d characters)",
                      getClientSequence(client),
                      client->receive.lineLength);
         client->receive.lineLength= 0;
         setClientIgnore(client, 2);      /* Make client wait */

         /* Ignore input line */
         return KWFalse;
      }

      /* Recompute (perhaps updated) line length */
      client->receive.lineLength = lineBreak - client->receive.line - 2;

      if ( equalni(client->receive.line, "pass", 4))
         printmsg(2,"%d <<< %.4s xxxxxxxx",
                  getClientSequence(client),
                  client->receive.line );
      else
         printmsg(2,"%d <<< %.125s",
                    getClientSequence(client),
                    client->receive.line );

   } /* else */

   return KWTrue;

} /* SMTPGetLine */

/*--------------------------------------------------------------------*/
/*       S M T P R e s p o n s e                                      */
/*                                                                    */
/*       Send a response to a remote client                           */
/*--------------------------------------------------------------------*/

KWBoolean
SMTPResponse(SMTPClient *client, int code, const char *text)
{
   static const char mName[] = "SMTPResponse";

   char buf[BUFSIZ];
   size_t totalLength;
   KWBoolean buffered = KWFalse;
   int printLevel = 2;

   assertSMTP(client);

   switch(code)
   {
         case 0:
            strcpy(buf, "??? ");
            printLevel = 0;
            break;

         case PR_TEXT:
            *buf = '\0';
            printLevel = 8;
            break;

         case PR_DATA:
            *buf = '\0';
            break;

         case PR_OK_GENERIC:
            strcpy(buf, "+OK ");
            break;

         case PR_ERROR_GENERIC:
            printLevel = 0;
            /* Fall through to warning */

         case PR_ERROR_WARNING:
            strcpy(buf, "-ERR ");
            break;

         default:
            /* Numeric codes for SMTP */
            switch(code / 100)
            {
               case 4:
               case 5:
                  printLevel = 0;
                  break;

               default:
                  break;
            }

            sprintf(buf, "%03.3d%c",
                          (code < 0) ? - code : code,
                          (code < 0) ? '-' : ' ');
            break;

   } /* switch(code) */

   if (printLevel <= debuglevel)
   {
      printmsg(printLevel,"%d >>> %s%.125s",
                          getClientSequence(client),
                          buf,
                          text);
   }

   totalLength = strlen(buf) + strlen(text);
   incrementClientBytesWritten(client, totalLength);
   incrementClientLinesWritten(client);

/*--------------------------------------------------------------------*/
/*       Special case for raw text with CR/LF already appended        */
/*--------------------------------------------------------------------*/

   if (code == PR_TEXT)
   {
      if(!SMTPWrite(client, text, totalLength))
      {
         printmsg(0,"%s: Error sending %u bytes to remote host: \"%.125s\"",
                    mName,
                    totalLength,
                    text);
         return KWFalse;
      }

      return KWTrue;

   } /* if (code == PR_TEXT) */

/*--------------------------------------------------------------------*/
/*       If all three parts of the message fit, pack it into one      */
/*       buffer so we only call the network write once to send        */
/*       one packet.                                                  */
/*--------------------------------------------------------------------*/

   totalLength += strlen(crlf);
   incrementClientBytesWritten(client, strlen(crlf));

   if (totalLength < sizeof buf)
   {
      strcat(buf, text);
      strcat(buf, crlf);
      buffered = KWTrue;
   }

   if(!SMTPWrite(client, buf, strlen(buf)))
   {
      printmsg(0,"%s: Error sending response code to remote host: %.4s %.125s",
                  mName,
                  buf,
                  text);
      return KWFalse;
   }

   if (buffered)                    /* All info written from buffer? */
      return KWTrue;                /* Yes --> Report success        */

   if(!SMTPWrite(client, text, strlen(text)))
   {
      printmsg(0,"%s: Error sending %d bytes to remote host: %s%.125s",
                 mName,
                 strlen(text),
                 buf,
                 text);
      return KWFalse;
   }

   if(!SMTPWrite(client, crlf, strlen(crlf)))
   {
      printmsg(0, "%s: Error sending CR/LF to remote host: %s%.125s",
                  mName,
                  buf,
                  text);
      return KWFalse;
   }

   return KWTrue;

} /* SMTPResponse */

/*--------------------------------------------------------------------*/
/*       S M T P B u r p B u f f e r                                  */
/*                                                                    */
/*       Discard already processed data from the input buffer         */
/*--------------------------------------------------------------------*/

static void
SMTPBurpBuffer(SMTPClient *client)
{
   static const char mName[] = "SMTPBurpBuffer";

/*--------------------------------------------------------------------*/
/*                 Verify the status of the input buffer              */
/*--------------------------------------------------------------------*/

   assertSMTP(client);

   if (client->receive.allocated < client->receive.used)
   {
         printmsg(0, "%s: Client has used more bytes (%d) "
                     "than buffer bytes allocated (%d)",
                    mName,
                    getClientSequence(client),
                    client->receive.used,
                    client->receive.allocated);

      panic();
   }

   if (client->receive.used < client->receive.lineLength)
   {
         printmsg(0, "%s: Client %d has parsed more bytes (%d) "
                     "than bytes in use (%d)",
                    mName,
                    getClientSequence(client),
                    client->receive.lineLength,
                    client->receive.used);

      panic();
   }

/*--------------------------------------------------------------------*/
/*            Handle simple case of no new data in buffer             */
/*--------------------------------------------------------------------*/

   if (client->receive.next == NULL)
   {
      client->receive.used = 0;
      client->receive.lineLength = 0;
      return;
   }

/*--------------------------------------------------------------------*/
/*     Discard any data we have already processed from the client     */
/*--------------------------------------------------------------------*/

   if (client->receive.next > client->receive.buffer)
   {
#ifdef UDEBUG2
      printmsg(5,"%s: Burped %ld of %ld bytes from buffer",
               mName,
               (long) (client->receive.next - client->receive.buffer),
               (long) client->receive.used );
#endif

      client->receive.used -= client->receive.next - client->receive.buffer;

/*--------------------------------------------------------------------*/
/*       If we still have data in the buffer, move it to front and    */
/*       reset pointer to it.  Otherwise, clear pointer to next       */
/*       line completely.                                             */
/*--------------------------------------------------------------------*/

      if (client->receive.used > 0)
      {
         memmove(client->receive.buffer,
                 client->receive.next,
                 client->receive.used);

         client->receive.next = client->receive.buffer;
      }
      else
         client->receive.next = NULL;

   } /* if (client->receive.parsed > 0) */

} /* SMTPBurpBuffer */
