/*
   For best results in visual layout while viewing this file, set
   tab stops to every 4 columns.
*/

/*
   dcpsys.c

   Revised edition of dcp

   Stuart Lynne May/87

   Copyright (c) Richard H. Lamb 1985, 1986, 1987
   Changes Copyright (c) Stuart Lynne 1987
   Changes Copyright (c) Andrew H. Derbyshire 1989, 1990

   Updated:

      13May89  - Modified checkname to only examine first token of name.
                 Modified rmsg to initialize input character before use.
                 - ahd
      16May89  - Moved checkname to router.c - ahd
      17May89  - Wrote real checktime() - ahd
      17May89  - Changed getsystem to return 'I' instead of 'G'
      25Jun89  - Added Reach-Out America to keyword table for checktime
      22Sep89  - Password file support for hosts
      25Sep89  - Change 'ExpectStr' message to debuglevel 2
      01Jan90  - Revert 'ExpectStr' message to debuglevel 1
      28Jan90  - Alter callup() to use table driven modem driver.
                 Add direct(), qx() procedures.
      8 Jul90  - Add John DuBois's expectstr() routine to fix problems
                 with long input buffers.
      11Nov90  - Delete QX support, add ddelay, ssleep calls
      21Sep92  - Insure system system name and time do not crash
                 UUCICO - from the original fix by Eugene Nesterenko,
                 Moscow, Russia


*/

/*
 *     $Id: DCPSYS.C 1.5 1992/11/18 03:49:21 ahd Exp $
 *
 *     $Log: DCPSYS.C $
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


/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */


/* Support routines */

/*--------------------------------------------------------------------*/
/*                        system include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "arpadate.h"
#include "checktim.h"
#include "dcp.h"
#include "dcpfpkt.h"
#include "dcpgpkt.h"
#include "dcplib.h"
#include "dcpsys.h"
#include "export.h"
#include "hlib.h"
#include "hostable.h"
#include "hostatus.h"
#include "modem.h"
#include "lock.h"
#include "nbstime.h"
#include "ndir.h"
#include "ssleep.h"
#include "security.h"
#include "ulib.h"

currentfile();

Proto Protolst[] = {
       { 'g', ggetpkt, gsendpkt, gopenpk, gclosepk,
              grdmsg,  gwrmsg,   geofpkt, gfilepkt } ,

       { 'G', ggetpkt, gsendpkt, Gopenpk, gclosepk,
              grdmsg,  gwrmsg,   geofpkt, gfilepkt } ,

       { 'f', fgetpkt, fsendpkt, fopenpk, fclosepk,
              frdmsg,  fwrmsg,   feofpkt, ffilepkt } ,

       { 'v', ggetpkt, gsendpkt, vopenpk, gclosepk,
              grdmsg,  gwrmsg,   geofpkt, gfilepkt } ,
   { '\0' }
   };

procref  getpkt, sendpkt, openpk, closepk, rdmsg, wrmsg, eofpkt, filepkt;

char *flds[60];
int kflds;
static char protocols[5];
static char S_sysline[BUFSIZ];

static void setproto(char wanted);

static char HostGrade( const char *fname, const char *remote );

/****************************************/
/*              Sub Systems             */
/****************************************/

/*--------------------------------------------------------------------*/
/*    g e t s y s t e m                                               */
/*                                                                    */
/*    Process a systems file (L.sys) entry.                           */
/*    Null lines or lines starting with '#' are comments.             */
/*--------------------------------------------------------------------*/

CONN_STATE getsystem( const char sendgrade )
{

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
          "remote=%s, call-time=%s, device=%s, telephone=%s, protocol=%s",
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

   if (debuglevel >= 4) {
      int   i;
      flds[ kflds ] = "";     /* Insure valid send string            */

      for (i = FLD_EXPECT; i < kflds; i += 2)
         printmsg(6, "expect [%02d]:\t%s\nsend   [%02d]:\t%s",
            i, flds[i], i + 1, flds[i + 1]);
   }

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

   fwork = nil(FILE);
   if ((hostp->hstatus != called) &&
       (equal(Rmtname, "all") || equal(Rmtname, rmtname) ||
        (equal(Rmtname, "any") &&
        (scandir(rmtname,sendgrade) == XFER_REQUEST))))
    {

      if (fwork != nil(FILE)) /* in case matched with scandir     */
         fclose(fwork);

      scandir( NULL, sendgrade); /* Reset directory search as well   */

/*--------------------------------------------------------------------*/
/*   We want to call the host; is it defined in our security table?   */
/*--------------------------------------------------------------------*/

      securep = GetSecurity( hostp );
      if ( securep == NULL )
      {
         printmsg(0,"getsystem: system \"%s\" not defined in "
                    "PERMISSIONS file", hostp->hostname);
         return CONN_INITIALIZE;
      }

      memset( &remote_stats, 0, sizeof remote_stats);
      return CONN_CALLUP1;    /* startup this system */

   } /* if */
   else
      return CONN_INITIALIZE;    /* Look for next system to process   */

} /*getsystem*/

