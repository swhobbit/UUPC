/*--------------------------------------------------------------------*/
/*       d e l i v e r s . C                                          */
/*                                                                    */
/*       SMTP delivery UUPC/extended; ported into UUPC/extended       */
/*       from original stand alone SMTPSEND                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       MODULE SMTPSEND : Send a mail to a host in SMTP protocol.    */
/*--------------------------------------------------------------------*/
/*       This code is placed in the public domain by its author to    */
/*       be used as you wish.  However, if you add neat new           */
/*       features, I'd appreciate having a copy sent to me at         */
/*       jack.rebiscoul@ssti.fr.                                      */
/*--------------------------------------------------------------------*/
/*       AUTHOR: Jacques REBISCOUL S.S.T.I.  jack.rebiscoul@ssti.fr   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Note:  No matter how different this looks from the           */
/*       original SMTPSend program, the core structure and            */
/*       housekeeping details were straight from the original         */
/*       program.  No matter what Tweaks (and bugs!) were added by    */
/*       Kendra Electronic Wonderworks, but a major debt is owed to   */
/*       Jacques.                                                     */
/*                                                                    */
/*       Major changes from the original:                             */
/*                                                                    */
/*          - Generic read/write functions are broken into            */
/*            smtptcp.c                                               */
/*          - Bad sender addresses are bounced                        */
/*          - SMTP data with leading periods (.) are handled          */
/*          - Errors which are fatal for connection do not cause      */
/*            program to exit                                         */
/*          - Exploits UUPC/extended routines for logging,            */
/*            bouncing mail, etc.                                     */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: delivers.c 1.17 1999/01/11 05:43:36 ahd Exp $
 *
 *       $Log: delivers.c $
 *       Revision 1.17  1999/01/11 05:43:36  ahd
 *       Revamp error processing to be more robust
 *
 *       Revision 1.16  1999/01/08 04:31:09  ahd
 *       Improve SMTP error handling
 *
 *       Revision 1.15  1999/01/08 02:21:01  ahd
 *       Convert currentfile() to RCSID()
 *
 *       Revision 1.14  1999/01/04 03:54:27  ahd
 *       Annual copyright change
 *
 *       Revision 1.13  1998/08/29 14:17:11  ahd
 *       Add setTitle() commands to report delivery progress
 *
 *       Revision 1.12  1998/07/27 01:03:54  ahd
 *       Add work around for sending overlength lines (now that
 *       rmail doesn't puke on such lines on input)
 *
 * Revision 1.11  1998/03/01  01:33:06  ahd
 * Annual Copyright Update
 *
 *       Revision 1.10  1997/12/14 21:17:03  ahd
 *       Transform selected well-known addresses into the error-response
 *       empty (<>) address.
 *
 *       Revision 1.9  1997/12/14 02:46:03  ahd
 *       Correct formatting of delivery message
 *
 *       Revision 1.8  1997/12/14 00:43:16  ahd
 *       Further cleanup of new sender protocol
 *
 *       Revision 1.7  1997/12/13 18:11:01  ahd
 *       Change parsing and passing of sender address information
 *
 *       Revision 1.6  1997/11/30 04:21:39  ahd
 *       Delete older RCS log comments, force full address for SMTP delivery,
 *       recongize difference between local and remote delivery
 *
 *       Revision 1.5  1997/11/29 12:59:50  ahd
 *       Correct SMTP outbound to correctly quote lines of all periods
 *       Enhance SMTP outbound to transmit CR/LF with rest of line when possible
 *       Suppress compiler warnings
 *
 */

/*--------------------------------------------------------------------*/
/*                           Include files                            */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"
#include "deliver.h"
#include "deliverm.h"
#include "delivers.h"
#include "timestmp.h"
#include "address.h"
#include "title.h"
#include "../uucico/commlib.h"

RCSID("$Id: delivers.c 1.17 1999/01/11 05:43:36 ahd Exp $");

/*--------------------------------------------------------------------*/
/*                       Local type definitions                       */
/*--------------------------------------------------------------------*/

typedef short KWNetStatus;

/* Special Network Status Codes */
enum
   {
      KWNSSuccess = 2,              /* No major issue with function  */
      KWNSNetTimeout,               /* No response from server       */
      KWNSNoNet,                    /* Net connection failed         */
      KWNSInternalError,            /* Undetermined error            */
      KWNSWrongReply,               /* Wrong but Successful reply    */
      KWNSTransientError,           /* 4xx temporary protocol prob   */
      KWNSPermanentError            /* 5xx permanent protocol error  */
   };

#define SMTP_PORT_NUMBER 25

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char    traceBuffer[10240];     /* For printing errors        */
static size_t  traceUsed;              /* Length of trace buffer     */

KWBoolean suspend_processing = KWFalse;
                             /* Dummy to replace copy in suspend.c   */

typedef struct _SMTP_ADDR
   {
      char *address;
      KWBoolean processed;
      KWBoolean retry;
   } SMTP_ADDR;

/*--------------------------------------------------------------------*/
/*                             Local macros                           */
/*--------------------------------------------------------------------*/

#define CRLF      "\r\n"
#define CRLF_LEN  2

#define SAVE_TRACE_BUFFER() (traceUsed)
#define RESTORE_TRACE_BUFFER(x)  {  \
            traceUsed = (x);    \
            traceBuffer[x] ='\0'; }
