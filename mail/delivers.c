/*--------------------------------------------------------------------*/
/*       d e l i v e r s . C                                          */
/*                                                                    */
/*       SMTP delivery UUPC/extended; ported into UUPC/extended       */
/*       from original stand alone SMTPSEND                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       MODULE SMTPSEND : Send a mail to a host in SMTP protocol.   $*/
/*--------------------------------------------------------------------*/
/*       This code is placed in the public domain by its author to    */
/*       be used as you wish.  However, if you add neat new           */
/*       features, I'd appreciate having a copy sent to me at         */
/*       jack.rebiscoul@ssti.fr.                                      */
/*--------------------------------------------------------------------*/
/*       AUTHOR: Jacques REBISCOUL S.S.T.I.  jack.rebiscoul@ssti.fr   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1997 by Kendra Electronic         */
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
 *       $Id: delivers.c 1.8 1997/12/14 00:43:16 ahd Exp $
 *
 *       $Log: delivers.c $
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
#include "../uucico/commlib.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

RCSID("$Id: delivers.c 1.8 1997/12/14 00:43:16 ahd Exp $");

#define SMTP_PORT_NUMBER 25

char SMTPRecvBuffer[BUFSIZ]; /* Public to allow printing      */

KWBoolean suspend_processing = KWFalse;
                             /* Dummy to replace copy in suspend.c   */

/*--------------------------------------------------------------------*/
/*    D e l i v e r S M T P                                           */
/*                                                                    */
/*    Perform control processing for delivery to another UUCP node    */
/*--------------------------------------------------------------------*/

size_t DeliverSMTP( IMFILE *imf,          /* Input file name          */
                    const MAIL_ADDR *sender,
                    const char *targetAddress,
                    const char *path)
{

   static char *savePath = NULL;    /* System we previously queued for*/
   static char *addrList[50];
   static int subscript = 0;
   static int addressMax = sizeof addrList / sizeof addrList[0];

/*--------------------------------------------------------------------*/
/*            Flush previously queued addresses, if needed            */
/*--------------------------------------------------------------------*/

   if (subscript)
   {
      KWBoolean queueNow = KWFalse;

      if ( path == NULL )
         queueNow = KWTrue;
      else if ( ! equal(savePath, path))
         queueNow = KWTrue;
      else if ( subscript >= addressMax )
         queueNow = KWTrue;

/*--------------------------------------------------------------------*/
/*                We need to actually perform delivery                */
/*--------------------------------------------------------------------*/

      if ( queueNow )
      {
         KWBoolean noConnect = KWFalse;

         if ( ! ConnectSMTP( imf,
                             sender,
                             savePath,
                             addrList,
                             subscript,
                             KWTrue ) )
               noConnect = KWTrue;

         while( subscript-- > 0 )
         {
            /* Queue failed SMTP mail for local node for retry */
            if ( noConnect )
               DeliverRemote( imf,
                              sender,
                              addrList[subscript],
                              E_nodename );

            free( addrList[subscript] );
         }

         subscript = 0;

      } /* if ( queueNow && subscript ) */

   } /* if (savePath != NULL) */

/*--------------------------------------------------------------------*/
/*                Return if we only flushing the cache                */
/*--------------------------------------------------------------------*/

   if ( path == NULL )
      return 0;

/*--------------------------------------------------------------------*/
/*               Report and queue the current delivery                */
/*--------------------------------------------------------------------*/

   printmsg(1,"Queuing SMTP mail %sfrom %s to %s via %s",
               formatFileSize( imf ),
               sender->address,
               targetAddress,
               path);

   savePath = newstr( path );
   addrList[subscript] = strdup( targetAddress );
   checkref( addrList[subscript] );
   subscript++;

   return 1;

} /* DeliverSMTP */

/*--------------------------------------------------------------------*/
/* ROLE Send a command to sendmail.                                  $*/
/*--------------------------------------------------------------------*/

