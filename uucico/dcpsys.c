/*--------------------------------------------------------------------*/
/*    d c p s y s . c                                                 */
/*                                                                    */
/*    System support functions for UUCICO                             */
/*                                                                    */
/*    Changes Copyright (c) 1989-2001 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*                                                                    */
/*    Copyright (c) Richard H. Lamb 1985, 1986, 1987                  */
/*    Changes Copyright (c) Stuart Lynne 1987                         */
/*                                                                    */
/* Updated:                                                           */
/*                                                                    */
/*    13May89  - Modified checkname to only examine first token of    */
/*               name.                                                */
/*               Modified rmsg to initialize input character before   */
/*               use.                                                 */
/*    16May89  - Moved checkname to router.c - ahd                    */
/*    17May89  - Wrote real checktime() - ahd                         */
/*    17May89  - Changed getsystem to return 'I' instead of 'G'       */
/*    25Jun89  - Added Reach-Out America to keyword table for         */
/*               checktime                                            */
/*    22Sep89  - Password file support for hosts                      */
/*    25Sep89  - Change 'ExpectStr' message to debuglevel 2           */
/*    01Jan90  - Revert 'ExpectStr' message to debuglevel 1           */
/*    28Jan90  - Alter callup() to use table driven modem driver.     */
/*               Add direct(), qx() procedures.                       */
/*    8 Jul90  - Add John DuBois's expectstr() routine to fix         */
/*               problems with long input buffers.                    */
/*    11Nov90  - Delete QX support, add ddelay, ssleep calls          */
/*    21Sep92  - Insure system system name and time do not crash      */
/*               UUCICO - from the original fix by Eugene             */
/*               Nesterenko, Moscow, Russia                           */
/*--------------------------------------------------------------------*/