#define GET_TRACE_BUFFER() (traceBuffer)

/*--------------------------------------------------------------------*/
/*                          Local prototypes                          */
/*--------------------------------------------------------------------*/

static void
ConnectSMTP(
   IMFILE *imf,                     /* Temporary input file          */
   const MAIL_ADDR *sender,         /* Originating (error) address   */
   const char *relay,               /* SMTP host to connect to       */
   SMTP_ADDR *toAddress,            /* List of target addressess     */
   int count,                       /* Number of addresses to send   */
   const KWBoolean validate         /* Perform bounce on failure     */
);

/*--------------------------------------------------------------------*/
/*       b u f f e r T r a c e                                        */
/*                                                                    */
/*       Inject new data into our trace buffer                        */
/*--------------------------------------------------------------------*/

static void
bufferTrace( const char *prefix,
             const char *string,
             int stringLength)
{
   size_t shiftLength = stringLength;
   size_t prefixLength = strlen(prefix);
   static const char mName[] = "bufferTrace";

   /* We perform tracing to the program log for free */
   switch(*prefix)
   {
      case '>':
      case '<':
      case ']':
      case '[':
         printmsg(2, "%s %.120s", prefix, string);
         break;

      default:
         /*
          *    No op for other characters, which prefixes to no-traffic
          *    information for the log
          */
         break;
   } /* switch(prefix) */

   if (stringLength == 0)
      stringLength = strlen(string);

   /* Add total bytes of line together, plus extra 3 for leading
      spaces, space after prefix, and trailing newline */
   shiftLength = stringLength + prefixLength + 5;

/*--------------------------------------------------------------------*/
/*       If we need to make room in our buffer, drop at least one     */
/*       of line of data                                              */
/*--------------------------------------------------------------------*/

   if (((sizeof traceBuffer) - traceUsed) <= (shiftLength+1))
   {
      char *p;
      size_t burpLength;

      if ((shiftLength+1) >= sizeof traceBuffer)
         p = NULL;
      else {
         /* Try to shift enough data we don't need to do this
            every pass */
         if (shiftLength  < (sizeof traceBuffer / 2))
            shiftLength = (sizeof traceBuffer / 2);
         p = strchr(traceBuffer + shiftLength + 1,'\n');
      }

      if (p == NULL)
      {
         printmsg(0,"%s: Cannot save entire trace message (length %d): %s %.75s",
                     mName,
                     shiftLength,
                     prefix,
                     string);

         /* Just log trailing part of message into emptied buffer */
         traceUsed = 0;
         bufferTrace(prefix, string + (stringLength / 2), 0);
         return;
      }

      p += 1;                          /* Step past newline character */
      burpLength = traceBuffer - p;
      memmove(traceBuffer, p, burpLength);   /* Actually free the space */
      traceUsed -= burpLength;

      printmsg(4,"%s Burped %d bytes from trace buffer",
                 mName,
                 burpLength);

   } /* if (((sizeof traceBuffer) - traceUsed) <= stringLength) */

/*--------------------------------------------------------------------*/
/*                      Insert the new line of data                   */
/*--------------------------------------------------------------------*/

   traceBuffer[traceUsed++] = ' ';
   traceBuffer[traceUsed++] = ' ';
   traceBuffer[traceUsed++] = ' ';

   memcpy(traceBuffer + traceUsed, prefix, prefixLength);
   traceUsed += prefixLength;

   traceBuffer[traceUsed++] = ' ';

   memcpy(traceBuffer + traceUsed, string, stringLength);
   traceUsed += stringLength;

   /* Add terminating newline and null character */
   traceBuffer[traceUsed++] = '\n';
   traceBuffer[traceUsed] = '\0';

} /* bufferTrace */

/*--------------------------------------------------------------------*/
/*    D e l i v e r S M T P                                           */
/*                                                                    */
/*    Perform control processing for delivery to another UUCP node    */
/*--------------------------------------------------------------------*/

