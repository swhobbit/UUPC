/*
   For best results in visual layout while viewing this file, set
   tab stops to every 4 columns.
   "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987
*/

/*
   dcpxfer.c

   Revised edition of dcp

   Stuart Lynne May/87

   Copyright (c) Richard H. Lamb 1985, 1986, 1987
   Changes Copyright (c) Stuart Lynne 1987
   Changes Copyright (c) Jordan Brown 1990, 1991
   Changes Copyright (c) Andrew H. Derbyshire 1989, 1991


   Maintenance Notes:

   01Nov87 - that strncpy should be a memcpy! - Jal
   22Jul90 - Add check for existence of the file before writing
             it.                                                  ahd
   09Apr91 - Add numerous changes from H.A.E.Broomhall and Cliff
             Stanford for bidirectional support                   ahd
   05Jul91 - Merged various changes from Jordan Brown's (HJB)
             version of UUPC/extended to clean up transmission
             of commands, etc.                                    ahd
   09Jul91 - Rewrite to use unique routines for all four kinds of
             transfers to allow for testing and security          ahd

*/

/*
 *       $Id: dcpxfer.c 1.2 1992/11/15 20:09:50 ahd Exp $
 *
 *       $Log: dcpxfer.c $
 * Revision 1.2  1992/11/15  20:09:50  ahd
 * Use unbuffered files to eliminate extra data copy
 * Clean up modem file support for different protocols
 *
 */

static const char rcsid[] =
         "$Id: dcpxfer.c 1.2 1992/11/15 20:09:50 ahd Exp $";

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <ctype.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "dcpsys.h"
#include "dcpxfer.h"
#include "expath.h"
#include "hlib.h"
#include "hostable.h"
#include "import.h"
#include "security.h"
#include "modem.h"
#include "ulib.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static unsigned char *databuf = NULL;
static unsigned int xfer_bufsize;

static char fname[FILENAME_MAX], tname[FILENAME_MAX], dname[FILENAME_MAX];
static char type, cmdopts[16];

static long bytes;
static struct timeb start_time;

static boolean spool = FALSE; /* Received file is into spool dir     */
static char spolname[FILENAME_MAX];
                              /* Final host name of file to be
                                 received into spool directory       */
static char tempname[FILENAME_MAX];
                              /* Temp name used to create received
                                 file                                */

currentfile();

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static boolean pktgetstr( char *s);
static boolean pktsendstr( char *s );
static void buf_init( void );

static int  bufill(char  *buffer);
static int  bufwrite(char  *buffer,int  len);

/*************** SEND PROTOCOL ***************************/

/*--------------------------------------------------------------------*/
/*    s d a t a                                                       */
/*                                                                    */
/*    Send File Data                                                  */
/*--------------------------------------------------------------------*/

XFER_STATE sdata( void )
{
   int S_size;
   int used = 0;

   S_size = bufill((char *) databuf);

   if (S_size == 0)                 /* Get data from file         */
      return XFER_SENDEOF;          /* if EOF set state to that   */
   else if (S_size == -1)           /* If error ...               */
      return XFER_ABORT;            /* Toss file                  */

   do {
      size_t xmit = min( (size_t) S_size - used , pktsize );

      if ((*sendpkt)((char *) databuf + used, xmit) != OK)   /* Send data */
      {
         fclose( xfer_stream );
         xfer_stream = NULL;
         return XFER_LOST;    /* Trouble!                            */
      }
      else
         used += xmit;

   } while( S_size > used );

   return XFER_SENDDATA;   /* Remain in send state                */

} /*sdata*/


/*--------------------------------------------------------------------*/
/*    b u f i l l                                                     */
/*                                                                    */
/*    Get a bufferful of data from the file that's being sent.        */
/*    (Should perform input buffering here, perhaps 4K at a time.)    */
/*--------------------------------------------------------------------*/

static int bufill(char *buffer)
{
   size_t count = fread(buffer,
                        sizeof *buffer,
                        xfer_bufsize,
                        xfer_stream);

   bytes += count;
   if ((count < xfer_bufsize) && ferror(xfer_stream))
   {
      printerr("bufill");
      clearerr(xfer_stream);
      return -1;
   }
   return count;

} /*bufill*/

/*--------------------------------------------------------------------*/
/*    b u f w r i t e                                                 */
/*                                                                    */
/*    Write a bufferful of data to the file that's being received.    */
/*--------------------------------------------------------------------*/