/*
 *     $Id: dcpsys.c 1.58 2000/05/12 12:32:55 ahd v1-13g $
 *
 *     $Log: dcpsys.c $
 *     Revision 1.58  2000/05/12 12:32:55  ahd
 *     Annual copyright update
 *
 *     Revision 1.57  1999/01/08 02:20:56  ahd
 *     Convert currentfile() to RCSID()
 *
 *     Revision 1.56  1999/01/04 03:53:57  ahd
 *     Annual copyright change
 *
 *     Revision 1.55  1998/03/01 01:39:37  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.54  1997/11/24 02:58:14  ahd
 *     Don't allow non-mail programs to call checkname(), which could use
 *     uninitialized local domain name.
 *
 *     Revision 1.53  1997/04/24 01:33:53  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.52  1996/11/18 04:46:49  ahd
 *     Normalize arguments to bugout
 *     Reset title after exec of sub-modules
 *     Normalize host status names to use HS_ prefix
 *
 *     Revision 1.51  1996/03/18 03:52:46  ahd
 *     Allow binary rmsg() processing without translation of CR/LF into \0
 *     Use enumerated list for synch types on rmsg() call
 *
 *     Revision 1.50  1996/01/01 21:21:23  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.49  1995/04/02 00:01:39  ahd
 *     Correct processing to not send files below requested call grade
 *
 *     Revision 1.48  1995/03/11 15:49:23  ahd
 *     Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *     Revision 1.47  1995/02/12 23:37:04  ahd
 *     compiler cleanup, NNS C/news support, optimize dir processing
 *
 *     Revision 1.46  1995/01/30 04:08:36  ahd
 *     Additional compiler warning fixes
 *
 *     Revision 1.45  1995/01/14 14:08:59  ahd
 *     Change grade processing message
 *
 *     Revision 1.44  1995/01/09 12:35:15  ahd
 *     Correct VC++ compiler warnings
 *
 *     Revision 1.43  1995/01/07 16:38:39  ahd
 *     Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *     Revision 1.42  1994/12/31 03:39:56  ahd
 *     Correct processing for systems with grade restrictions in the
 *     systems file when calling system "any"
 *
 *     Revision 1.41  1994/12/27 20:45:50  ahd
 *     Smoother call grading'
 *
 *     Revision 1.40  1994/12/22 04:13:38  ahd
 *     Correct 't' protocol processing to use 512 messages with no header
 *
 *     Revision 1.39  1994/12/22 00:35:12  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.38  1994/02/20 19:11:18  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.37  1994/02/19  05:12:25  ahd
 * Use standard first header
 *
 * Revision 1.36  1994/02/13  04:46:01  ahd
 * Handle systems which only send seven characters of their names
 *
 * Revision 1.35  1994/01/24  03:04:16  ahd
 * Annual Copyright Update
 *
 * Revision 1.34  1994/01/01  19:19:30  ahd
 * Annual Copyright Update
 *
 * Revision 1.33  1993/12/02  13:49:58  ahd
 * 'e' protocol support
 *
 * Revision 1.32  1993/12/02  03:59:37  dmwatt
 * 'e' protocol support
 *
 * Revision 1.31  1993/11/14  20:51:37  ahd
 * Normalize internal speed for network links to 115200 (a large number)
 *
 * Revision 1.30  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.29  1993/10/28  12:19:01  ahd
 * Cosmetic time formatting twiddles and clean ups
 *
 * Revision 1.28  1993/10/12  01:33:59  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.27  1993/10/09  22:11:49  ahd
 * ANSIfy per rhg
 *
 * Revision 1.26  1993/10/07  23:04:13  ahd
 * Suppress unused variable
 *
 * Revision 1.25  1993/10/07  22:51:00  ahd
 * Suppress displaying login script on multiple lines
 *
 * Revision 1.24  1993/10/03  22:34:33  ahd
 * Alter format of numbers printed
 *
 * Revision 1.23  1993/09/29  04:52:03  ahd
 * Use additional state in support of suspend port code
 *
 * Revision 1.22  1993/09/28  01:38:19  ahd
 * Add configurable timeout for conversation start up phase
 *
 * Revision 1.21  1993/09/27  04:04:06  ahd
 * Normalize references to modem speed to avoid incorrect displays
 *
 * Revision 1.20  1993/09/27  00:48:43  ahd
 * Allow 't' protocol under 16 bit OS/2
 *
 * Revision 1.19  1993/09/21  01:42:13  ahd
 * Move declare of protocol list into source from header
 *
 * Revision 1.18  1993/09/20  04:48:25  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2)
 *
 * Revision 1.17  1993/08/26  05:00:25  ahd
 * Debugging code for odd failures on J. McBride's network
 *
 * Revision 1.16  1993/05/30  00:01:47  ahd
 * Multiple commuications drivers support
 *
 * Revision 1.15  1993/05/09  03:41:47  ahd
 * Make wmsg accept const string
 * Make sending/receiving of -x string to/from remote UUCICO optional
 * Delete rejection of high levels of debug for anonymous UUCICO's
 *
 * Revision 1.14  1993/05/06  03:41:48  ahd
 * Save true host name of caller in hostp->via field for use by
 * SYSLOG processing.
 *
 * Revision 1.13  1993/04/11  00:34:11  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.12  1993/04/05  04:35:40  ahd
 * Add timestamp, file size to directory information
 *
 * Revision 1.11  1993/01/23  19:08:09  ahd
 * Don't update system stats in sysend()
 *
 * Revision 1.10  1992/12/11  12:45:11  ahd
 * Shorten remote display to improve OS/2 windowed scrolling
 *
 * Revision 1.9  1992/12/01  04:37:03  ahd
 * Modify *nbstime call restrictions to make it less agressive
 *
 * Revision 1.8  1992/11/22  21:20:45  ahd
 * Use strpool for const string allocation
 *
 * Revision 1.7  1992/11/21  06:17:08  ahd
 * Transmit only one character in response to P (protocol) request
 *
 * Revision 1.6  1992/11/19  03:00:51  ahd
 * drop rcsid
 *
 * Revision 1.5  1992/11/18  03:49:21  ahd
 * Move check of call window to avoid premature lock file overhead
 *
 * Revision 1.4  1992/11/17  13:46:42  ahd
 * If host lookup fails, issue real error message, not malloc failure!
 *
 * Revision 1.3  1992/11/16  02:14:17  ahd
 * Initialize previous directory scanned variable in scandir
 *
 * Revision 1.2  1992/11/15  20:11:07  ahd
 * Clean up modem file support for different protocols
 *
 */

/*--------------------------------------------------------------------*/
/*                        system include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "checktim.h"
#include "dcp.h"
#include "dcpfpkt.h"
#include "dcpgpkt.h"
#include "dcptpkt.h"
#include "dcpepkt.h"
#include "dcplib.h"
#include "dcpsys.h"
#include "export.h"
#include "hostable.h"
#include "hostatus.h"
#include "modem.h"
#include "lock.h"
#include "uundir.h"
#include "ssleep.h"
#include "security.h"
#include "commlib.h"

RCSID("$Id: dcpsys.c 1.58 2000/05/12 12:32:55 ahd v1-13g $");

/*--------------------------------------------------------------------*/
/*                     Define available protocols                     */
/*--------------------------------------------------------------------*/

#if _MSC_VER >= 700
#pragma warning(disable:4121)   /* suppress packing-sensitivity warning */
#endif

typedef struct {
        char type;
        short (*getpkt)(char *data, short *len);
        short (*sendpkt)(char *data, short len);
        short (*openpk)(const KWBoolean caller);
        short (*closepk)(void);
        short (*rdmsg)(char *data);
        short (*wrmsg)(char *data);
        short (*eofpkt)(void);
        short (*filepkt)(const KWBoolean master, const unsigned long fileSize);
        KWBoolean network;
} Proto;