size_t DeliverSMTP(IMFILE *imf,           /* Input file name          */
                    const MAIL_ADDR *sender,
                    const char *targetAddress,
                    const char *path)
{

   static char *savePath = NULL;    /* System we previously queued for*/
   static SMTP_ADDR addrList[50];
   static int subscript = 0;
   static int addressMax = sizeof addrList / sizeof addrList[0];
   static KWBoolean recursive = KWFalse;  /* Only one connection at a time */

   if (recursive)
   {
      if (path == NULL)
      {
         printmsg(0,"DeliverSMTP: Recursive flush call!");
         panic();
      }

      return DeliverRemote(imf,
                            sender,
                            targetAddress,
                            E_nodename);
   } /* if (recursive) */

   recursive = KWTrue;

/*--------------------------------------------------------------------*/
/*            Flush previously queued addresses, if needed            */
/*--------------------------------------------------------------------*/

   if (subscript)
   {
      KWBoolean queueNow = KWFalse;

      if (path == NULL)
         queueNow = KWTrue;
      else if (! equal(savePath, path))
         queueNow = KWTrue;
      else if (subscript >= addressMax)
         queueNow = KWTrue;

/*--------------------------------------------------------------------*/
/*                We need to actually perform delivery                */
/*--------------------------------------------------------------------*/

      if (queueNow)
      {
         ConnectSMTP(imf,
                      sender,
                      savePath,
                      addrList,
                      subscript,
                      KWTrue);

         while(subscript-- > 0)
         {
            /* Queue failed SMTP mail for local node for retry */
            if (!addrList[subscript].processed)
            {
               printmsg(0,"Requeuing mail for %s:",
                           addrList[subscript].address);
               DeliverRemote(imf,
                              sender,
                              addrList[subscript].address,
                              E_nodename);
            }

            free(addrList[subscript].address);
            addrList[subscript].address = NULL;
         }

         subscript = 0;

      } /* if (queueNow && subscript) */

   } /* if (savePath != NULL) */

/*--------------------------------------------------------------------*/
/*                Return if we only flushing the cache                */
/*--------------------------------------------------------------------*/

   if (path == NULL)
   {
      recursive = KWFalse;
      return 0;
   }

/*--------------------------------------------------------------------*/
/*               Report and queue the current delivery                */
/*--------------------------------------------------------------------*/

   printmsg(1,"Queuing SMTP mail %sfrom %s to %s via %s",
               formatFileSize(imf),
               sender->address,
               targetAddress,
               path);

   savePath = newstr(path);
   memset(&addrList[subscript], 0, sizeof addrList[subscript]);
   addrList[subscript].address = strdup(targetAddress);
   checkref(addrList[subscript].address);
   subscript++;

   recursive = KWFalse;
   return 1;

} /* DeliverSMTP */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P C m d                                        */
/*                                                                    */
/*       Send a arbitary command to remote SMTP server                */
/*--------------------------------------------------------------------*/

static KWNetStatus
SendSMTPCmd(
  char        *cmd      /* IN The command to send */
)
{
   char buffer[KW_BUFSIZ];
   int len = (int) strlen(cmd);
   KWBoolean buffered = KWFalse;

  bufferTrace(">>>", cmd, len);

/*--------------------------------------------------------------------*/
/*       Buffer the command if possible to allow appending the        */
/*       CR/LF for transmission of the resulting logical line as      */
/*       one network write.                                           */
/*--------------------------------------------------------------------*/

  if (sizeof buffer > (len + 3))
  {
     strcpy(buffer, cmd);
     strcpy(buffer + len, CRLF);
     len += CRLF_LEN;
     cmd = buffer;
     buffered = KWTrue;
  }

/*--------------------------------------------------------------------*/
/*           Actually perform the network write of the command        */
/*--------------------------------------------------------------------*/

  if (swrite(cmd, len) < len)
  {
     printmsg(0,"Error sending command to remote host");
     return KWNSNoNet;
  }

/*--------------------------------------------------------------------*/
/*       If the command was not buffered, write the trailing CR/LF    */
/*--------------------------------------------------------------------*/

  if (!buffered && (swrite(CRLF, CRLF_LEN) < CRLF_LEN))
  {
    printmsg(0, "Error sending CR/LF to remote host");
    return KWNSNoNet;
  }

  return KWNSSuccess;

} /* SendSMTPCmd */

/*--------------------------------------------------------------------*/
/*       G e t s S M T P                                              */
/*                                                                    */
/*       Retrieve CR/LF terminated line from SMTP network             */
/*       connection                                                   */
/*--------------------------------------------------------------------*/