/*--------------------------------------------------------------------*/
/*    s y s e n d                                                     */
/*                                                                    */
/*    End UUCP session negotiation                                    */
/*--------------------------------------------------------------------*/

CONN_STATE sysend()
{
   char msg[80];

   if (hostp->hstatus == inprogress)
      hostp->hstatus = call_failed;

   wmsg("OOOOOO", TRUE);
   rmsg(msg, TRUE, 5, sizeof msg);
   wmsg("OOOOOO", TRUE);
   ssleep(2);                 /* Wait for it to be transmitted       */

   return CONN_DROPLINE;
} /*sysend*/


/*
   w m s g

   write a ^P type msg to the remote uucp
*/

void wmsg(char *msg, const boolean synch)
{

   if (synch)
      swrite("\0\020", 2);

   printmsg( 4, "==> %s%s", synch ? "^p" : "", msg );

   swrite(msg, strlen(msg));

   if (synch)
      swrite("\0", 1);

} /*wmsg*/


/*
   r m s g

   read a ^P msg from UUCP
*/

int rmsg(char *msg, const boolean synch, unsigned int msgtime, int max_len)
{
   int i;
   char ch = '?';       /* Initialize to non-zero value  */    /* ahd   */

/*--------------------------------------------------------------------*/
/*                        flush until next ^P                         */
/*--------------------------------------------------------------------*/

   if (synch == 1)
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
/*   Read until timeout, next newline, or we fill the input buffer    */
/*--------------------------------------------------------------------*/

   for (i = 0; (i < max_len) && (ch != '\0'); )
   {
      if (sread(&ch, 1, msgtime) < 1)
      {
         printmsg(1 ,"rmsg: Timeout reading message");
         return TIMEOUT;
      }

/*--------------------------------------------------------------------*/
/*               Process backspaces if not in sync mode               */
/*--------------------------------------------------------------------*/

      if ((synch != 1) &&
          (ch != '\r') &&
          (ch != '\n') &&
          (ch != '\0') &&
          iscntrl( ch ))
      {
         if ( i && ((ch == 0x7f) || (ch == '\b')))
         {
            i--;
            if ( synch == 2 )
               swrite( "\b \b", 3);
         }
         else {

            swrite( "\a", 1 );   /* Beep in response to invalid
                                    cntrl characters, including
                                    extra backspaces                 */
         } /* else */

      } /* if */
      else {                  /* else a normal character             */

/*--------------------------------------------------------------------*/
/*             Echo the character if requested by caller              */
/*--------------------------------------------------------------------*/

         if ( synch == 2 )
            swrite( &ch, 1);

         ch &= 0x7f;
         if (ch == '\r' || ch == '\n')
            ch = '\0';
         msg[i++] = ch;
      } /* else */
   }

   msg[max_len - 1] = '\0';
   printmsg( 4, "<== %s%s",
                (synch == 1) ? "^p" : "",
                msg);
   return strlen(msg);

} /*rmsg*/


/*--------------------------------------------------------------------*/
/*    s t a r t u p _ s e r v e r                                     */
/*                                                                    */
/*    Exchange host and protocol information for a system we called   */
/*--------------------------------------------------------------------*/