static KWBoolean
SendSMTPCmd(
  char        *cmd      /* IN The command to send $*/
)
{
   char buffer[BUFSIZ];
   int len = (int) strlen(cmd);
   KWBoolean buffered = KWFalse;

  /* PSEUDO Send command to SMTP host $*/
  printmsg(2,">>> %.75s", cmd);

  if (sizeof buffer > (len + 3))
  {
     strcpy(buffer, cmd);
     strcat(buffer, "\r\n");
     len += 2;
     cmd = buffer;
     buffered = KWTrue;
  }

  if(!swrite(cmd, strlen(cmd)))
  {
     printmsg(0,"Error sending command to remote host");
     return KWFalse;
  }

  if(!buffered && !swrite("\r\n", 2) < 2)
  {
    printmsg(0, "Error sending CR/LF to remote host");
    return KWFalse;
  }

  return KWTrue;

} /* SendSMTPCmd */

/*--------------------------------------------------------------------*/
/* ROLE Gets a line (linefeed terminated) from SMTP host              */
/*  .PP returns pointer to buffer or NULL if error/connection lost.  $*/
/*--------------------------------------------------------------------*/

static char*
GetsSMTP(
  char          *buf,   /* IN Buffer in which to store $*/
  int           len     /* IN Number max of bytes in buffer $*/
)
{
  int    i;

  /* PSEUDO Read until newline, EOF or buffer full $*/
  for(i=0; i <(len-1); i++)
  {
    if (! sread(buf + i, 1, E_timeoutSMTP))
    {
       printmsg(0, "GetsSMTP: Read timeout after %d seconds",
                 E_timeoutSMTP);
       return NULL;
    }

    /* After we have at least two characters, check for CR/LF pair */
    if (i && ! memcmp(buf + i - 1, "\r\n", 2))
    {
        buf[i-1] = '\0';
        return buf;
    }
  }

  buf[len - 1] = '\0';
  return buf;

} /* GetsSMTP */

/*--------------------------------------------------------------------*/
/* ROLE Get a reply from the SMTP host.                               */
/*  .PP returns reply code or -1 if error or EOF                     $*/
/*--------------------------------------------------------------------*/

static int
GetSMTPReply(void)
{

  /* PSEUDO Get line $*/
  /* To support multi-line responses, we check - */
  do {
    if(GetsSMTP(SMTPRecvBuffer, sizeof(SMTPRecvBuffer)) == NULL)
      return -1;

    printmsg(2, "<<< %.75s", SMTPRecvBuffer);
  } while (SMTPRecvBuffer[3] == '-');

  /* PSEUDO Convert reply in code $*/
  return atoi(SMTPRecvBuffer);
}

/*--------------------------------------------------------------------------*/
/* ROLE Present address to remote SMTP host                                 */
/*  .PP returns KWTrue if okay, else bounces message and returns KWFalse   $*/
/*--------------------------------------------------------------------------*/

static KWBoolean
SendSMTPAddressCmd(
   IMFILE *imf,
   const MAIL_ADDR *sender,
   const char *address,
   KWBoolean validate
)
{
   static const char pattern[] = "RCPT To: <%s>";
   char buf[MAXADDR + sizeof pattern];
   char *errorType = NULL;
   int rep;

   /* RMAIL requires empty addresses be handled silently */
   if (! strlen(address))
      return KWFalse;

   sprintf(buf, pattern, address);

   if (! SendSMTPCmd(buf))
      return KWFalse;

   rep = GetSMTPReply();

   if (rep == 250)
      return KWTrue;

   switch (rep / 100)
   {
      case 2:
         printmsg(1,"Unexpected positive return code %d for address %s",
                    rep,
                    address);
         return KWTrue;

      case 4:
         errorType = "Transient";
         break;

      case 5:
         errorType = "Permanent";
         break;

      default:
         errorType = "Unknown";
         break;

   } /* else switch */

/*--------------------------------------------------------------------*/
/*                     Report the error we received                   */
/*--------------------------------------------------------------------*/

   printmsg(0, "%s failure %d of address %s",
               errorType,
               rep,
               address);

   sprintf(buf, "%s error with code %d", errorType, rep);

   Bounce(imf,
          sender,
          "Remote server error during SMTP delivery",
          buf,
          address,
          validate);

   return KWFalse;

} /* SendSMTPAddressCmd */

/*--------------------------------------------------------------------*/
/* ROLE Send a command to sendmail and get a reply.                   */
/*  .PP Return reply code or -1 if error or EOF                      $*/
/*--------------------------------------------------------------------*/