static KWNetStatus
GetsSMTP(
  char          *buffer,         /* IN  Buffer in which to store     */
  unsigned int  len,             /* IN  Maximum bytes to return      */
  unsigned int  timeout,         /* IN  time to wait for reply       */
  unsigned int  *returned        /* OUT Actual byte count returned   */
)
{
  unsigned int    i;
  char c;

  /* Read until newline, EOF or buffer full */
  for(i=0; i < (len-1); i++)
  {
    if (! sread(buffer + i, 1, timeout))
    {

       bufferTrace("***", "Network Read Failed", 0);
       printmsg(0, "GetsSMTP: Read timeout after %u seconds",
                 timeout);
       return KWNSNetTimeout;
    }

    /* After we have at least two characters, check for CR/LF pair */
    if (i && ! memcmp(buffer + i - 1, CRLF, CRLF_LEN))
    {
        buffer[--i] = '\0';
        *returned = i;

        /* Log message both locally and into trace buffer */
        bufferTrace("<<<", buffer, i);
        return KWNSSuccess;
    }

  } /* for(i=0; i <(len-1); i++) */

/*--------------------------------------------------------------------*/
/*              Discard the rest of the overlength line               */
/*--------------------------------------------------------------------*/

   do {
      if (!sread(&c, 1, timeout))
      {
         bufferTrace("***", "Network Read Failed", 0);
         printmsg(0, "GetsSMTP: Read timeout after %u seconds",
                   timeout);
         return KWNSNetTimeout;
      }
   } while( c != '\n');

/*--------------------------------------------------------------------*/
/*       Terminate buffer without the CRLF at the end and report      */
/*       this length to caller                                        */
/*--------------------------------------------------------------------*/

  buffer[--len] = '\0';
  *returned = len;

  /* Log message both locally and into trace buffer */
  bufferTrace("[[[", buffer, len);

  return KWNSSuccess;

} /* GetsSMTP */

/*--------------------------------------------------------------------*/
/*       G e t S M T P R e p l y                                      */
/*                                                                    */
/*       Retrieve an SMTP response and extract the reply code         */
/*--------------------------------------------------------------------*/

static KWNetStatus
GetSMTPReply(unsigned int timeout,     /* IN  SMTP read timeout      */
             int *code)                /* OUT SMTP response code     */
{
   char SMTPRecvBuffer[KW_BUFSIZ];

   /* Insure this is invalid until we set it */
   *code = -1;

  /* To support multi-line responses, we check xxx- format responses */
  do {
      int returned;
      KWNetStatus status = GetsSMTP(SMTPRecvBuffer,
                                    sizeof(SMTPRecvBuffer),
                                    timeout,
                                    &returned);
      if (status != KWNSSuccess)
         return status;

  } while (SMTPRecvBuffer[3] == '-');

  /* Convert reply in code */
  *code = atoi(SMTPRecvBuffer);
  return KWNSSuccess;

} /* GetSMTPReply */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P C m d C h e c k R e p l y                    */
/*                                                                    */
/*       Send an SMTP command, retrieve the response and check        */
/*       this response against an expected standard response code.    */
/*--------------------------------------------------------------------*/

static KWNetStatus
SendSMTPCmdCheckReply(
  char        *cmd,        /* IN The command to send                 */
  int          expected,   /* IN The reply waited for                */
  unsigned int timeout     /* IN Command timeout period              */
)
{
  int     rep;
  KWNetStatus status;

  /* Send command to SMTP host */
  status = SendSMTPCmd(cmd);

  if (status != KWNSSuccess)
    return status;

  /* Get and check reply */
  status = GetSMTPReply(timeout, &rep);

  if (status != KWNSSuccess)
    return status;

  if (rep != expected)
  {
      printmsg(0, "SendSMTPCmdCheckReply: Wanted response %d, "
                  "but received %d for command: %.80s",
                  expected,
                  rep,
                  cmd);

      switch(rep / 100)
      {
         case 4:
            /* Please try later ... */
            return KWNSTransientError;

         case 5:
            /* GONG! */
            return KWNSPermanentError;

         case 2:
            /* Hmmm, we may just be confused as to the protocol */
            return KWNSWrongReply;

         default:
            /* Hmmm, what could it be? */
            return KWNSInternalError;

      } /* switch(rep / 100) */

  } /* if (rep != expected) */

  return KWNSSuccess;

} /* SendSMTPCmdCheckReply */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P M a i l C m d                                */
/*                                                                    */
/*       Format sender address and send it to the remote SMTP         */
/*       server                                                       */
/*--------------------------------------------------------------------*/