Proto Protolst[] = {
       { 'g', ggetpkt, gsendpkt, gopenpk, gclosepk,
              grdmsg,  gwrmsg,   geofpkt, gfilepkt,
              KWFalse,
       } ,

       { 'G', ggetpkt, gsendpkt, Gopenpk, gclosepk,
              grdmsg,  gwrmsg,   geofpkt, gfilepkt,
              KWFalse,
       } ,

       { 'f', fgetpkt, fsendpkt, fopenpk, fclosepk,
              frdmsg,  fwrmsg,   feofpkt, ffilepkt,
              KWFalse,
       } ,

       { 'v', ggetpkt, gsendpkt, vopenpk, gclosepk,
              grdmsg,  gwrmsg,   geofpkt, gfilepkt,
              KWFalse,
       } ,
#if defined(_Windows) || defined(BIT32ENV)
       { 'e', egetpkt, esendpkt, eopenpk, eclosepk,
              erdmsg,  ewrmsg,   eeofpkt, efilepkt,
              KWTrue,
       } ,
       { 't', tgetpkt, tsendpkt, topenpk, tclosepk,
              trdmsg,  twrmsg,
              geofpkt, gfilepkt, /* Yup, same as 'g'  */
              KWTrue,
       } ,
#endif
   { '\0' }
   };

short (*sendpkt)(char *data, short len);
short (*getpkt)(char *data, short *len);
short (*openpk)(const KWBoolean caller);
short (*closepk)(void);
short (*wrmsg)(char *data);
short (*rdmsg)(char *data);
short (*eofpkt)(void);
short (*filepkt)(const KWBoolean master, const unsigned long bytes);

char *flds[60];
int kflds;
static char protocols[5];
static char S_sysline[BUFSIZ];

static char bestGrade = 'A';

static void setproto(char wanted);

static char HostGrade( const char *fname, const char *remote );

/* ************************************** */
/*              Sub Systems             */
/* ************************************** */

/*--------------------------------------------------------------------*/
/*    g e t s y s t e m                                               */
/*                                                                    */
/*    Process a systems file (L.sys) entry.                           */
/*    Null lines or lines starting with '#' are comments.             */
/*--------------------------------------------------------------------*/