CONN_STATE startup_server(const char recvgrade )
{
   char msg[80];
   char *s;


   hostp->hstatus = startup_failed;

/*--------------------------------------------------------------------*/
/*    Handle the special case of '*' protocol, which is really our    */
/*    NBS time setting support                                        */
/*--------------------------------------------------------------------*/

   if (*protocols == '*')
   {
      if (nbstime())
      {
         hostp->hstatus = called;
         time( &hostp->hstats->lconnect );
      }

      return CONN_DROPLINE;
   }

/*--------------------------------------------------------------------*/
/*                      Begin normal processing                       */
/*--------------------------------------------------------------------*/

   if (rmsg(msg, TRUE, PROTOCOL_TIME, sizeof msg) == TIMEOUT)
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

   if ((msg[5] == '=') && !equaln(&msg[6], rmtname, HOSTLEN))
   {
      printmsg(0,"Startup: Wrong host %s, expected %s",
               &msg[6], rmtname);
      hostp->hstatus = wrong_host;
      return CONN_TERMINATE; /* wrong host */              /* ahd */
   }

   /* sprintf(msg, "S%.7s -Q0 -x%d", E_nodename, debuglevel); */
   /* -Q0 -x16 remote debuglevel set */

   if (recvgrade != ALL_GRADES)
     sprintf(msg, "S%s -p%c -vgrade=%c", securep->myname,
                  recvgrade, recvgrade );
   else
     sprintf(msg, "S%s", securep->myname );

   wmsg(msg, TRUE);

/*--------------------------------------------------------------------*/
/*                  Second message is system is okay                  */
/*--------------------------------------------------------------------*/

   if (rmsg(msg, TRUE, PROTOCOL_TIME, sizeof msg) == TIMEOUT)
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

   if (rmsg(msg, TRUE, PROTOCOL_TIME, sizeof msg) == TIMEOUT)
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
      wmsg("UN", TRUE);
      return CONN_TERMINATE; /* no common protocol */
   }

/*--------------------------------------------------------------------*/
/*       While the remote is waiting for us, update our status        */
/*--------------------------------------------------------------------*/

   hostp->hstatus = inprogress;
   hostp->hstats->lconnect = time( &remote_stats.lconnect );

/*--------------------------------------------------------------------*/
/*              Tell the remote host the protocol to use              */
/*--------------------------------------------------------------------*/

   sprintf(msg, "U%s", s);
   wmsg(msg, TRUE);

   setproto(*s);

/*--------------------------------------------------------------------*/
/*    The connection is complete; report this and return to caller    */
/*--------------------------------------------------------------------*/

   printmsg(0,"%s connected to %s: %ld bps, %c protocol, %c grade",
         E_nodename, rmtname, (long) GetSpeed() , *s, recvgrade );

   return CONN_SERVER;

} /*startup_server*/

/*--------------------------------------------------------------------*/
/*    s t a r t u p _ c l i e n t                                     */
/*                                                                    */
/*    Setup a host connection with a system which has called us       */
/*--------------------------------------------------------------------*/

CONN_STATE startup_client( char *sendgrade )
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
   wmsg(msg, TRUE);

   if (rmsg(msg, TRUE, PROTOCOL_TIME, sizeof msg) == TIMEOUT)
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
            case 'Q' :             /* Ignore the remote sequence number   */
               break;

            case 'x' :
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


   *sendgrade = min(grade,*sendgrade);

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
            wmsg("RWrong time for anonymous system",TRUE);
            printmsg(0,"Wrong time for anonymous system \"%s\"",sysname);
         }  /* if */

         if ( !LockSystem( sysname , B_UUCICO ))
         {
            wmsg("RLCK",TRUE);   /* Odd, we locked anonymous system? */
            return CONN_TERMINATE;
         }

         hostp->via = strdup( sysname );
         sysname = ANONYMOUS_HOST;

         if ((xdebug > 3)  && (xdebug > debuglevel))
         {
            wmsg("RDebug (-x) level too high for anonymous UUCP - rejected",
                  TRUE);
            printmsg(0,"Excessive debug for anonymous system \"%s\"",sysname);
            return CONN_TERMINATE;
         } /* if (xdebug > 3) */

      }    /* if (E_anonymous != NULL) */
      else {
         wmsg("RYou are unknown to me",TRUE);
         printmsg(0,"startup: Unknown host \"%s\"", sysname);
         return CONN_TERMINATE;
      } /* else */
   } /* if ( hostp == BADHOST ) */
   else if ( LockSystem( hostp->hostname , B_UUCICO ))
      hostp->via = hostp->hostname;
   else {
      wmsg("RLCK",TRUE);
      return CONN_TERMINATE;
   } /* else */