static KWNetStatus
SendSMTPMailCmd(
   IMFILE *imf,                  /* IN message text for bounce msg   */
   const MAIL_ADDR *sender,      /* IN sender for bounce msg         */
   const char *bounceAddress,    /* IN Address to report in bounces  */
   KWBoolean validate,           /* IN control for bounce processing */
   unsigned int timeout          /* IN seconds to wait for reply     */
)
{
   char buffer[MAXADDR + 100];
   char    *s;

/*--------------------------------------------------------------------*/
/*       Format the sender address; we try to use what we were        */
/*       presented, and rewrite it as user@host if it the host if     */
/*       needed.  If it is NOT an FQDN, we will rewrite it as         */
/*       gatewayed via us (yuck).                                     */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(4,"ConnectSMTP: Sender is %s (%s at %s via %s)",
            sender->address,
            sender->user,
            sender->host,
            (sender->relay == NULL) ? E_nodename : sender->relay);
#endif

   strcpy(buffer, "MAIL From: ");
   s = buffer + strlen(buffer);

   /* Reformat as RFC-822 only if needed */
   if (equal(sender->address , "<>")       ||
       equali(sender->user, "mailer-daemon") ||
       equali(sender->user, "uucp"))
      strcpy(s, "<>");            /* Special postmaster address   */
   else if (strchr(sender->address , '@') != NULL)
      sprintf(s, "<%s>", sender->address);
   else if (strchr(sender->host, '.') != NULL)
      sprintf(s, "<%s@%s>", sender->user, sender->host);
   else
      sprintf(s, "<%s@%s>", sender->address, E_domain);

/*--------------------------------------------------------------------*/
/*                 Actually send MAIL From: command                   */
/*--------------------------------------------------------------------*/

   return SendSMTPCmdCheckReply(buffer,
                                250,
                                E_timeoutSMTP * 2);


} /* SendSMTPMailCmd */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P A d d r e s s C m d                          */
/*                                                                    */
/*       Send a single receiver SMTP address to remote server.        */
/*       Automatically requests requeue or bounce of bad addresses    */
/*--------------------------------------------------------------------*/

static KWNetStatus
SendSMTPAddressCmd(
   IMFILE *imf,                  /* IN message text for bounce msg   */
   const MAIL_ADDR *sender,      /* IN sender for bounce msg         */
   SMTP_ADDR *addressee,         /* IN target addr to send to server */
   KWBoolean validate,           /* IN control for bounce process    */
   unsigned int timeout          /* IN seconds to wait for reply     */
)
{
   static const char pattern[] = "RCPT To: <%s>";
   char buffer[MAXADDR + 100];
   char *errorType = NULL;
   KWNetStatus status;
   int rep;

   /* RMAIL requires empty addresses be handled silently */
   if (! strlen(addressee->address))
   {
      addressee->processed = KWTrue;
      return KWNSPermanentError;
   }

/*--------------------------------------------------------------------*/
/*              Format and send the target address command            */
/*--------------------------------------------------------------------*/

   sprintf(buffer, pattern, addressee->address);

   status = SendSMTPCmd(buffer);

   if (status != KWNSSuccess)
      return status;

/*--------------------------------------------------------------------*/
/*            Request and verify we did get a server response         */
/*--------------------------------------------------------------------*/

   status = GetSMTPReply(timeout, &rep);

   if (status != KWNSSuccess)
      return status;

/*--------------------------------------------------------------------*/
/*       We have a valid command response, parse it further           */
/*--------------------------------------------------------------------*/

   switch (rep / 100)
   {
      case 2:
         if (rep != 250)
            printmsg(0,"Warning: Unexpected positive return code %d for address %s",
                        rep,
                        addressee->address);
         return KWNSSuccess;

      case 4:
         errorType = "Transient";
         status = KWNSTransientError;
         break;

      case 5:
         errorType = "Permanent";
         status = KWNSPermanentError;
         break;

      default:
         errorType = "Unknown";
         status = KWNSInternalError;
         break;

   } /* else switch */

/*--------------------------------------------------------------------*/
/*                     Report the error we received                   */
/*--------------------------------------------------------------------*/

   printmsg(0, "%s failure %d of address %s",
               errorType,
               rep,
               addressee->address);

   if (status != KWNSTransientError)
   {
      size_t  bufferStatus = SAVE_TRACE_BUFFER();
      sprintf(buffer, "%s SMTP procotol error %d",
                       errorType,
                       rep);

      bufferTrace("\n***", buffer, 0);
      Bounce(imf,
             sender,
             "Remote SMTP server rejected recipient address",
             GET_TRACE_BUFFER(),
             addressee->address,
             validate);
      RESTORE_TRACE_BUFFER(bufferStatus);
      addressee->processed = KWTrue;

   } /* if (status != KWNSTransientError) */
   else
      addressee->retry = KWTrue;

   return status;

} /* SendSMTPAddressCmd */

/*--------------------------------------------------------------------*/
/*       i s A l l P e r i o d s                                      */
/*                                                                    */
/*       Determine if a character array of specified length is all    */
/*       periods.                                                     */
/*--------------------------------------------------------------------*/

static KWBoolean
isAllPeriods(char *s, int len)
{
   int column;

   for (column = 0; column < len; column++)
   {
      if (s[column] != '.')
         return KWFalse;
   }

   return KWTrue;

} /* allPeriods */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P D a t a                                      */
/*                                                                    */
/*       Send the contents of the SMTP message to the remote          */
/*--------------------------------------------------------------------*/

