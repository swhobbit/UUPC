/*--------------------------------------------------------------------*/
/*       s m t p n e t w . c                                          */
/*                                                                    */
/*       SMTP generic support for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: smtpnetw.c 1.30 2001/03/12 13:56:08 ahd v1-13k $
 *
 *    $Log: smtpnetw.c $
 *    Revision 1.30  2001/03/12 13:56:08  ahd
 *    Annual Copyright update
 *
 *    Revision 1.29  2000/05/25 03:41:49  ahd
 *    Use more conservative buffering to avoid aborts
 *
 *    Revision 1.28  2000/05/12 12:36:30  ahd
 *    Annual copyright update
 *
 *    Revision 1.27  1999/02/21 04:09:32  ahd
 *    Support for BSMTP support, with routines for batch file I/O
 *    and breakout of TCP/IP routines into their own file.
 *
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

RCSID("$Id: smtpnetw.c 1.30 2001/03/12 13:56:08 ahd v1-13k $");

static const char crlf[] = "\r\n";

#define NULL_TERMINATED_LENGTH  INT_MAX

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static char
*getLineBreak( SMTPBuffer *sb );

static KWBoolean
SetDataLine(SMTPClient *client, char *buffer, size_t length);

static void
SMTPBurpBuffer(SMTPClient *client, size_t burpBytes);

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
   size_t lineLength;
   KWBoolean bResult = KWTrue;

/*--------------------------------------------------------------------*/
/*                   Handle previously signaled EOF                   */
/*--------------------------------------------------------------------*/

   if (isClientEOF(client) && (client->receive.NetworkUsed == 0))
   {
      printmsg(0, "%s: client %d is out of data (EOF)",
                   mName,
                   getClientSequence(client));

      SetDataLine(client, "", 0);
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

         if ((client->receive.NetworkUsed == quitLength) &&
             equalni(client->receive.NetworkBuffer, quit, quitLength))
         {
            printmsg(8,"%s: Applying CR/LF after Netscape %s/EOF",
                     mName,
                     quit);
            SetDataLine(client, client->receive.NetworkBuffer, quitLength);
            SMTPBurpBuffer(client, client->receive.NetworkUsed);
         }
         else {
            printmsg(0, "%s: Client %d Terminated unexpectedly without QUIT",
                       mName,
                       getClientSequence(client));
            SMTPBurpBuffer(client, client->receive.NetworkUsed);

            /* Abort client immediately */
            setClientMode(client, SM_ABORT);
            return KWTrue;
         }
      }
      else if (client->receive.NetworkUsed < MAX_BUFFER_SIZE)
      {
         printmsg(2, "%s: Client %d Input buffer "
                      "(%d bytes) waiting for data.",
                      mName,
                      getClientSequence(client),
                      client->receive.NetworkUsed);

         /* Sleep client for few secs  */
         setClientIgnore(client, (time_t) ++client->stalledReads);

         /* Don't process command yet  */
         return KWFalse;

      } /* END if (client->receive.NetworkUsed < MAX_BUFFER_SIZE) */
      else {

        printmsg(0, "%d <<< %.125s",
                     getClientSequence(client),
                     client->receive.NetworkBuffer);
        printmsg(0, "%s: Client %d Input buffer (%d bytes) full (cannot append more).",
                     mName,
                     getClientSequence(client),
                     client->receive.NetworkUsed);

        SetDataLine(client,
                    client->receive.NetworkBuffer,
                    client->receive.NetworkUsed);
        SMTPBurpBuffer(client, client->receive.NetworkUsed);

        /* Abort client immediately     */
        setClientMode(client, SM_ABORT);
        return KWTrue;

      } /* END else */

   } /* END if (lineBreak == NULL) */

   lineLength = lineBreak - client->receive.NetworkBuffer;
   bResult = SetDataLine(client,
                         client->receive.NetworkBuffer,
                         lineLength);

   SMTPBurpBuffer(client, lineLength + 2);
   return bResult;

} /* END SMTPGetLine */

/*--------------------------------------------------------------------*/
/*       g e t L i n e B r e a k                                      */
/*                                                                    */
/*       Determine where end of next line is                          */
/*--------------------------------------------------------------------*/

static char
*getLineBreak( SMTPBuffer *sb )
{

   static const char mName[] = "getLineBreak";

   if ( sb->NetworkUsed == 0)
      return NULL;

   return memstr(sb->NetworkBuffer, crlf, sb->NetworkUsed);

} /* END getLineBreak */

/*--------------------------------------------------------------------*/
/*      S e t D a t a L i n e                                         */
/*                                                                    */
/*      Fill in data buffer for client use                            */
/*--------------------------------------------------------------------*/