static int bufwrite(char *buffer, int len)
{
   int count = fwrite(buffer, sizeof *buffer, len, xfer_stream);

   bytes += count;
   if (count < len)
   {
      printerr("bufwrite");
      clearerr(xfer_stream);
   }

   return count;

} /*bufwrite*/

/*--------------------------------------------------------------------*/
/*    s b r e a k                                                     */
/*                                                                    */
/*    Switch from master to slave mode                                */
/*                                                                    */
/*    Sequence:                                                       */
/*                                                                    */
/*       We send "H" to other host to ask if we should hang up        */
/*       If it responds "HN", it has work for us, we become           */
/*          the slave.                                                */
/*       If it responds "HY", it has no work for us, we               */
/*          response "HY" (we have no work either), and               */
/*          terminate protocol and hangup                             */
/*                                                                    */
/*    Note that if more work is queued on the local system while      */
/*    we are in slave mode, schkdir() causes us to become the         */
/*    master again; we just decline here to avoid trying the queue    */
/*    again without intervening work from the other side.             */
/*--------------------------------------------------------------------*/

XFER_STATE sbreak( void )
{
   if (!pktsendstr("H"))      /* Tell slave it can become the master */
      return XFER_LOST;       /* Xmit fail?  If so, quit transmitting*/

   if (!pktgetstr((char *)databuf)) /* Get their response            */
      return XFER_LOST;       /* Xmit fail?  If so, quit transmitting*/

   if ((*databuf != 'H') || ((databuf[1] != 'N') && (databuf[1] != 'Y')))
   {
      printmsg(0,"Invalid response from remote: %s",databuf);
      return XFER_ABORT;
   }

   if (databuf[1] == 'N')     /* "HN" (have work) message from host? */
   {                          /* Yes --> Enter Receive mode          */
      printmsg( 2, "sbreak: Switch into slave mode" );
      return XFER_SLAVE;
   }
   else {                     /* No --> Remote host is done as well  */
      pktsendstr("HY");       /* Tell the host we are done as well   */
      hostp->hstatus = called;/* Update host status flags            */
      return XFER_ENDP;       /* Terminate the protocol              */
   } /* else */

} /*sbreak*/

/*--------------------------------------------------------------------*/
/*    s e o f                                                         */
/*                                                                    */
/*    Send End-Of-File                                                */
/*--------------------------------------------------------------------*/