static KWNetStatus
SendSMTPData(
   IMFILE *imf                   /* IN Contents of message            */
)
{


   char dataBuf[MAXPACK+CRLF_LEN];
   int used = 0;
   int len;

   imrewind(imf);

/*--------------------------------------------------------------------*/
/*                      Outer loop handles input                      */
/*--------------------------------------------------------------------*/

   while((len = (int) imread(dataBuf + used,
                             1,
                             (sizeof dataBuf - CRLF_LEN - (size_t) used),
                             imf)) != 0)
   {
     char *start = dataBuf;
     char *eol;
     KWBoolean firstPass = KWTrue;

     len += used;                   /* Make total amount buffered    */

/*--------------------------------------------------------------------*/
/*          Inner loop handles output from buffer to network          */
/*--------------------------------------------------------------------*/

     while(len &&
            ((eol = memchr(start, '\n', (size_t) len)) != NULL))
     {
         int lineLength = eol - start;

         firstPass = KWFalse;

         /* Trace our write */
         *eol = '\0';
         printmsg(5, "--> %.75s", start);

         /* Write data line if not empty; we tack CR/LF on by hand */
         if (lineLength)
         {
            char saveByte1, saveByte2;
            KWBoolean periodQuoted = isAllPeriods(start, lineLength);

            /* If data on line consists of periods, quote it. */
            if (periodQuoted)
            {
               if (start > dataBuf) /* Room to insert quote period?  */
               {                    /* Yes --> Include it into buff  */
                  *--start = '.';
                  lineLength++;
               }
               else if  (swrite(".", 1) < 1)
               {
                  printmsg(0, "SendSMTPData of leading period failed.");
                  return KWNSNoNet;
               }
               else
                  periodQuoted = KWFalse;

            } /* if (periodQuoted) */

            /* Save bytes we need to overlay for cr/lf */
            saveByte1 = start[lineLength];
            start[lineLength++] = '\r';

            saveByte2 = start[lineLength];
            start[lineLength++] = '\n';

            if (swrite(start, (size_t) lineLength) < lineLength)
            {
                printmsg(0, "SendSMTPData of %d bytes failed: %.80s",
                           lineLength,
                           start);
                return KWNSNoNet;
            }

            /* Restore overlayed bytes */
            start[--lineLength] = saveByte2;
            start[--lineLength] = saveByte1;

            /* Also restore pointers from quoting */
            if (periodQuoted)
            {
               start++;
               lineLength--;
            }

         } /* if (lineLength) */
         else if (swrite(CRLF, CRLF_LEN) < CRLF_LEN) /* Write empty line */
         {
             printmsg(0, "SendSMTPData of CR/LF failed.");
             return KWNSNoNet;
         }

         /* Update lengths and actual date buffer contents */
         len -= lineLength + 1;  /* Add in LF not sent to remote     */
         start = eol + 1;        /* Start next search at new line    */

         if ((len > sizeof dataBuf) || (len < 0))
         {
            printmsg(0,"Length has gone wild (%d), line was %d",
                        len,
                        lineLength);
            panic();
         }

     } /* while((eol = memchr(start, '\n', len)) != NULL) */

/*--------------------------------------------------------------------*/
/*       Handle the special case of an overlength line; we can        */
/*       only send it if it's not all periods (we would fail to       */
/*       quote the line properly if it were).                         */
/*--------------------------------------------------------------------*/

     if (firstPass && !isAllPeriods(dataBuf, len))
     {
        printmsg(5, "--> %.75s", dataBuf);

        if (swrite(dataBuf, len) < len)
        {
           printmsg(0, "SendSMTPData of CR/LF failed.");
           return KWNSNoNet;
        }

        len = 0;
        firstPass = KWFalse;

     } /* if (firstPass && !isAllPeriods(dataBuf, len)) */

/*--------------------------------------------------------------------*/
/*        Verify the input buffer had at least one valid line         */
/*--------------------------------------------------------------------*/

     if (firstPass)
     {
        printmsg(0,"SendSMTPData: Overlength input line "
                   "(%d bytes, buffer %d) not trapped",
                   used,
                   sizeof dataBuf);
        panic();
     }

     /* Burp remaining data to front of buffer */
     used = len;

     if (used)
        memmove(dataBuf, start, (size_t) used);

   } /* while((len = imread()) */

/*--------------------------------------------------------------------*/
/*     Verify that RMAIL previously terminated final line for us      */
/*--------------------------------------------------------------------*/

   if (used)
   {
      printmsg(0,"SendSMTPData: Unterminated final line not trapped: %.80s",
               dataBuf);
      panic();
   }

   return KWNSSuccess;

} /* SendSMTPData */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P P e r i o d                                  */
/*                                                                    */
/*       Send period which terminates the SMTP mail transmission and  */
/*       check results for the resulting mail delivery                */
/*--------------------------------------------------------------------*/