static KWBoolean
SendSMTPCmdCheckReply(
  char        *cmd,        /* IN The command to send $*/
  int          expected    /* IN The reply waited for $*/
)
{
  int     rep;

  /* PSEUDO Send command to SMTP host $*/
  if (! SendSMTPCmd(cmd))
    return KWFalse;

  /* PSEUDO Get and check reply $*/
  if((rep = GetSMTPReply()) != expected)
  {

    printmsg(0, "SendSMTPCmdCheckReply: Wanted response %d, "
                 "but received %d for command: %.80s",
              expected,
              rep,
              cmd);
    return KWFalse;
  }

  return KWTrue;

} /* SendSMTPCmdCheckReply */

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
   for ( column = 0; column < len; column++ )
   {
      if ( s[column] != '.' )
         return KWFalse;
   }

   return KWTrue;

} /* allPeriods */

/*--------------------------------------------------------------------*/
/*       S e n d S M T P D a t a                                      */
/*                                                                    */
/*       Send the contents of the SMTP message to the remote          */
/*--------------------------------------------------------------------*/

static KWBoolean
SendSMTPData(
   IMFILE *imf                   /* Contents of message            */
)
{

#define CRLF_LEN  2

   char dataBuf[MAXPACK+CRLF_LEN];
   int used = 0;
   int len;

   /* PSEUDO Transfer DATA $*/
   if (!SendSMTPCmdCheckReply("DATA", 354))
      return KWFalse;

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

     len += used;                   /* Make total amount buffered    */

/*--------------------------------------------------------------------*/
/*          Inner loop handles output from buffer to network          */
/*--------------------------------------------------------------------*/

     while(len &&
            ((eol = memchr(start, '\n', (size_t) len)) != NULL))
     {
         int lineLength = eol - start;

         /* Trace our write */
         *eol = '\0';
         printmsg(5, "--> %.75s", start);

         /* Write data line if not empty; we tack CR/LF on by hand  */
         if (lineLength)
         {
            char save1, save2;
            KWBoolean periodQuoted = isAllPeriods(start, lineLength);

            /* If data on line consists of periods, quote it. */
            if (periodQuoted)
            {
               if (start > dataBuf) /* Room to insert quote period?  */
               {                    /* Yes --> Include it into buff  */
                  *--start = '.';
                  lineLength++;
               }
               else if  (! swrite(".", 1))
               {
                  printmsg(0, "SendSMTPData of leading period failed.");
                  return KWFalse;
               }
               else
                  periodQuoted = KWFalse;

            } /* if (periodQuoted) */

            /* Save bytes we need to overlay for cr/lf */
            save1 = start[lineLength];
            start[lineLength++] = '\r';

            save2 = start[lineLength];
            start[lineLength++] = '\n';

            if (! swrite(start, (size_t) lineLength))
            {
                printmsg(0, "SendSMTPData of %d bytes failed: %.80s",
                           lineLength,
                           start);
                return KWFalse;
            }

            /* Restore overlayed bytes */
            start[--lineLength] = save2;
            start[--lineLength] = save1;

            /* Also restore pointers from quoting */
            if (periodQuoted)
            {
               start++;
               lineLength--;
            }

         } /* if (lineLength) */
         else if (! swrite("\r\n", CRLF_LEN)) /* Write empty line    */
         {
             printmsg(0, "SendSMTPData of CR/LF failed.");
             return KWFalse;
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
/*        Verify the input buffer had at least one valid line         */
/*--------------------------------------------------------------------*/

     if (used == sizeof dataBuf)
     {
        printmsg(0,"SendSMTPData: Overlength input line not trapped");
        panic();
     }

     /* Burp remaining data to front of buffer  */
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

/*--------------------------------------------------------------------*/
/*        Send command to return data mode and get out of here        */
/*--------------------------------------------------------------------*/

   if (! SendSMTPCmdCheckReply(".", 250))
      return KWFalse;

   return KWTrue;

} /* SendSMTPData */

/*--------------------------------------------------------------------*/
/*       s h u t d o w n S M T P                                      */
/*                                                                    */
/*       Terminate SMTP connection for specified client               */
/*--------------------------------------------------------------------*/

static void
shutdownSMTP(void)
{
   if (CD())
      SendSMTPCmdCheckReply("QUIT", 221);

   if (CD())
      hangup();

   closeline();

} /* shutdownSMTP */

/*--------------------------------------------------------------------*/
/* ROLE Main program to send a mail in SMTP protocol.                $*/
/*--------------------------------------------------------------------*/

size_t
ConnectSMTP(
   IMFILE *imf,                     /* Temporary input file          */
   const MAIL_ADDR *sender,         /* Originating (error) address   */
   const char *relay,               /* SMTP host to connect to       */
   const char **toAddress,          /* List of target addressess     */
   int count,                       /* Number of addresses to send   */
   const KWBoolean validate         /* Perform bounce on failure     */
)
{
   static const char mName[] = "ConnectSMTP";
   char    buf[BUFSIZ];
   int     rep;
   int     subscript = 0;
   size_t  successes = 0;

  if (! chooseCommunications(SUITE_TCPIP, KWTrue, NULL))
      return 0;

  if (debuglevel >= 5)
     traceEnabled = KWTrue;

  /* PSEUDO Connect to SMTP host (exit on error) $*/
  if (activeopenline((char *) relay, SMTP_PORT_NUMBER, KWFalse))
     return 0;                      /* Deliver via alt method        */

/*--------------------------------------------------------------------*/
/*                Get host greeting and respond to it                 */
/*--------------------------------------------------------------------*/

  if((rep = GetSMTPReply()) != 220)
  {
    printmsg(0, "Connection wanted 220, received %d", rep);
    shutdownSMTP();
    return 0;
  }

  sprintf(buf, "HELO %s (%s %s SMTP client) ",
                 E_domain,
                 compilep,
                 compilev);

  if (! SendSMTPCmdCheckReply(buf, 250))
  {
     printmsg(0, "HELO wanted 250, received %d", rep);
     shutdownSMTP();
     return 0;
  }

/*--------------------------------------------------------------------*/
/*       Handle the sender address command; we try to use what we     */
/*       were presented, and rewrite it as user@host if it the        */
/*       host if needed.  If it is NOT an FQDN, we're rewrite it      */
/*       as gatewayed via us (yuck).                                  */
/*--------------------------------------------------------------------*/

  /* PSEUDO Send MAIL From: $*/

#ifdef UDEBUG
   printmsg( 4,"ConnectSMTP: Sender is %s (%s at %s via %s)",
            sender->address,
            sender->user,
            sender->host,
            (sender->relay == NULL) ? "*local*" : sender->relay );
#endif

   /* Reformat as RFC-822 only if needed */
   if (strchr( sender->address , '@' ) != NULL )
      sprintf(buf, "MAIL From: <%s>", sender->address );
   else if (strchr(sender->host, '.' ) != NULL)
      sprintf(buf, "MAIL From: <%s@%s>", sender->user, sender->host );
   else
      sprintf(buf, "MAIL From: <%s@%s>", sender->address, E_domain );

  if (! SendSMTPCmdCheckReply(buf, 250))
  {
     shutdownSMTP();
     return 0;
  }

  /* PSEUDO Send RCPT To: $*/

  for (subscript = 0; subscript < count; subscript++)
  {
     if (SendSMTPAddressCmd(imf,
                            sender,
                            toAddress[subscript],
                            validate))
        successes ++;
  }

  if (successes)                    /* At least one receiver?        */
  {
      if (SendSMTPData(imf))         /* yes --> Transmit the message  */
      {
          printmsg(0, "%s: Delivered %ld byte message from %s "
                      "to %ld addresses via relay %s",
                      mName,
                      imlength( imf ),
                      sender->address,
                      successes,
                      relay );
      }
      else {
          Bounce(imf,
                 sender,
                 "Data transmission error during SMTP delivery",
                 relay,
                 "*unknown*",
                 validate);
      }
  }
  else {
      SendSMTPCmdCheckReply("RSET", 250);    /* no --> Abort send    */
      successes = 1;                /* Avoid retries                 */
  }

/*--------------------------------------------------------------------*/
/*                   Clean up after our connection                    */
/*--------------------------------------------------------------------*/

  /* PSEUDO Terminate $*/
  shutdownSMTP();

  return successes;

} /* ConnectSMTP */