XFER_STATE seof( const boolean purge_file )
{

   struct tm  *tmx;
   long ticks;
   struct timeb now;
   char hostname[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*    Send end-of-file indication, and perhaps receive a              */
/*    lower-layer ACK/NAK                                             */
/*--------------------------------------------------------------------*/

   switch ((*eofpkt)())
   {
      case RETRY:                /* retry */
         printmsg(0, "Remote system asks that the file be resent");
         fseek(xfer_stream, 0L, SEEK_SET);
         bytes = 0;
         (*filepkt)();           /* warmstart file-transfer protocol */
         return XFER_SENDDATA;   /* stay in data phase */

      case FAILED:
         fclose(xfer_stream);
         xfer_stream = NULL;
         return XFER_ABORT;      /* cannot send file */

      case OK:
         fclose(xfer_stream);
         xfer_stream = NULL;
         break;                  /* sent, proceed */

      default:
         fclose(xfer_stream);
         xfer_stream = NULL;
         return XFER_LOST;
   }

   if (!pktgetstr((char *)databuf)) /* Receive CY or CN              */
      return XFER_LOST;       /* Bomb the connection if no packet    */

   if ((*databuf != 'C') || ((databuf[1] != 'N') && (databuf[1] != 'Y')))
   {
      printmsg(0,"Invalid response from remote: %s",
                  ( char *) databuf);
      return XFER_ABORT;
   }

   if (!equaln((char *) databuf, "CY", 2))
      printmsg(0,"seof: Host was unable to save file after transmission");

/*--------------------------------------------------------------------*/
/*                   If local spool file, delete it                   */
/*--------------------------------------------------------------------*/

   importpath(hostname, dname, rmtname);
                              /* Local name also used by logging     */

   if (purge_file && !equal(dname,"D.0"))
   {
     unlink( hostname );
     printmsg(4,"seof: Deleted file %s (%s)", dname, hostname );
   } /* if (purge_file && !equal(dname,"D.0")) */

/*--------------------------------------------------------------------*/
/*                            Update stats                            */
/*--------------------------------------------------------------------*/

   remote_stats.fsent++;
   remote_stats.bsent += bytes;

   if (bflag[F_SYSLOG] || (debuglevel > 2 ))
   {
      ftime(&now);
      ticks = (now.time - start_time.time) * 1000 +
               ((long) now.millitm - (long) start_time.millitm);
      printmsg(2, "Transfer completed, %ld chars/sec",
                  (long) ((bytes * 1000) / (ticks ? ticks : 1) ));

      if (bflag[F_SYSLOG])
      {
         tmx = localtime(&now.time);
         fprintf( syslog,
               "%s!%s (%s) (%d/%d-%02d:%02d:%02d) -> %ld / %ld.%02d secs\n",
                   E_nodename, tname, hostname,
                   (tmx->tm_mon+1), tmx->tm_mday,
                   tmx->tm_hour, tmx->tm_min, tmx->tm_sec, bytes,
                   ticks / 1000 , (int) ((ticks % 1000) / 10) );
      } /* if (bflag[F_SYSLOG] ) */

   } /* if (bflag[F_SYSLOG] || (debuglevel > 2 )) */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return XFER_FILEDONE;    /* go get the next file to send */

} /*seof*/

/*--------------------------------------------------------------------*/
/*    n e w r e q u e s t                                             */
/*                                                                    */
/*    Determine the next request to be sent to other host             */
/*--------------------------------------------------------------------*/

XFER_STATE newrequest( void )
{
   int i;

/*--------------------------------------------------------------------*/
/*                 Verify we have no work in progress                 */
/*--------------------------------------------------------------------*/

   if (!(xfer_stream == NULL))
      return XFER_ABORT;      /* Something is already being
                                 transferred; we're in trouble!      */

/*--------------------------------------------------------------------*/
/*    Look for work in the current call file; if we do not find       */
/*    any, the job is complete and we can delete all the files we     */
/*    worked on in the file                                           */
/*--------------------------------------------------------------------*/

   if (fgets(databuf, BUFSIZ, fwork) == nil(char)) /* More data?     */
   {                          /* No --> clean up list of files       */
      printmsg(3, "newrequest: EOF for workfile %s",workfile);
      fclose(fwork);
      fwork = nil(FILE);
      unlink(workfile);       /* Delete completed call file          */
      return XFER_NEXTJOB;    /* Get next C.* file to process     */
   } /* if (fgets(databuf, BUFSIZ, fwork) == nil(char)) */

/*--------------------------------------------------------------------*/
/*                  We have a new request to process                  */
/*--------------------------------------------------------------------*/

   i = strlen(databuf) - 1;
   printmsg(3, "newrequest: got command from %s",workfile);
   if (databuf[i] == '\n')            /* remove new_line from card */
      databuf[i] = '\0';

   sscanf(databuf, "%c %s %s %*s %s %s",
         &type, fname, tname, cmdopts, dname);

/*--------------------------------------------------------------------*/
/*                           Reset counters                           */
/*--------------------------------------------------------------------*/

   bytes = 0;
   ftime(&start_time);
   (*filepkt)();              /* Init for file transfer */

/*--------------------------------------------------------------------*/
/*             Process the command according to its type              */
/*--------------------------------------------------------------------*/

   switch( type )
   {
      case 'R':
         return XFER_GETFILE;

      case 'S':
         return XFER_PUTFILE;

      default:
         return XFER_FILEDONE;   /* Ignore the line                  */
   } /* switch */

} /* newrequest */

/*--------------------------------------------------------------------*/
/*    s s f i l e                                                     */
/*                                                                    */
/*    Send File Header for file to be sent                            */
/*--------------------------------------------------------------------*/

XFER_STATE ssfile( void )
{
   char hostfile[FILENAME_MAX];
   char *filename;

   if (equal(dname, "D.0"))   /* Is there a spool file?              */
      filename = fname;       /* No --> Use the real name            */
   else
      filename = dname;       /* Yes --> Use it                      */

/*--------------------------------------------------------------------*/
/*              Convert the file name to our local name               */
/*--------------------------------------------------------------------*/

   importpath(hostfile, filename, rmtname);

/*--------------------------------------------------------------------*/
/*    Try to open the file; if we fail, we just continue, because we  */
/*    may have sent the file on a previous call which failed part     */
/*    way through this job                                            */
/*--------------------------------------------------------------------*/

   xfer_stream = FOPEN( hostfile, "r", BINARY );
                                    /* Open stream to send           */
   if (xfer_stream == NULL)
   {
      printmsg(0, "ssfile: Cannot open file %s (%s).", filename, hostfile);
      printerr(hostfile);
      return XFER_FILEDONE;      /* Try next file in this job  */
   } /* if */

/*--------------------------------------------------------------------*/
/*              The file is open, now set its buffering               */
/*--------------------------------------------------------------------*/

   if (setvbuf( xfer_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "ssfile: Cannot unbuffer file %s (%s).",
                  filename, hostfile);
      printerr(hostfile);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;         /* Clearly not our day; quit  */
   } /* if */

/*--------------------------------------------------------------------*/
/*    Okay, we have a file to process; offer it to the other host     */
/*--------------------------------------------------------------------*/

   printmsg(0, "Sending \"%s\" (%s) as \"%s\"", fname, hostfile, tname);
   if (!pktsendstr( databuf ))   /* Tell them what is coming at them */
   {
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_LOST;
   }

   if (!pktgetstr((char *)databuf))
   {
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_LOST;
   }

   if ((*databuf != 'S') || ((databuf[1] != 'N') && (databuf[1] != 'Y')))
   {
      printmsg(0,"Invalid response from remote: %s",databuf);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;
   }

   if (databuf[1] != 'Y')     /* Otherwise reject file transfer?     */
   {                          /* Yes --> Look for next file          */
      printmsg(0, "ssfile: Remote host rejected file %s, reason %s",
                   tname,
                   databuf[2] ? (char *) &databuf[2] : "unknown" );
      fclose( xfer_stream );
      xfer_stream = NULL;
      return XFER_FILEDONE;
   }

   return XFER_SENDDATA;      /* Enter data transmission mode        */

} /*ssfile*/

/*--------------------------------------------------------------------*/
/*    s r f i l e                                                     */
/*                                                                    */
/*    Send File Header for file to be received                        */
/*--------------------------------------------------------------------*/

XFER_STATE srfile( void )
{
   char hostfile[FILENAME_MAX];
   struct  stat    statbuf;

/*--------------------------------------------------------------------*/
/*               Convert the filename to our local name               */
/*--------------------------------------------------------------------*/

   importpath(hostfile, tname, rmtname);

/*--------------------------------------------------------------------*/
/*    If the destination is a directory, put the originating          */
/*    original file name at the end of the path                       */
/*--------------------------------------------------------------------*/

   if ((hostfile[strlen(hostfile) - 1] == '/') ||
       ((stat(hostfile , &statbuf) == 0) && (statbuf.st_mode & S_IFDIR)))
   {
      char *slash = strrchr( fname, '/');

      if ( slash == NULL )
         slash = fname;
      else
         slash ++ ;

      printmsg(3, "srfile: Destination \"%s\" is directory, \
appending filename \"%s\"", hostfile, slash);

      if (hostfile[strlen(hostfile) - 1] != '/')
         strcat(hostfile, "/");

      strcat( hostfile, slash );
   } /* if */

   printmsg(0, "Receiving \"%s\" as \"%s\" (%s)", fname, tname, hostfile);

   if (!pktsendstr( databuf ))
      return XFER_LOST;

   if (!pktgetstr((char *)databuf))
      return XFER_LOST;

   if ((*databuf != 'R') || ((databuf[1] != 'N') && (databuf[1] != 'Y')))
   {
      printmsg(0,"Invalid response from remote: %s",
                  databuf);
      return XFER_ABORT;
   }

   if (databuf[1] != 'Y')     /* Otherwise reject file transfer?     */
   {                          /* Yes --> Look for next file          */
      printmsg(0, "srfile: Remote host denied access to file %s, reason %s",
         fname, databuf[2] ? (char *) &databuf[2] : "unknown" );
      return XFER_FILEDONE;
   }

/*--------------------------------------------------------------------*/
/*    We should verify the directory exists if the user doesn't       */
/*    specify the -d option, but I've got enough problems this        */
/*    week; we'll just auto-create using FOPEN()                      */
/*--------------------------------------------------------------------*/

   xfer_stream = FOPEN(hostfile, "w", BINARY);
                           /* Allow auto-create of directory      */
   if (xfer_stream == NULL)
   {
      printmsg(0, "srfile: cannot create %s", hostfile);
      printerr(hostfile);
      return XFER_ABORT;
   }

/*--------------------------------------------------------------------*/
/*                     Set buffering for the file                     */
/*--------------------------------------------------------------------*/

   if (setvbuf( xfer_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "srfile: Cannot unbuffer file %s (%s).",
          tname, hostfile);
      printerr(hostfile);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;
   } /* if */

   spool = FALSE;             /* Do not rename file at completion */
   return XFER_RECVDATA;      /* Now start receiving the data     */

} /*stfile*/

/*--------------------------------------------------------------------*/
/*    s i n i t                                                       */
/*                                                                    */
/*    Send Initiate:  send this host's parameters and get other       */
/*    side's back.                                                    */
/*--------------------------------------------------------------------*/

XFER_STATE sinit( void )
{
   if ((*openpk)( TRUE ))     /* Initialize in caller mode           */
      return XFER_ABORT;
   else {
      buf_init();
      return XFER_MASTER;
   } /* else */

} /*sinit*/


/*********************** MISC SUB SUB PROTOCOL *************************/

/*
   s c h k d i r

   scan spooling directory for C.* files for the other system
*/

XFER_STATE schkdir( const boolean outbound, const char callgrade )
{
   XFER_STATE c;

   if ( hostp->hsecure->sendfiles || outbound )
                                 /* Send our work to other host?        */
   {
      c = scandir(rmtname,callgrade);
                                 /* Determine if data for the host      */
      scandir( NULL,callgrade ); /* Reset directory search pointers     */
   }
   else {
      hostp->hstatus = called;/* Update host status flags            */
      c = XFER_NOLOCAL;       /* Do not send data on inbound call    */
   }

   switch ( c )
   {
      case XFER_ABORT:        /* Internal error opening file         */
         return XFER_ABORT;

      case XFER_NOLOCAL:      /* No work for host                    */
         if (! pktsendstr("HY") )
            return XFER_LOST;

         if (!pktgetstr((char *)databuf))
            return XFER_LOST; /* Didn't get response, die quietly    */
         else {
            hostp->hstatus = called;/* Update host status flags            */
            return XFER_ENDP; /* Got response, we're out of here     */
         }

      case XFER_REQUEST:
         if (! pktsendstr("HN") )
            return XFER_LOST;
         else {
            printmsg( 2, "schkdir: Switch into master mode" );
            return XFER_MASTER;
         }

      default:
         panic();
         return XFER_ABORT;

   } /* switch */
} /*schkdir*/

/*--------------------------------------------------------------------*/
/*    e n d p                                                         */
/*                                                                    */
/*    end the protocol                                                */
/*--------------------------------------------------------------------*/

XFER_STATE endp( void )
{
   (*closepk)();

   if (spool)
   {
      unlink(tempname);
      spool = FALSE;
   }
   return XFER_EXIT;

} /*endp*/


/*********************** RECIEVE PROTOCOL **********************/

/*--------------------------------------------------------------------*/
/*    r i n i t                                                       */
/*                                                                    */
/*    Receive Initialization                                          */
/*--------------------------------------------------------------------*/

XFER_STATE rinit( void )
{

   if ((*openpk)( FALSE ) == OK )   /* Initialize in callee mode     */
   {
      buf_init();
      return XFER_SLAVE;
   }
   else
      return XFER_LOST;

} /*rinit*/

/*--------------------------------------------------------------------*/
/*    r h e a d e r                                                   */
/*                                                                    */
/*    Receive File Header                                             */
/*--------------------------------------------------------------------*/

XFER_STATE rheader( void )
{

   if (!pktgetstr(databuf))
      return XFER_LOST;

/*--------------------------------------------------------------------*/
/*        Return if the remote system has no more data for us         */
/*--------------------------------------------------------------------*/

   if ((databuf[0] & 0x7f) == 'H')
      return XFER_NOREMOTE;   /* Report master has no more data to   */

/*--------------------------------------------------------------------*/
/*                  Begin transforming the file name                  */
/*--------------------------------------------------------------------*/

   sscanf(databuf, "%c %s %s %*s %s %s",
         &type, fname, tname, cmdopts, dname);

/*--------------------------------------------------------------------*/
/*                           Reset counters                           */
/*--------------------------------------------------------------------*/

   ftime(&start_time);
   bytes = 0;
   (*filepkt)();              /* Init for file transfer */

/*--------------------------------------------------------------------*/
/*                 Return with next state to process                  */
/*--------------------------------------------------------------------*/

   switch (type)
   {
      case 'R':
         return XFER_GIVEFILE;

      case 'S':
         return XFER_TAKEFILE;

      default:
         printmsg(0,"rheader: Unsupported verb \"%c\" rejected",type);
         if (!pktsendstr("XN"))  /* Reject the request               */
            return XFER_LOST;    /* Die if reponse fails             */
         else
            return XFER_FILEDONE;   /* Process next request          */
   } /* switch */

} /* rheader */

/*--------------------------------------------------------------------*/
/*    r r f i l e                                                     */
/*                                                                    */
/*    Setup for receiving a file as requested by the remote host      */
/*--------------------------------------------------------------------*/

XFER_STATE rrfile( void )
{
   char filename[FILENAME_MAX];
   size_t subscript;
   struct  stat    statbuf;

/*--------------------------------------------------------------------*/
/*       Determine if the file can go into the spool directory        */
/*--------------------------------------------------------------------*/

   spool = ((*tname == 'D') || (*tname == 'X')) && tname[1] == '.';

   expand_path( strcpy( filename, tname),
      spool ? "." : securep->pubdir, securep->pubdir , NULL );

/*--------------------------------------------------------------------*/
/*       Check if the name is a directory name (end with a '/')       */
/*--------------------------------------------------------------------*/

   subscript = strlen( filename ) - 1;

   if ((filename[subscript] == '/') ||
       ((stat(filename , &statbuf) == 0) && (statbuf.st_mode & S_IFDIR)))
   {
      char *slash = strrchr(fname, '/');
      if (slash  == NULL)
         slash = fname;
      else
         slash++;

      printmsg(3, "rrfile: destination is directory \"%s\", adding \"%s\"",
               filename, slash);

      if ( filename[ subscript ] != '/')
         strcat(filename, "/");
      strcat(filename, slash);
   } /* if */

/*--------------------------------------------------------------------*/
/*          Let host munge filename as appropriate                    */
/*--------------------------------------------------------------------*/

   importpath(spolname, filename, rmtname);

/*--------------------------------------------------------------------*/
/* If the name has a path and we don't allow it, reject the transfer  */
/*--------------------------------------------------------------------*/

   if ( !spool && !ValidateFile( spolname , ALLOW_WRITE ))
   {
      if (!pktsendstr("SN2")) /* Report access denied to requestor   */
         return XFER_LOST;
      else
         return XFER_FILEDONE;   /* Look for next file from master   */
   } /* if */

/*--------------------------------------------------------------------*/
/*            The filename is transformed, try to open it             */
/*--------------------------------------------------------------------*/

   if (spool)
#ifdef __TURBOC__
      xfer_stream = fopen( tmpnam( tempname ), "wb");
#else

/*--------------------------------------------------------------------*/
/*    MS C 6.0 doesn't generate the name for the current directory,   */
/*    so we cheat and use our own temporary file name generation      */
/*    routine.                                                        */
/*--------------------------------------------------------------------*/

   {
      char *savetemp = E_tempdir;   /* Save the real temp directory  */

      E_tempdir = E_spooldir;       /* Generate this file in spool   */
      mktempname(tempname, "TMP");  /* Get the file name             */
      E_tempdir = savetemp;         /* Restore true directory name   */

      xfer_stream = fopen( tempname , "wb");

   }
#endif

   else if (strchr( cmdopts,'d'))
      xfer_stream = FOPEN( spolname, "w", BINARY );
   else
      xfer_stream = fopen( spolname, "wb");


   if (xfer_stream == NULL)
   {
      printmsg(0, "rrfile: cannot open file %s (%s).",
           filename, spool ? tempname : spolname);
      printerr(spool ? tempname : spolname);
      if (!pktsendstr("SN4"))    /* Report cannot create file     */
         return XFER_LOST;       /* School is out, die            */
      else
         return XFER_FILEDONE;   /* Tell them to send next file   */
   } /* if */

/*--------------------------------------------------------------------*/
/*               The file is open, now try to buffer it               */
/*--------------------------------------------------------------------*/

   if (setvbuf( xfer_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "rrfile: Cannot unbuffer file %s (%s).",
          filename, spool ? tempname : spolname);
      printerr(spool ? tempname : spolname);
      fclose(xfer_stream);
      xfer_stream = NULL;
      pktsendstr("SN4");             /* Report cannot create file     */
      return XFER_ABORT;
   } /* if */

/*--------------------------------------------------------------------*/
/*    Announce we are receiving the file to console and to remote     */
/*--------------------------------------------------------------------*/

   printmsg(0, "Receiving \"%s\" as \"%s\" (%s)",
               fname,filename,spolname);

   if (spool)
      printmsg(2,"Using temp name %s",tempname);

   if (!pktsendstr("SY"))
   {
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_LOST;
   }

   return XFER_RECVDATA;   /* Switch to data state                */

} /*rrfile*/

/*--------------------------------------------------------------------*/
/*    r s f i l e                                                     */
/*                                                                    */
/*    Receive File Header for a file remote has requested us to       */
/*    send                                                            */
/*--------------------------------------------------------------------*/

XFER_STATE rsfile( void )
{
   char filename[FILENAME_MAX];
   char hostname[FILENAME_MAX];
   struct  stat    statbuf;
   size_t subscript;

   expand_path( strcpy(filename, fname ) ,
                securep->pubdir ,
                securep->pubdir ,
                NULL );

/*--------------------------------------------------------------------*/
/*               Let host munge filename as appropriate               */
/*--------------------------------------------------------------------*/

   importpath(hostname, filename, rmtname);
   printmsg(3, "rsfile: input \"%s\", source \"%s\", host \"%s\"",
                                    fname, filename , hostname);

/*--------------------------------------------------------------------*/
/*       Check if the name is a directory name (end with a '/')       */
/*--------------------------------------------------------------------*/

   subscript = strlen( filename ) - 1;

   if ((filename[subscript] == '/') ||
       ((stat(hostname , &statbuf) == 0) && (statbuf.st_mode & S_IFDIR)))
   {
      printmsg(3, "rsfile: source is directory \"%s\", rejecting",
               hostname);

      if (!pktsendstr("RN2"))    /* Report cannot send file       */
         return XFER_LOST;       /* School is out, die            */
      else
         return XFER_FILEDONE;   /* Tell them to send next file   */
   } /* if */

/*--------------------------------------------------------------------*/
/*                Check the access to the file desired                */
/*--------------------------------------------------------------------*/

   if ( !ValidateFile( hostname , ALLOW_READ ))
   {
      if (!pktsendstr("RN2")) /* Report access denied to requestor   */
         return XFER_LOST;
      else
         return XFER_FILEDONE;   /* Look for next file from master   */
   } /* if */

/*--------------------------------------------------------------------*/
/*            The filename is transformed, try to open it             */
/*--------------------------------------------------------------------*/

   xfer_stream = FOPEN( hostname, "r" , BINARY);
                              /* Open stream to transmit       */
   if (xfer_stream == NULL)
   {
      printmsg(0, "rsfile: Cannot open file %s (%s).", fname, hostname);
      printerr(hostname);
      if (!pktsendstr("RN2"))    /* Report cannot send file       */
         return XFER_LOST;       /* School is out, die            */
      else
         return XFER_FILEDONE;   /* Tell them to send next file   */
   } /* if */

   if (setvbuf( xfer_stream, NULL, _IONBF, 0))
   {
      printmsg(0, "rsfile: Cannot unbuffer file %s (%s).", fname, hostname);
      pktsendstr("RN2");         /* Tell them we cannot handle it */
      printerr(hostname);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;
   } /* if */

/*--------------------------------------------------------------------*/
/*  We have the file open, announce it to the log and to the remote   */
/*--------------------------------------------------------------------*/

   if (!pktsendstr("RY"))
   {
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_LOST;
   }

   printmsg(0, "Sending \"%s\" (%s) as \"%s\"", fname, hostname, tname);

   return XFER_SENDDATA;   /* Switch to send data state        */

} /*rsfile*/

/*--------------------------------------------------------------------*/
/*    r d a t a                                                       */
/*                                                                    */
/*    Receive Data                                                    */
/*--------------------------------------------------------------------*/

XFER_STATE rdata( void )
{
   size_t len;
   size_t used = 0;

   do {

      if ((*getpkt)((char *) (databuf + used), &len) != OK)
      {
         fclose(xfer_stream);
         xfer_stream = NULL;
         return XFER_LOST;
      }
      else
         used += len;

   }  while (((used - pktsize) <= xfer_bufsize) && len);

/*--------------------------------------------------------------------*/
/*                  Write incoming data to the file                   */
/*--------------------------------------------------------------------*/

   if (used && (bufwrite((char *) databuf, used) < (int) used))
   {                                                        /* ahd   */
      printmsg(0, "rdata: Error writing data to file.");
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;
   }

/*--------------------------------------------------------------------*/
/*                         Handle end of file                         */
/*--------------------------------------------------------------------*/

   if (len == 0)
      return XFER_RECVEOF;
   else
      return XFER_RECVDATA;      /* Remain in data state                */

} /*rdata*/

/*--------------------------------------------------------------------*/
/*    r e o f                                                         */
/*                                                                    */
/*    Process EOF for a received file                                 */
/*--------------------------------------------------------------------*/

XFER_STATE reof( void )
{
   struct tm *tmx;
   struct timeb now;
   long ticks;
   char *cy = "CY";
   char *cn = "CN";
   char *response = cy;
   char *fname = spool ? tempname : spolname;

/*--------------------------------------------------------------------*/
/*            Close out the file, checking for I/O errors             */
/*--------------------------------------------------------------------*/

   fclose(xfer_stream);
   if (ferror (xfer_stream ))
   {
      response = cn;          /* Report we had a problem             */
      printerr( fname );
   }

   xfer_stream = NULL;        /* To make sure!                       */

/*--------------------------------------------------------------------*/
/*    If it was a spool file, rename it to its permanent location     */
/*--------------------------------------------------------------------*/

   if (spool && equal(response,cy))
   {
      unlink( spolname );     /* Should be safe, since we only do it
                                 for spool files                     */

      if ( RENAME(tempname, spolname ))
      {
         printmsg(0,"reof: Unable to rename %s to %s",
                  tempname, spolname);
         response = cn;
         printerr(spolname);
      } /* if ( RENAME(tempname, spolname )) */
      spool = FALSE;
   } /* if (equal(response,cy) && spool) */

   if (!pktsendstr(response)) /* Announce we accepted the file       */
      return XFER_LOST;       /* No ACK?  Return, if so              */

   if ( !equal(response, cy) )   /* If we had an error, delete file  */
   {
      printmsg(0,"reof: Deleting corrupted file %s", fname );
      unlink(fname );
      return XFER_ABORT;
   } /* if ( !equal(response, cy) ) */

/*--------------------------------------------------------------------*/
/*            The file is delivered; compute stats for it             */
/*--------------------------------------------------------------------*/

   remote_stats.freceived++;
   remote_stats.breceived += bytes;

   if (bflag[F_SYSLOG] || (debuglevel > 2 ))
   {
      ftime(&now);
      ticks = (now.time - start_time.time) * 1000 +
               ((long) now.millitm - (long) start_time.millitm);
      printmsg(2, "Transfer completed, %ld chars/sec",
                  (long) ((bytes * 1000) / (ticks ? ticks : 1) ));

      if ( bflag[F_SYSLOG] )
      {
         tmx = localtime(&now.time);
         fprintf( syslog,
            "%s!%s (%s) (%d/%d-%02d:%02d:%02d) <- %ld / %ld.%02d secs\n",
                   rmtname, tname, spolname,
                   (tmx->tm_mon+1), tmx->tm_mday,
                   tmx->tm_hour, tmx->tm_min, tmx->tm_sec, bytes,
                   ticks / 1000 , (int) ((ticks % 1000) / 10) );
      } /* if ( bflag[SYSLOG ) */
   } /* if (bflag[F_SYSLOG] || (debuglevel > 2 )) */

/*--------------------------------------------------------------------*/
/*                      Return success to caller                      */
/*--------------------------------------------------------------------*/

   return XFER_FILEDONE;
} /* reof */

/*--------------------------------------------------------------------*/
/*                           MISC ROUTINES                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    p k t s e n d s t r                                             */
/*                                                                    */
/*    Transmit a control packet                                       */
/*--------------------------------------------------------------------*/

static boolean pktsendstr( char *s )
{
   printmsg(2, ">>> %s", s);
   if ( ! bflag[ F_MULTITASK ] )
      fflush( logfile );         /* Known safe place  to flush log      */

   if((*wrmsg)(s) != OK )
      return FALSE;

   remote_stats.bsent += strlen(s)+1;

   return TRUE;
} /* pktsendstr */

/*--------------------------------------------------------------------*/
/*    p k t g e t s t r                                               */
/*                                                                    */
/*    Receive a control packet                                        */
/*--------------------------------------------------------------------*/

static boolean pktgetstr( char *s)
{
   if ((*rdmsg)(s) != OK )
     return FALSE;

   remote_stats.breceived += strlen( s ) + 1;
   printmsg(2, "<<< %s", s);

   return TRUE;
} /* pktgetstr */

/*--------------------------------------------------------------------*/
/*    b u f _ i n i t                                                 */
/*                                                                    */
/*    Alocate buffers for file transfer                               */
/*--------------------------------------------------------------------*/

static void buf_init( void )
{
   xfer_bufsize = max( pktsize * 4, max( M_xfer_bufsize, BUFSIZ) );

   if (databuf == NULL)
      databuf = malloc( xfer_bufsize );
   else
      databuf = realloc( databuf, xfer_bufsize );
   checkref( databuf );

} /* buf_init */