static int
SendSMTPPeriod(
   IMFILE *imf,                  /* Temporary input file             */
   const MAIL_ADDR *sender,      /* Originating (error) address      */
   const char *bounceAddress,    /* IN Address to report in bounces  */
   const char *relay,            /* IN relay for bounce processing   */
   const KWBoolean validate,     /* Perform bounce on failure        */
   const unsigned int timeout
)
{
   static const char mName[] = "SendSMTPPeriod";
   KWNetStatus status;

   status = SendSMTPCmdCheckReply(".", 250, timeout);

/*--------------------------------------------------------------------*/
/*       Unlike most SMTP commands, we check errors directly; we      */
/*       bounce messages here where the standard routine might        */
/*       requeue, because we don't want transmit the same message     */
/*       over and over.                                               */
/*--------------------------------------------------------------------*/

   switch(status)
   {
      case KWNSSuccess:
         break;

      case KWNSTransientError:
         printmsg(0,"%s: Transient Error processing SMTP DATA command",
                  mName);
         /* Unprocessed addresses will be retried */
         break;

      case KWNSNetTimeout:
         bufferTrace("\n***","Timeout during SMTP data transmission",0);
         Bounce(imf,
                sender,
                GET_TRACE_BUFFER(),
                relay,
                bounceAddress,
                validate);
         return status;
         break;

      case KWNSNoNet:
         bufferTrace("\n***","SMTP Network Connection Lost",0);
         Bounce(imf,
                sender,
                GET_TRACE_BUFFER(),
                relay,
                bounceAddress,
                validate);

         break;

      default:
         bufferTrace("\n***",
                    "General error during SMTP procotol exchange",
                    0);
         Bounce(imf,
                sender,
                GET_TRACE_BUFFER(),
                relay,
                bounceAddress,
                validate);
         break;

   } /* switch(status) */

   return status;

} /* SendSMTPPeriod */

/*--------------------------------------------------------------------*/
/*       s h u t d o w n S M T P                                      */
/*                                                                    */
/*       Terminate SMTP connection for specified client               */
/*--------------------------------------------------------------------*/

static void
shutdownSMTP(
               unsigned int timeout    /* IN timeout for reply in secs */
)
{
   if (CD() && timeout)
      SendSMTPCmdCheckReply("QUIT", 221, timeout);

   if (CD())
      hangup();

   closeline();

} /* shutdownSMTP */

/*--------------------------------------------------------------------*/
/*       A u t o B o u n c e S M T P                                  */
/*                                                                    */
/*       Handle error conditions in most commands                     */
/*--------------------------------------------------------------------*/

static KWBoolean
AutoBounceSMTP(
   int   status,
   IMFILE *imf,
   const MAIL_ADDR *sender,
   SMTP_ADDR *toAddress,
   int count,
   const char *relay,
   const KWBoolean validate,
   const unsigned int timeout
)
{
   int subscript;

   switch(status)
   {
      case KWNSSuccess:
         /* No problem, so do not force termination */
         return KWFalse;

      case KWNSNoNet:
         printmsg(0,"Network connection lost with %s"
                    ", requeuing for retry.",
                    relay);
         shutdownSMTP(0);
         return KWTrue;

      case KWNSNetTimeout:
         printmsg(0,"Network timeout occurred, breaking off exchange with %s"
                    " and requeuing for retry.",
                    relay );
         shutdownSMTP(timeout);
         return KWTrue;

      case KWNSTransientError:
         printmsg(0,"Transient procotol error with %s"
                     ", requeuing for retry.",
                     relay);
         shutdownSMTP(timeout);
         return KWTrue;

      default:
         Bounce(imf,
                sender,
                "SMTP Protocol Permanent Error occurred",
                GET_TRACE_BUFFER(),
                relay,
                validate);
       /* Treat all bounced addresses as processed */
       for (subscript = 0; subscript < count; subscript++)
             toAddress[subscript].processed = KWTrue;
         return KWTrue;

   } /* switch(status) */

} /* SMTPAutoBounce */

/*--------------------------------------------------------------------*/
/*       C o n n e c t S M T P                                        */
/*                                                                    */
/*       High level protocol driver for SMTP protocol exchange.       */
/*--------------------------------------------------------------------*/