static KWBoolean
SetDataLine(SMTPClient *client, char *buffer, size_t length)
{
    static const int nBaseLevel = 2;
    int nTraceLevel = nBaseLevel;

    /* Determine the length of the buffer, if needed */
    if (length == NULL_TERMINATED_LENGTH)
    {
       client->receive.DataUsed = strlen(buffer);
    }
    else {
       client->receive.DataUsed = length;
    }

/*--------------------------------------------------------------------*/
/*               Insure the data buffer is large enough               */
/*--------------------------------------------------------------------*/

    if (client->receive.DataUsed >= client->receive.DataAllocated)
    {
        /* Compute new buffer length with some extra slop */
        client->receive.DataAllocated = client->receive.DataUsed + 80;

        if (client->receive.DataBuffer != NULL)
        {
            free(client->receive.DataBuffer);
        }

        /* Get the buffer now needed, including NULL terminator */
        client->receive.DataBuffer = malloc(client->receive.DataAllocated);
        checkref(client->receive.DataBuffer);

    } /* END if (client->receive.DataUsed > client->receive.DataAllocated) */

/*--------------------------------------------------------------------*/
/*          If in data mode, simply copy the data and return          */
/*--------------------------------------------------------------------*/

   if (isClientFlag(client, SF_NO_TOKENIZE))
   {
      clearClientFlag(client, SF_NO_TOKENIZE);
      nTraceLevel += 3;
   }
   else if (client->receive.DataUsed > 0)
   {
      /* Strip leading spaces, if needed */
      while(isspace(*buffer))
      {
         buffer++;
         client->receive.DataUsed--;
      }

      /* If we removed all characters, command is missing -- an error */
      if (client->receive.DataUsed == 0)
      {
         printmsg(0, "%d <<<   (empty line with %d characters)",
                      getClientSequence(client),
                      client->receive.DataUsed);
         setClientIgnore(client, 2);      /* Make client wait */

         /* Ignore input line */
         return KWFalse;
      }

   } /* END if (client->receive.DataUsed != 0) */

/*--------------------------------------------------------------------*/
/*              Copy the data then print it for tracing               */
/*--------------------------------------------------------------------*/

   if (client->receive.DataUsed > 0)
   {
      memcpy(client->receive.DataBuffer,
             buffer,
             client->receive.DataUsed);
   }

   client->receive.DataBuffer[client->receive.DataUsed] = '\0';

   if ((nTraceLevel == nBaseLevel) &&
       (equalni(client->receive.DataBuffer, "pass", 4)))

   {
      printmsg(nTraceLevel,"%d <<< %.4s xxxxxxxx",
               getClientSequence(client),
               client->receive.DataBuffer);
   }
   else {
      printmsg(nTraceLevel,"%d <<< %.125s",
                 getClientSequence(client),
                 client->receive.DataBuffer);
   }

/*--------------------------------------------------------------------*/
/*                      Report success to caller                      */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* END SetDataLine */

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

   } /* END switch(code) */

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

   } /* END if (code == PR_TEXT) */

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

} /* END SMTPResponse */

/*--------------------------------------------------------------------*/
/*       S M T P B u r p B u f f e r                                  */
/*                                                                    */
/*       Discard already processed data from the input buffer         */
/*--------------------------------------------------------------------*/

static void
SMTPBurpBuffer(SMTPClient *client, size_t burpBytes)
{
   static const char mName[] = "SMTPBurpBuffer";

/*--------------------------------------------------------------------*/
/*                 Verify the status of the input buffer              */
/*--------------------------------------------------------------------*/

   assertSMTP(client);

   if (client->receive.NetworkAllocated < client->receive.NetworkUsed)
   {
         printmsg(0, "%s: Client has used more bytes (%d) "
                     "than network buffer allocated (%d)",
                    mName,
                    getClientSequence(client),
                    client->receive.NetworkUsed,
                    client->receive.NetworkAllocated);

      panic();
   }

   if (client->receive.NetworkUsed < burpBytes)
   {
         printmsg(0, "%s: Client %d has parsed more bytes (%d) "
                     "than network buffer in use (%d)",
                    mName,
                    getClientSequence(client),
                    burpBytes,
                    client->receive.NetworkUsed);

      panic();
   }

/*--------------------------------------------------------------------*/
/*            Handle simple case of no new data in buffer             */
/*--------------------------------------------------------------------*/

   if (burpBytes == 0)
   {
      return;
   }

/*--------------------------------------------------------------------*/
/*     Discard any data we have already processed from the client     */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG2
   printmsg(5,"%s: Burped %ld of %ld bytes from buffer",
            mName,
            (long) burpBytes,
            (long) client->receive.NetworkUsed);
#endif

   client->receive.NetworkUsed -= burpBytes;

/*--------------------------------------------------------------------*/
/*       If we still have data in the buffer, move it to front and    */
/*       reset pointer to it.                                         */
/*--------------------------------------------------------------------*/

   if (client->receive.NetworkUsed > 0)
   {
      memmove(client->receive.NetworkBuffer,
              client->receive.NetworkBuffer + burpBytes,
              client->receive.NetworkUsed);
   }

} /* END SMTPBurpBuffer */