/*--------------------------------------------------------------------*/
/*                   Correct host for this user id?                   */
/*--------------------------------------------------------------------*/

   if ( !ValidateHost( sysname ))
                                          /* Wrong host for user? */
   {                                      /* Yes --> Abort        */
      wmsg("RLOGIN",TRUE);
      printmsg(0,"startup: Access rejected for host \"%s\"", sysname);
      hostp->hstatus = wrong_host;
      return CONN_TERMINATE;
   } /* if */

   strcpy(rmtname, hostp->hostname);      /* Make sure we use the
                                             full host name       */

/*--------------------------------------------------------------------*/
/*                If we must call the user back, do so                */
/*--------------------------------------------------------------------*/

   if (securep->callback)
   {
      wmsg("RCB",TRUE);
      hostp->hstatus = callback_req;
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
      *s++ = tproto->type;

   *s = '\0';                 /* Terminate our string                */

/*--------------------------------------------------------------------*/
/*              The host name is good; get the protocol               */
/*--------------------------------------------------------------------*/

   wmsg("ROK", TRUE);

   sprintf(msg, "P%s", plist);
   wmsg(msg, TRUE);

   if (rmsg(msg, TRUE, PROTOCOL_TIME, sizeof msg) == TIMEOUT)
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

   printmsg(0,"%s called by %s: %ld bps, %c protocol, %c grade",
         E_nodename,
         hostp->via,
         (long) GetSpeed(),
         msg[1],
         *sendgrade );

   if ( hostp == BADHOST )
      panic();

   hostp->hstatus = inprogress;
   hostp->hstats->lconnect = time( &remote_stats.lconnect );

   return CONN_CLIENT;

} /*startup_client*/

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

} /*setproto*/

/*
      s c a n d i r

      Scan spooling directory for C.* files for the remote host
      (rmtname)

      Assumes the parameter remote is from static storage!
*/

XFER_STATE scandir(char *remote, const char grade )
{
   static DIR *dirp;
   static char SaveRemote[HOSTLEN+1] = "";
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

   if ( (remote == NULL) || (! strlen(SaveRemote) ) ||
        !equaln(remote, SaveRemote, sizeof SaveRemote - 1 ) )
   {
      if ( strlen( SaveRemote )) /* Clean up old directory? */
      {                          /* Yes --> Do so           */
         closedir(dirp);
         *SaveRemote = '\0';
      } /* if */

      if ( remote == NULL )      /* Clean up only, no new search? */
         return XFER_NOLOCAL;    /* Yes --> Return to caller      */

      sprintf(remotedir,"%s/%.8s/C", E_spooldir, remote);
      if ((dirp = opendir(remotedir)) == nil(DIR))
      {
         printmsg(2, "scandir: couldn't opendir() %s", remotedir);
         return XFER_NOLOCAL;
      } /* if */

      strncpy( SaveRemote , (char *) remote, sizeof SaveRemote - 1 );
                              /* Flag we have an active search    */

      SaveRemote[ sizeof SaveRemote - 1 ] = '\0';

   } /* if */

/*--------------------------------------------------------------------*/
/*              Look for the next file in the directory               */
/*--------------------------------------------------------------------*/

   while ((dp = readdir(dirp)) != nil(struct direct))
   {
      sprintf(workfile, "%s/%s", remotedir, dp->d_name);

      if ( HostGrade( workfile, remote ) > grade )
         printmsg(5, "scandir: skipped \"%s\" (grade %c not met)",             workfile,
                      grade );
      else if ((fwork = FOPEN(workfile, "r", TEXT)) == nil(FILE))
      {
         printmsg(0,"scandir: open failed for %s",workfile);
         *SaveRemote = '\0';
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
   *SaveRemote = '\0';
   return XFER_NOLOCAL;

} /*scandir*/

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

boolean CallWindow( const char callgrade )
{

/*--------------------------------------------------------------------*/
/*      Determine if the window for calling this system is open       */
/*--------------------------------------------------------------------*/

   if ( !callgrade && equal(flds[FLD_CCTIME],"Never" ))
   {
      hostp->hstatus = wrong_time;
      return FALSE;
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
          ((hostp->hstats->ltime >  hostp->hstats->lconnect) &&
           (hostp->hstats->ltime >  630720000L )))
                                             /* Clock okay?          */
      {                                      /* Yes--> Return        */
         hostp->hstatus = wrong_time;
         time(&hostp->hstats->ltime);  /* Save time of last attempt to call   */
         return FALSE;
      }
   } /* if */

/*--------------------------------------------------------------------*/
/*       We pass the time check                                       */
/*--------------------------------------------------------------------*/

   return TRUE;

} /* CallWindow */