static void
ConnectSMTP(
   IMFILE *imf,                     /* Temporary input file          */
   const MAIL_ADDR *sender,         /* Originating (error) address   */
   const char *relay,               /* SMTP host to connect to       */
   SMTP_ADDR *toAddress,            /* List of target addressess     */
   int count,                       /* Number of addresses to send   */
   const KWBoolean validate         /* Perform bounce on failure     */
)
{
   static const char mName[] = "ConnectSMTP";
   char    buffer[KW_BUFSIZ];
   int     rep;
   int     subscript = 0;
   size_t  successes = 0;
   char    *bounceAddress = "(not available)";
   KWNetStatus status;

   if (! chooseCommunications(SUITE_TCPIP, KWTrue, NULL))
   {
      printmsg(0,"%s: Cannot select %s communications suite",
                 mName,
                 SUITE_TCPIP);
      panic();
   }

   /* Intialize our SMTP trace buffer */
   traceUsed = 0;
   sprintf(buffer, "Connecting to SMTP relay %s\n", relay);
   bufferTrace("\n\n\t...", buffer, 0);

   setTitle("%s SMTP connection in progress", relay);

   /* Enable our disk trace log, if desired */
   if (debuglevel >= 5)
      traceEnabled = KWTrue;

  /* Connect to SMTP host (exit on error) */
  if (activeopenline((char *) relay, SMTP_PORT_NUMBER, KWFalse))
     return;                      /* Deliver via alt method        */

   if (count == 1)
   {
     bounceAddress = toAddress[0].address;
     setTitle("Sending mail to %s via %s",
              bounceAddress,
              relay);
   }

/*--------------------------------------------------------------------*/
/*                Get host greeting and respond to it                 */
/*--------------------------------------------------------------------*/

   /* Get initial banner from the remote */
   status = GetSMTPReply(E_timeoutSMTP * 2, &rep);

   if (AutoBounceSMTP(status, imf, sender, toAddress, count, relay, validate, E_timeoutSMTP))
      return;

   if (rep != 220)
   {
      printmsg(0, "Connection wanted 220, received %d", rep);
      shutdownSMTP(E_timeoutSMTP);
      return;
   }

   /* Okay, respond to greeting with HELO of our own */
   sprintf(buffer, "HELO %s", E_domain);
   status = SendSMTPCmdCheckReply(buffer, 250, E_timeoutSMTP);

   if (AutoBounceSMTP(status, imf, sender, toAddress, count, relay, validate, E_timeoutSMTP))
      return;

/*--------------------------------------------------------------------*/
/*                  Tell the server the sender address                */
/*--------------------------------------------------------------------*/

   status = SendSMTPMailCmd(imf,
                            sender,
                            bounceAddress,
                            validate,
                            E_timeoutSMTP * 2);

   if (AutoBounceSMTP(status, imf, sender, toAddress, count, relay, validate, E_timeoutSMTP))
      return;

/*--------------------------------------------------------------------*/
/*                Send as many RCPT To: lines as we need              */
/*--------------------------------------------------------------------*/

   for (subscript = 0; subscript < count; subscript++)
   {
      if (SendSMTPAddressCmd(imf,
                             sender,
                             &toAddress[subscript],
                             validate,
                             E_timeoutSMTP * 2) == KWNSSuccess)
          successes ++;
   }

   /* If no valid addresses passed, don't try to send the data */
   if (successes == 0)
   {
      printmsg(0,"No valid addresses, aborting SMTP transaction");
      SendSMTPCmdCheckReply("RSET", 250, E_timeoutSMTP);
      shutdownSMTP(E_timeoutSMTP);
      return;
   }

/*--------------------------------------------------------------------*/
/*                  Prepare to send the actual message                */
/*--------------------------------------------------------------------*/

   status = SendSMTPCmdCheckReply("DATA", 354, E_timeoutSMTP);

   if (AutoBounceSMTP(status, imf, sender, toAddress, count, relay, validate, E_timeoutSMTP))
      return;

/*--------------------------------------------------------------------*/
/*                       Actually send the data                       */
/*--------------------------------------------------------------------*/

  setTitle("Sending data via %s",  relay);

  status = SendSMTPData(imf);

   if (AutoBounceSMTP(status, imf, sender, toAddress, count, relay, validate, E_timeoutSMTP))
      return;

/*--------------------------------------------------------------------*/
/*       Termination transmission of the message and check result     */
/*--------------------------------------------------------------------*/

   status = SendSMTPPeriod(imf,
                           sender,
                           bounceAddress,
                           relay,
                           validate,
                           E_timeoutSMTP * 2);

   if (status == KWNSSuccess)
   {
       printmsg(0, "%s: Delivered %ld byte message from %s "
                   "to %ld addresses via relay %s",
                    mName,
                   imlength(imf),
                   sender->address,
                   successes,
                   relay);

       for (subscript = 0; subscript < count; subscript++)
       {
          /* Don't requeue any address which made it this far */
          if (!toAddress[subscript].retry)
             toAddress[subscript].processed = KWTrue;
       }
   }

/*--------------------------------------------------------------------*/
/*                   Clean up after our connection                    */
/*--------------------------------------------------------------------*/

  /* Terminate */
  shutdownSMTP(E_timeoutSMTP);

} /* ConnectSMTP */