CONN_STATE getsystem( const char sendGrade )
{

   CONN_STATE nextState = CONN_CHECKTIME;

   do {

      char *p;

      /* flush to next non-comment line */

      if (fgets(S_sysline, BUFSIZ, fsys) == nil(char))
         return CONN_EXIT;

      p = S_sysline + strlen( S_sysline );

/*--------------------------------------------------------------------*/
/*                     Trim trailing white space                      */
/*--------------------------------------------------------------------*/

      while ((p-- > S_sysline) && isspace( *p ))
         *p = '\0';

   } while ((*S_sysline == '\0') || (*S_sysline == '#'));

   printmsg(8, "sysline=\"%s\"", S_sysline);

   kflds = getargs(S_sysline, flds);

/*--------------------------------------------------------------------*/
/*              Blitz all the extra fields we don't need              */
/*--------------------------------------------------------------------*/

   if ( kflds < FLD_EXPECT )
   {
      printmsg(0,"getsystem:  Invalid system entry "
                 "for %s (missing dial script)",
            flds[FLD_REMOTE] );

      return CONN_INITIALIZE;
   }

   strcpy(protocols, flds[FLD_PROTO]);
   strcpy(rmtname, flds[FLD_REMOTE]);

/*--------------------------------------------------------------------*/
/*                      Summarize the host data                       */
/*--------------------------------------------------------------------*/

   printmsg(2,
          "remote=%s, when=%s, device=%s, phone=%s, protocol=%s",
          rmtname, flds[FLD_CCTIME], flds[FLD_TYPE], flds[FLD_PHONE],
          protocols);

/*--------------------------------------------------------------------*/
/*                  Determine if the remote is valid                  */
/*--------------------------------------------------------------------*/

   hostp = checkreal( rmtname );
   if ( hostp == NULL )
   {
      printmsg(0,"getsystem: Internal lookup error for system %s",
                  rmtname);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                   Display the send/expect fields                   */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   if (debuglevel >= 4)
   {
      size_t i;

      flds[ kflds ] = "";     /* Insure valid send string            */

      for (i = FLD_EXPECT; i < kflds; i += 2)
         printmsg(6, "expect [%02d]:\t%s\nsend   [%02d]:\t%s",
            i, flds[i], i + 1, flds[i + 1]);

   }
#endif

/*--------------------------------------------------------------------*/
/*               Determine if we want to call this host               */
/*                                                                    */
/*    The following if statement breaks down to:                      */
/*                                                                    */
/*       if host not successfully called this run and                 */
/*             (  we are calling all hosts or                         */
/*                we are calling this host or                         */
/*                we are hosts with work and this host has work )     */
/*       then call this host                                          */
/*--------------------------------------------------------------------*/

   if (hostp->status.hstatus == HS_CALLED)
      nextState = CONN_INITIALIZE;
   else if (equal(Rmtname, "all") || equal(Rmtname, rmtname))
      nextState = CONN_CHECKTIME;
   else if ( equal(Rmtname, "any"))
   {
      char sysGrade = checktime( flds[FLD_CCTIME] );
                                 /* Initialize with lowest grade for
                                    this time of day                 */
      char scanGrade = sendGrade;

      if ( sysGrade && (scanGrade > sysGrade ))
         scanGrade = sysGrade;

      scandir( NULL, 0 );        /* Reset directory search if active */

      if (scandir(rmtname, scanGrade ) != XFER_REQUEST)
         nextState = CONN_INITIALIZE;  /* No work available          */
      else if ( sysGrade )
         nextState = CONN_CHECKTIME;   /* Work available, use it     */
      else
         nextState = CONN_NOGRADE;     /* Work avail, but wrong time */

   } /* if ( equal(Rmtname, "any")) */
   else
      nextState = CONN_INITIALIZE;

   scandir( NULL, 0);         /* Reset directory search again     */

/*--------------------------------------------------------------------*/
/*   We want to call the host; is it defined in our security table?   */
/*--------------------------------------------------------------------*/

   if ( nextState == CONN_CHECKTIME )
   {
      securep = GetSecurity( hostp );

      if ( securep == NULL )
      {
         printmsg(0,"getsystem: system \"%s\" not defined in "
                    "PERMISSIONS file", hostp->hostname);

         nextState = CONN_INITIALIZE;
      }
      else
         memset( &remote_stats, 0, sizeof remote_stats);

   } /* if */

/*--------------------------------------------------------------------*/
/*           Return our next requested state to the caller            */
/*--------------------------------------------------------------------*/

   return nextState;

} /* getsystem */

/*--------------------------------------------------------------------*/
/*    s y s e n d                                                     */
/*                                                                    */
/*    End UUCP session negotiation                                    */
/*--------------------------------------------------------------------*/

CONN_STATE sysend()
{
   char msg[80];

   wmsg("OOOOOO", KWTrue);
   rmsg(msg, SYNCH_DLE, 5, sizeof msg);
   wmsg("OOOOOO", KWTrue);
   ssleep(2);                 /* Wait for it to be transmitted       */

   return CONN_DROPLINE;

} /* sysend */

/*--------------------------------------------------------------------*/
/*    w m s g                                                         */
/*                                                                    */
/*    write a ^P type msg to the remote uucp                          */
/*--------------------------------------------------------------------*/

void wmsg(const char *msg, const KWBoolean synch)
{

   if (synch)
      swrite("\0\020", 2);

   printmsg( 4, "==> %s%s", synch ? "^p" : "", msg );

   swrite(msg, strlen(msg));

   if (synch)
      swrite("\0", 1);

} /* wmsg */

/*--------------------------------------------------------------------*/
/*    r m s g                                                         */
/*                                                                    */
/*    read a ^P msg from UUCP                                         */
/*--------------------------------------------------------------------*/

int rmsg(char *msg, const SYNCH synch, unsigned int msgtime, int max_len)
{
   int i = 0;
   char ch = '?';       /* Initialize to non-zero value  */    /* ahd  */

/*--------------------------------------------------------------------*/
/*                        flush until next ^P                         */
/*--------------------------------------------------------------------*/

   if (synch == SYNCH_DLE)
   {
      do {

         if (sread(&ch, 1, msgtime) < 1)
         {
            printmsg(2 ,"rmsg: Timeout waiting for sync");
            return TIMEOUT;
         } /* if */

      } while ((ch & 0x7f) != '\020');
   }

/*--------------------------------------------------------------------*/
/*       Read until timeout, null character, newline (for             */
/*       SYNCH_NONE and SYNCH_ECHO modes only), or until we fill      */
/*       the input buffer                                             */
/*--------------------------------------------------------------------*/

   while((i < max_len) && (ch != '\0'))
   {
      KWBoolean appendCharacter = KWTrue;

      if (sread(&ch, 1, msgtime) < 1)
      {
         printmsg(1 ,"rmsg: Timeout reading message");
         return TIMEOUT;
      }

/*--------------------------------------------------------------------*/
/*               Process backspaces if not in sync mode               */
/*--------------------------------------------------------------------*/

      if (( synch == SYNCH_NONE ) || ( synch == SYNCH_ECHO ))
      {
         char ASCIIch = (unsigned char) (0x7f & (unsigned char) ch);

         switch( ASCIIch )
         {

            case '\0':
               break;

            case 0x7F:
            case '\b':
               appendCharacter = KWFalse;

               if ( i )
               {
                  if ( synch == SYNCH_ECHO )
                     swrite( "\b \b", 3);
                  i--;
               }
               else
                  swrite( "\a", 1 );      /* Invalid backspace */
               break;

            case '\r':
            case '\n':
               if ( synch == SYNCH_ECHO )
                  swrite( &ch, 1 );
               ch = 0;                 /* Terminate processing */
               break;

            default:
               if ( ! isprint( ASCIIch ) )
               {
                  swrite( "\a", 1 );
                  appendCharacter = KWFalse;
               }
               else {
                  ch = ASCIIch;

                  if ( synch == SYNCH_ECHO )
                     swrite( &ch, 1 );
               }
               break;

         } /* switch( ASCIIch ) */

      } /* if */
      else if ( synch == SYNCH_DLE )
      {
         if (ch == '\r' || ch == '\n')
            ch = '\0';

      }
      else if ( synch == SYNCH_BINARY )
      {
         /* No operation */
      }
      else {
         printmsg(0,"rmsg: Invalid synch parameter %d",
                    (int) synch );
         panic();
      }

      if ( appendCharacter )
         msg[i++] = ch;

   } /* while((i < max_len) && (ch != '\0')) */

   msg[max_len - 1] = '\0';

   printmsg( 4, "<== %s%s",
                (synch == SYNCH_DLE) ? "^p" : "",
                msg);

   return (int) strlen(msg);

} /* rmsg */

/*--------------------------------------------------------------------*/
/*    s t a r t u p _ s e r v e r                                     */
/*                                                                    */
/*    Exchange host and protocol information for a system we called   */
/*--------------------------------------------------------------------*/

CONN_STATE startup_server(const char recvgrade )
{
   char msg[80];
   char *s;
   size_t hostlen;

   hostp->status.hstatus= HS_STARTUP_FAILED;
   hostp->via     = hostp->hostname;   /* Save true hostname           */

/*--------------------------------------------------------------------*/
/*                      Begin normal processing                       */
/*--------------------------------------------------------------------*/

   if (rmsg(msg, SYNCH_DLE, M_startupTimeout, sizeof msg) == TIMEOUT)
   {
      printmsg(0,"Startup: Timeout for first message");
      return CONN_TERMINATE;
   }

/*--------------------------------------------------------------------*/
/*              The first message must begin with Shere               */
/*--------------------------------------------------------------------*/

   if (!equaln(msg,"Shere",5))
   {
      printmsg(0,"Startup: First message not Shere, was \"%s\"", msg);
      return CONN_TERMINATE;
   }

/*--------------------------------------------------------------------*/
/*    The host can send either a simple Shere, or Shere=hostname;     */
/*    we allow either.                                                */
/*--------------------------------------------------------------------*/

   hostlen = strlen( msg + 6 );
   if ( hostlen < 7 )
      hostlen = 7;

   if ((msg[5] == '=') && !equaln(&msg[6], rmtname, hostlen))
   {
      printmsg(0,"Startup: Wrong host %s, expected %s",
               &msg[6], rmtname);
      hostp->status.hstatus= HS_WRONG_HOST;
      return CONN_TERMINATE; /* wrong host */              /* ahd */
   }

/*--------------------------------------------------------------------*/
/*    Setup our hello message with system name and optional debug     */
/*    and call grade levels.                                          */
/*--------------------------------------------------------------------*/

   sprintf(msg, "S%s", securep->myname );

   if ( bflag[F_SENDDEBUG] )
      sprintf( msg + strlen(msg), " -x%d", debuglevel );

   if (recvgrade != ALL_GRADES)
      sprintf( msg + strlen(msg), " -p%c -vgrade=%c",
                  recvgrade, recvgrade );

   wmsg(msg, KWTrue);

/*--------------------------------------------------------------------*/
/*                  Second message is system is okay                  */
/*--------------------------------------------------------------------*/

   if (rmsg(msg, SYNCH_DLE, M_startupTimeout, sizeof msg) == TIMEOUT)
   {
      printmsg(0,"Startup: Timeout for second message");
      return CONN_TERMINATE;
   }

   if (!equaln(&msg[1], "OK", 2))
   {
      printmsg(0,"Unexpected second message: %s",&msg[1]);
      return CONN_TERMINATE;
   }

/*--------------------------------------------------------------------*/
/*                Third message is protocol exchange                  */
/*--------------------------------------------------------------------*/

   if (rmsg(msg, SYNCH_DLE, M_startupTimeout, sizeof msg) == TIMEOUT)
      return CONN_TERMINATE;

   if (*msg != 'P')
   {
      printmsg(0,"Unexpected third message: %s",&msg[1]);
      return CONN_TERMINATE;
   }

/*--------------------------------------------------------------------*/
/*                      Locate a common procotol                      */
/*--------------------------------------------------------------------*/

   s = strpbrk( protocols, &msg[1] );
   if ( s == NULL )
   {
      printmsg(0,"Startup: No common protocol");
      wmsg("UN", KWTrue);
      return CONN_TERMINATE; /* no common protocol */
   }

/*--------------------------------------------------------------------*/
/*       While the remote is waiting for us, update our status        */
/*--------------------------------------------------------------------*/

   hostp->status.hstatus= HS_INPROGRESS;
   hostp->status.lconnect = time( &remote_stats.lconnect );

/*--------------------------------------------------------------------*/
/*              Tell the remote host the protocol to use              */
/*--------------------------------------------------------------------*/

   sprintf(msg, "U%c", *s);
   wmsg(msg, KWTrue);

   setproto(*s);

/*--------------------------------------------------------------------*/
/*    The connection is complete; report this and return to caller    */
/*--------------------------------------------------------------------*/

   if ( IsNetwork() )
      printmsg(0,"%s connected to %s: network link, %c protocol, %c grade",
                  securep->myname,
                  rmtname,
                  *s,
                  recvgrade );
   else
      printmsg(0,"%s connected to %s: %ld bps, %c protocol, %c grade",
                 securep->myname,
                 rmtname,
                 (long) GetSpeed(),
                 *s,
                 recvgrade );

   return CONN_SERVER;

} /* startup_server */

/*--------------------------------------------------------------------*/
/*    s t a r t u p _ c l i e n t                                     */
/*                                                                    */
/*    Setup a host connection with a system which has called us       */
/*--------------------------------------------------------------------*/

CONN_STATE startup_client( char *sendGrade )
{
   char plist[20];
   char msg[80];
   int xdebug = debuglevel;
   char *sysname = rmtname;
   Proto *tproto;
   char *s;
   char *flds[10];
   int  kflds,i;
   char grade = ALL_GRADES;

/*--------------------------------------------------------------------*/
/*    Challange the host calling in with the name defined for this    */
/*    login (if available) otherwise our regular node name.  (It's    */
/*    a valid session if the securep pointer is NULL, but this is     */
/*    trapped below in the call to ValidateHost()                     */
/*--------------------------------------------------------------------*/

   sprintf(msg, "Shere=%s", securep == NULL ?
                              E_nodename : securep->myname );
   wmsg(msg, KWTrue);

   if (rmsg(msg, SYNCH_DLE, M_startupTimeout, sizeof msg) == TIMEOUT)
      return CONN_TERMINATE;

   printmsg(2, "1st msg from remote = %s", msg);

/*--------------------------------------------------------------------*/
/*             Parse additional flags from remote system              */
/*--------------------------------------------------------------------*/

   kflds = getargs(msg,flds);
   strcpy(sysname,&flds[0][1]);

   for (i=1; i < kflds; i++)
   {
      if (flds[i][0] != '-')
         printmsg(0,"Invalid argument \"%s\" from system %s",
                    flds[i],
                    sysname);
      else
         switch(flds[i][1])
         {
            case 'Q' :             /* Ignore the remote sequence number  */
               break;

            case 'x' :
               if ( bflag[ F_HONORDEBUG ] )
                  sscanf(flds[i], "-x%d", &xdebug);
               break;

            case 'p' :
               sscanf(flds[i], "-p%c", &grade);
               break;

            case 'v' :
               sscanf(flds[i], "-vgrade=%c", &grade);
               break;

            default  :
               printmsg(0,"Invalid argument \"%s\" from system %s",
                          flds[i],
                          sysname);
               break;
         } /* switch */
   } /* for */

   *sendGrade = (char) min(grade,*sendGrade);

/*--------------------------------------------------------------------*/
/*                Verify the remote host name is good                 */
/*--------------------------------------------------------------------*/

   hostp = checkreal( sysname );

   if ( hostp == BADHOST )
   {
      if (E_anonymous != NULL)
      {
         hostp = checkreal( ANONYMOUS_HOST );      /* Find dummy entry */

         if ( hostp == BADHOST )       /* Was it there?              */
            panic();                   /* No --> Drop wing, run in
                                          circles like sky is falling*/

         if (!checktime( E_anonymous )) /* Good time to call?         */
         {
            wmsg("RWrong time for anonymous system",KWTrue);
            printmsg(0,"Wrong time for anonymous system \"%s\"",sysname);
         }  /* if */

         if ( !LockSystem( sysname , B_UUCICO ))
         {
            wmsg("RLCK",KWTrue);  /* Odd, we locked anonymous system? */
            return CONN_TERMINATE;
         }

         hostp->via = newstr( sysname );
         sysname = ANONYMOUS_HOST;

      }    /* if (E_anonymous != NULL) */
      else {
         wmsg("RYou are unknown to me",KWTrue);
         printmsg(0,"startup: Unknown host \"%s\"", sysname);
         return CONN_TERMINATE;
      } /* else */
   } /* if ( hostp == BADHOST ) */
   else if ( LockSystem( hostp->hostname , B_UUCICO ))
      hostp->via = hostp->hostname;
   else {
      wmsg("RLCK",KWTrue);
      return CONN_TERMINATE;
   } /* else */

/*--------------------------------------------------------------------*/
/*                   Correct host for this user id?                   */
/*--------------------------------------------------------------------*/

   if ( !ValidateHost( sysname ))
                                          /* Wrong host for user? */
   {                                      /* Yes --> Abort        */
      wmsg("RLOGIN",KWTrue);
      printmsg(0,"startup: Access rejected for host \"%s\"", sysname);
      hostp->status.hstatus= HS_WRONG_HOST;
      return CONN_TERMINATE;
   } /* if */

   strcpy(rmtname, hostp->hostname);      /* Make sure we use the
                                             full host name       */

/*--------------------------------------------------------------------*/
/*                If we must call the user back, do so                */
/*--------------------------------------------------------------------*/

   if (securep->callback)
   {
      wmsg("RCB",KWTrue);
      hostp->status.hstatus= HS_CALLBACK_REQ;
      return CONN_TERMINATE;  /* Really more complex than this       */
   }

/*--------------------------------------------------------------------*/
/*                     Set the local debug level                      */
/*--------------------------------------------------------------------*/

   if ( xdebug > debuglevel )
   {
      debuglevel = xdebug;
      printmsg(0, "Debuglevel set to %d by remote", debuglevel);
   }

/*--------------------------------------------------------------------*/
/*                     Build local protocol list                      */
/*--------------------------------------------------------------------*/

   s = plist;
   for (tproto = Protolst; tproto->type != '\0' ; tproto++)
   {
      if ( IsNetwork() || !tproto->network )
         *s++ = tproto->type;
   }

   *s = '\0';                 /* Terminate our string                */

/*--------------------------------------------------------------------*/
/*              The host name is good; get the protocol               */
/*--------------------------------------------------------------------*/

   wmsg("ROK", KWTrue);

   sprintf(msg, "P%s", plist);
   wmsg(msg, KWTrue);

   if (rmsg(msg, SYNCH_DLE, M_startupTimeout, sizeof msg) == TIMEOUT)
      return CONN_TERMINATE;

   if (msg[0] != 'U')
   {
      printmsg(0,"Unexpected second message: %s", msg);
      return CONN_TERMINATE;
   }

   if (strchr(plist, msg[1]) == nil(char))
   {
      printmsg(0,"startup: Host %s does not support our protocols",
            rmtname );
      return CONN_TERMINATE;
   }

   setproto(msg[1]);

/*--------------------------------------------------------------------*/
/*            Report that we connected to the remote host             */
/*--------------------------------------------------------------------*/

   if ( IsNetwork() )
      printmsg(0,"%s called by %s: network link, %c protocol, %c grade",
            securep->myname,
            hostp->via,
            msg[1],
            *sendGrade );
   else
      printmsg(0,"%s called by %s: %ld bps, %c protocol, %c grade",
            securep->myname,
            hostp->via,
            (long) GetSpeed(),
            msg[1],
            *sendGrade );

   if ( hostp == BADHOST )
      panic();

   hostp->status.hstatus = HS_INPROGRESS;
   hostp->status.lconnect = time( &remote_stats.lconnect );

   return CONN_CLIENT;

} /* startup_client */

/*--------------------------------------------------------------------*/
/*    s e t p r o t o                                                 */
/*                                                                    */
/*    set the protocol to be used                                     */
/*--------------------------------------------------------------------*/

static void setproto(char wanted)
{
   Proto *tproto;

   for (tproto = Protolst;
      tproto->type != '\0' && tproto->type != wanted;
      tproto++) {
      printmsg(3, "setproto: wanted '%c', have '%c'", wanted, tproto->type);
   }

   if (tproto->type == '\0') {
      printmsg(0, "setproto: You said I have protocol '%c' but I cant find it!",
            wanted);
      panic();
   }

   printmsg(3, "setproto: wanted '%c', have '%c'", wanted, tproto->type);

   getpkt  = tproto->getpkt;
   sendpkt = tproto->sendpkt;
   openpk  = tproto->openpk;
   closepk = tproto->closepk;
   rdmsg   = tproto->rdmsg;
   wrmsg   = tproto->wrmsg;
   eofpkt  = tproto->eofpkt;
   filepkt = tproto->filepkt;

} /* setproto */

/*--------------------------------------------------------------------*/
/*       r e s e t G r a d e                                          */
/*                                                                    */
/*       Reset the best grade to one which cannot be satisfied,       */
/*       insuring failure if the file search doesn't turn up a        */
/*       better one.                                                  */
/*--------------------------------------------------------------------*/

void resetGrade( void )
{

   bestGrade = 0x7f;

} /* resetGrade */

/*--------------------------------------------------------------------*/
/*       n e x t G r a d e                                            */
/*                                                                    */
/*       Return next best grade to process at or above minimum        */
/*       specified grade.  Returns 0 if no files remain within        */
/*       requested range.                                             */
/*--------------------------------------------------------------------*/

char nextGrade( const char grade )
{
   char saveGrade = bestGrade;

   resetGrade();           /* Don't allow old grade to be used again  */

/*--------------------------------------------------------------------*/
/*               Return the status of the previous search             */
/*--------------------------------------------------------------------*/

   if ( saveGrade > grade )
      return '\0';
   else {
      printmsg(1,"nextGrade: Processing grade %c call files",
                 saveGrade );
      return saveGrade;
   }

} /* nextGrade */

/*--------------------------------------------------------------------*/
/*    s c a n d i r                                                   */
/*                                                                    */
/*    Scan spooling directory for C.* files for the remote host       */
/*    (rmtname)                                                       */
/*--------------------------------------------------------------------*/

XFER_STATE scandir(char *remote, const char grade )
{
   static DIR *dirp = NULL;
   static char *SaveRemote = NULL;
   static char remotedir[FILENAME_MAX];

   struct direct *dp;

/*--------------------------------------------------------------------*/
/*          Determine if we must restart the directory scan           */
/*--------------------------------------------------------------------*/

   if (fwork != NULL )
   {
      fclose( fwork );
      fwork = NULL;
   }

   if ( (remote == NULL) || ( SaveRemote == NULL ) ||
        !equaln(remote, SaveRemote, sizeof SaveRemote - 1 ) )
   {
      if ( SaveRemote != NULL ) /* Clean up old directory? */
      {                          /* Yes --> Do so           */
         closedir(dirp);
         dirp = NULL;
         SaveRemote = NULL;
      } /* if */

      if ( remote == NULL )      /* Clean up only, no new search? */
         return XFER_NOLOCAL;    /* Yes --> Return to caller      */

      sprintf(remotedir,"%s/%.8s/C", E_spooldir, remote);

      if ((dirp = opendir(remotedir)) == nil(DIR))
      {
         printmsg(2, "scandir: couldn't opendir() %s", remotedir);
         return XFER_NOLOCAL;
      } /* if */

      SaveRemote = newstr( remote );
                              /* Flag we have an active search    */

   } /* if */

/*--------------------------------------------------------------------*/
/*              Look for the next file in the directory               */
/*--------------------------------------------------------------------*/

   while ((dp = readdir(dirp)) != nil(struct direct))
   {
      char fileGrade;

      sprintf(workfile, "%s/%s", remotedir, dp->d_name);

      fileGrade = HostGrade( workfile, remote );

      if ( fileGrade > grade )
      {
         printmsg(5, "scandir: skipped \"%s\" (grade %c not met)",
                     workfile, grade );

         if ( fileGrade < bestGrade )
         {
            printmsg(5,"scandir: Previous best grade = %c, new best = %c",
                        bestGrade,
                        fileGrade );

            bestGrade = fileGrade;

         }  /* if ( fileGrade < bestGrade ) */

      } /* if ( fileGrade > grade ) */
      else if ((fwork = FOPEN(workfile, "r",TEXT_MODE)) == nil(FILE))
      {
         printmsg(0,"scandir: open failed for %s",workfile);
         SaveRemote = NULL;
         return XFER_ABORT;   /* Very bad, since we just read its
                                 directory entry!                 */
      }
      else {
         setvbuf( fwork, NULL, _IONBF, 0);
         printmsg(5, "scandir: matched \"%s\"",workfile);
         return XFER_REQUEST; /* Return success                   */
      }

   } /* while */

/*--------------------------------------------------------------------*/
/*     No hit; clean up after ourselves and return to the caller      */
/*--------------------------------------------------------------------*/

   printmsg(5, "scandir: \"%s\" not matched", remotedir);
   closedir(dirp);
   dirp = NULL;
   SaveRemote = NULL;

   return XFER_NEXTGRADE;

} /* scandir */

/*--------------------------------------------------------------------*/
/*    H o s t G r a d e                                               */
/*                                                                    */
/*    Return host grade of a call file                                */
/*--------------------------------------------------------------------*/

static char HostGrade( const char *fname, const char *remote )
{

   char tempname[FILENAME_MAX];
   size_t len = strlen( remote );

   exportpath( tempname, fname, remote );

   if ( len > HOSTLEN )
      len = HOSTLEN;

   return tempname[len + 2 ];

} /* HostGrade */

/*--------------------------------------------------------------------*/
/*    C a l l W i n d o w                                             */
/*                                                                    */
/*    Determine if we can call a system                               */
/*--------------------------------------------------------------------*/

KWBoolean CallWindow( const char callgrade )
{

/*--------------------------------------------------------------------*/
/*      Determine if the window for calling this system is open       */
/*--------------------------------------------------------------------*/

   if ( !callgrade && equal(flds[FLD_CCTIME],"Never" ))
   {
      hostp->status.hstatus = HS_WRONG_TIME;
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*    Check the time of day and whether or not we should call now.    */
/*                                                                    */
/*    If calling a system to set the clock and we determine the       */
/*    system clock is bad (we fail the sanity check of the last       */
/*    connected a host to being in the future), then we ignore the    */
/*    time check field.                                               */
/*--------------------------------------------------------------------*/

   if (!callgrade)
   {
      if ((*flds[FLD_PROTO] != '*') ||       /* Not setting clock?   */
          ((hostp->status.ltime >  630720000L )))
                                             /* Clock okay?          */
      {                                      /* Yes--> Return        */
         hostp->status.hstatus = HS_WRONG_TIME;
         time(&hostp->status.ltime);  /* Save time of last attempt to call  */
         return KWFalse;
      }
   } /* if */

/*--------------------------------------------------------------------*/
/*       We pass the time check                                       */
/*--------------------------------------------------------------------*/

   return KWTrue;

} /* CallWindow */
