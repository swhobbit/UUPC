/*--------------------------------------------------------------------*/
/*    d c p x f e r . c                                               */
/*                                                                    */
/*    Procotol independent transfer level for UUCICO                  */
/*                                                                    */
/*    Stuart Lynne May/87                                             */
/*                                                                    */
/*    Copyright (c) Richard H. Lamb 1985, 1986, 1987                  */
/*    Changes Copyright (c) Stuart Lynne 1987                         */
/*    Changes Copyright (c) Jordan Brown 1990, 1991                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*
 *       $Id: dcpxfer.c 1.63 2001/03/12 13:55:29 ahd v1-13k $
 *
 *       $Log: dcpxfer.c $
 *       Revision 1.63  2001/03/12 13:55:29  ahd
 *       Annual copyright update
 *
 *       Revision 1.62  2000/05/12 12:32:55  ahd
 *       Annual copyright update
 *
 *       Revision 1.61  1999/01/07 02:28:51  ahd
 *       Convert to RCSID format
 *
 *       Revision 1.60  1999/01/04 03:53:57  ahd
 *       Annual copyright change
 *
 *       Revision 1.59  1998/03/01 01:39:46  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.58  1997/04/24 01:34:08  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.57  1996/11/18 04:46:49  ahd
 *       Normalize arguments to bugout
 *       Reset title after exec of sub-modules
 *       Normalize host status names to use HS_ prefix
 *
 *       Revision 1.56  1996/01/20 12:55:34  ahd
 *       After rejected transfer from spool directory, delete temp copy of file
 *
 *       Revision 1.55  1996/01/01 21:22:21  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.54  1995/04/02 00:01:39  ahd
 *       Correct processing to not send files below requested call grade
 *
 *       Revision 1.53  1995/03/11 22:29:21  ahd
 *       Use macro for file delete to allow special OS/2 processing
 *
 *       Revision 1.52  1995/03/11 15:49:23  ahd
 *       Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *       Revision 1.51  1995/02/21 02:47:44  ahd
 *       The compiler warnings war never ends!
 *
 *       Revision 1.50  1995/02/14 04:38:42  ahd
 *       Correct problems with directory processing under NT
 *
 *       Revision 1.49  1995/01/07 16:38:55  ahd
 *       Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *       Revision 1.48  1994/12/27 20:45:50  ahd
 *       Smoother call grading'
 *
 *       Revision 1.47  1994/12/22 00:35:22  ahd
 *       Annual Copyright Update
 *
 *       Revision 1.46  1994/08/07 21:45:09  ahd
 *       Initialize process ID properly when receiving files
 *
 *        Revision 1.45  1994/06/05  01:57:26  ahd
 *        Truncate invalid responses after one line of data, more or less
 *
 * Revision 1.44  1994/03/05  21:12:05  ahd
 * Do not allow anonymous systems to feed us files (or cause
 * us to abort processing)
 *
 * Revision 1.43  1994/02/26  17:20:48  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 * Revision 1.42  1994/02/20  19:11:18  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.41  1994/02/19  05:08:15  ahd
 * Use standard first header
 *
 * Revision 1.40  1994/01/01  19:19:51  ahd
 * Annual Copyright Update
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <sys/timeb.h>
#include <sys/stat.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dcp.h"
#include "dcpsys.h"
#include "dcpxfer.h"
#include "expath.h"
#include "hostable.h"
#include "import.h"
#include "security.h"
#include "modem.h"
#include "commlib.h"          /* For MAXPACK                         */

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char *databuf = NULL;
static unsigned int xferBufSize = 0;

static char fName[FILENAME_MAX], tName[FILENAME_MAX], dName[FILENAME_MAX];
static char *lName;           /* Name to report in syslog             */
static char type, cmdopts[16];
static char *syslogName = NULL;

static long bytes;
static unsigned long fileSize;
static struct timeb startTime;

static KWBoolean spool = KWFalse; /* Received file is into spool dir   */
static char spolName[FILENAME_MAX];
                              /* Final host name of file to be
                                 received into spool directory       */
static char tempName[FILENAME_MAX];
                              /* Temp name used to create received
                                 file                                */
static char userid[20];
static int seq = 0;           /* Number of files transfered this
                                 connection                          */
static int pid;
static size_t vbufsize;       /* Amount to buffer in std library     */

RCSID("$Id: dcpxfer.c 1.63 2001/03/12 13:55:29 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static KWBoolean pktgetstr( char *s);
static KWBoolean pktsendstr( char *s );
static void buf_init( void );

static int  bufill(char  *buffer);
static int  bufwrite(char  *buffer,int  len);

/* ************** SEND PROTOCOL ************************** */

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
      short xmit = (short) min( (size_t) (S_size - used), s_pktsize );

      if ((*sendpkt)((char *) databuf + used, xmit) != DCP_OK)
                                    /* Send data fail?            */
      {
         fclose( xfer_stream );
         xfer_stream = NULL;
         return XFER_LOST;    /* Trouble!                         */
      }
      else
         used += xmit;

   } while( S_size > used );

   return XFER_SENDDATA;   /* Remain in send state                */

} /* sdata */

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
                        xferBufSize,
                        xfer_stream);

   bytes += (int) count;

   if ((count < xferBufSize) && ferror(xfer_stream))
   {
      printerr("bufill");
      clearerr(xfer_stream);
      return -1;
   }

   return (int) count;

} /* bufill */

/*--------------------------------------------------------------------*/
/*    b u f w r i t e                                                 */
/*                                                                    */
/*    Write a bufferful of data to the file that's being received.    */
/*--------------------------------------------------------------------*/

static int bufwrite(char *buffer, int len)
{
   int count = (int) fwrite(buffer, sizeof *buffer, (size_t) len, xfer_stream);

   bytes += count;

   if (count < len)
   {
      printerr("bufwrite");
      printmsg(0, "bufwrite: Tried to write %d bytes, actually wrote %d",
        len, count);
      clearerr(xfer_stream);
   }

   return count;

} /* bufwrite */

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
      return XFER_LOST;       /* Xmit fail?  If so, quit transmitting */

   if (!pktgetstr((char *)databuf)) /* Get their response            */
      return XFER_LOST;       /*  Xmit fail?  If so, quit transmitting */

   if ((*databuf != 'H') || ((databuf[1] != 'N') && (databuf[1] != 'Y')))
   {
      printmsg(0,"Invalid response from remote: %.50s",databuf);
      return XFER_ABORT;
   }

   if (databuf[1] == 'N')     /* "HN" (have work) message from host? */
   {                          /* Yes --> Enter Receive mode          */
      printmsg( bflag[F_SHOWSPOOL] ? 2 : 1,
                "sbreak: Switch into slave mode" );
      return XFER_SLAVE;
   }
   else {                     /* No --> Remote host is done as well  */
      pktsendstr("HY");       /* Tell the host we are done as well   */
      hostp->status.hstatus = HS_CALLED;/* Update host status flags  */
      return XFER_ENDP;       /* Terminate the protocol              */
   } /* else */

} /* sbreak */

/*--------------------------------------------------------------------*/
/*    s e o f                                                         */
/*                                                                    */
/*    Send End-Of-File                                                */
/*--------------------------------------------------------------------*/

XFER_STATE seof( const KWBoolean purge_file )
{

   struct tm  *tmx;
   long ticks;
   struct timeb now;

/*--------------------------------------------------------------------*/
/*    Send end-of-file indication, and perhaps receive a              */
/*    lower-layer ACK/NAK                                             */
/*--------------------------------------------------------------------*/

   switch ((*eofpkt)())
   {
      case DCP_RETRY:            /* retry */
         printmsg(0, "Remote system asks that the file be resent");
         fseek(xfer_stream, 0L, SEEK_SET);
         bytes = 0;
         (*filepkt)(KWTrue, fileSize);
                                 /* Warmstart file-transfer protocol */
         return XFER_SENDDATA;   /* stay in data phase */

      case DCP_FAILED:
         fclose(xfer_stream);
         xfer_stream = NULL;
         return XFER_ABORT;      /* cannot send file */

      case DCP_OK:
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
      printmsg(0,"Invalid response from remote: %.50s",
                  ( char *) databuf);
      return XFER_ABORT;
   }

   if (!equaln((char *) databuf, "CY", 2))
      printmsg(0,"seof: Host was unable to save file after transmission");

/*--------------------------------------------------------------------*/
/*                   If local spool file, delete it                   */
/*--------------------------------------------------------------------*/

   if (purge_file && !equal(dName,"D.0"))
   {
     char hostName[FILENAME_MAX];
     importpath(hostName, dName, rmtname);
     REMOVE( hostName );
     printmsg(4,"seof: Deleted file %s (%s)", dName, hostName );
   } /* if (purge_file && !equal(dName,"D.0")) */

/*--------------------------------------------------------------------*/
/*                            Update stats                            */
/*--------------------------------------------------------------------*/

   remote_stats.fsent++;
   remote_stats.bsent += bytes;

   if (bflag[F_SYSLOG] || (debuglevel > 2 ))
   {
      ftime(&now);
      ticks = (now.time - startTime.time) * 1000 +
               ((long) now.millitm - (long) startTime.millitm);
      printmsg(2, "Transfer completed, %ld chars/sec",
                  (long) ((bytes * 1000) / (ticks ? ticks : 1) ));

      if (bflag[F_SYSLOG])
      {
         tmx = localtime(&now.time);
         seq++;
         if ( bflag[F_MULTITASK] )
            syslog = FOPEN(syslogName, "a",TEXT_MODE);

         if ( syslog == NULL )
            printerr(syslogName);
#ifndef _Windows
         else if ((bflag[F_MULTITASK] && setvbuf( syslog, NULL, _IONBF, 0)))
            printerr(syslogName);
#endif
         else {
            fprintf( syslog,
                   "%s!%s %c %s (%d/%d-%02d:%02d:%02d) (C,%d,%d) [%s]"
                         " -> %ld / %ld.%02d secs\n",
                   hostp->via,
                   userid,
                   type,
                   lName,
                   (tmx->tm_mon+1), tmx->tm_mday,
                   tmx->tm_hour, tmx->tm_min, tmx->tm_sec,
                   pid,
                   seq,
                   M_device,
                   bytes,
                   ticks / 1000 ,
                   (int) ((ticks % 1000) / 10) );

            if ( bflag[F_MULTITASK] )
            {
               fclose( syslog );
               syslog = NULL;
            }
         }

      } /* if (bflag[F_SYSLOG]) */

   } /* if (bflag[F_SYSLOG] || (debuglevel > 2 )) */

/*--------------------------------------------------------------------*/
/*                      Return success to caller                      */
/*--------------------------------------------------------------------*/

   return XFER_FILEDONE;    /* go get the next file to process */

} /* seof */

/*--------------------------------------------------------------------*/
/*    n e w r e q u e s t                                             */
/*                                                                    */
/*    Determine the next request to be sent to other host             */
/*--------------------------------------------------------------------*/

XFER_STATE newrequest( void )
{
   size_t len;

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

   if (fgets(databuf, (int) xferBufSize, fwork) == nil(char))
                              /* More data?                          */
   {                          /* No --> clean up list of files       */
      printmsg(3, "newrequest: EOF for workfile %s",workfile);
      fclose(fwork);
      fwork = nil(FILE);
      REMOVE(workfile);       /* Delete completed call file          */
      return XFER_NEXTJOB;    /* Get next C.* file to process        */

   } /* if (fgets(databuf, xferBufSize, fwork) == nil(char)) */

/*--------------------------------------------------------------------*/
/*                  We have a new request to process                  */
/*--------------------------------------------------------------------*/

   printmsg(3, "newrequest: got command from %s", workfile);
   len = strlen(databuf) - 1;

   if (databuf[len] == '\n')            /* remove new_line from card */
      databuf[len] = '\0';

   *cmdopts = *dName = '\0';

   sscanf(databuf, "%c %s %s %s %s %s",
         &type, fName, tName, spolName, cmdopts, dName);

   if ( !strlen( dName ))
      strcpy( dName, "D.0");

   spolName[ sizeof userid - 1] = '\0';
   strcpy( userid, spolName );

/*--------------------------------------------------------------------*/
/*                           Reset counters                           */
/*--------------------------------------------------------------------*/

   bytes = 0;
   ftime(&startTime);

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
   char hostFile[FILENAME_MAX];
   char *fileName;

/*--------------------------------------------------------------------*/
/*              Convert the file name to our local name               */
/*--------------------------------------------------------------------*/

   if (equal(dName, "D.0"))   /* Is there a spool file?              */
   {
      fileName = fName;       /* No --> Use the real name            */
      strcpy( hostFile, fileName );

      if (!ValidateFile( hostFile , ALLOW_READ ))
         return XFER_FILEDONE;   /* Look for next file in our queue  */
   }
   else {
      fileName = dName;       /* Yes --> Use it                      */
      importpath(hostFile, fileName, rmtname);  /* And map to local  */
   }

   lName = fName;             /* Always log the real name             */

/*--------------------------------------------------------------------*/
/*    Try to open the file; if we fail, we just continue, because we  */
/*    may have sent the file on a previous call which failed part     */
/*    way through this job                                            */
/*--------------------------------------------------------------------*/

   xfer_stream = FOPEN( hostFile, "r", IMAGE_MODE);
                                    /* Open stream to send           */
   if (xfer_stream == NULL)
   {
      printmsg(0, "ssfile: Cannot open file %s (%s) from %s.",
                           fileName,
                           hostFile,
                           E_cwd );

      printerr(hostFile);
      return XFER_FILEDONE;      /* Try next file in this job  */
   } /* if */

/*--------------------------------------------------------------------*/
/*              The file is open, now set its buffering               */
/*--------------------------------------------------------------------*/

   if (setvbuf( xfer_stream, NULL, vbufsize ? _IOFBF : _IONBF, vbufsize))
   {
      printmsg(0, "ssfile: Cannot unbuffer file %s (%s).",
                  fileName, hostFile);
      printerr(hostFile);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;         /* Clearly not our day; quit  */
   } /* if */

/*--------------------------------------------------------------------*/
/*    Okay, we have a file to process; offer it to the other host     */
/*--------------------------------------------------------------------*/

   printmsg( (equal(fName,dName) && !bflag[F_SHOWSPOOL]) ? 2 : 0,
            "Sending \"%s\" (%s) as \"%s\"",
            fName,
            hostFile,
            tName);

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
      printmsg(0,"Invalid response from remote: %.50s",databuf);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;
   }

   if (databuf[1] != 'Y')     /* Otherwise reject file transfer?     */
   {                          /* Yes --> Look for next file          */
      printmsg(0, "ssfile: Remote host rejected file %s, reason %s",
                   tName,
                   databuf[2] ? (char *) &databuf[2] : "unknown" );
      fclose( xfer_stream );
      xfer_stream = NULL;

      if (! equal(dName, "D.0"))    /* Is there a spool file?        */
         REMOVE( hostFile );        /* yes --> Nuke it !             */

      return XFER_FILEDONE;
   }

   fileSize = (unsigned long) filelength( fileno( xfer_stream ) );
   (*filepkt)(KWTrue, fileSize);
                              /* Init for file transfer             */

   return XFER_SENDDATA;      /* Enter data transmission mode        */

} /* ssfile */

/*--------------------------------------------------------------------*/
/*    s r f i l e                                                     */
/*                                                                    */
/*    Send File Header for file to be received                        */
/*--------------------------------------------------------------------*/

XFER_STATE srfile( void )
{
   struct  stat    statbuf;

/*--------------------------------------------------------------------*/
/*               Convert the filename to our local name               */
/*--------------------------------------------------------------------*/

   strcpy( spolName, normalize(tName));
                                    /* Assume the local user can type */

/*--------------------------------------------------------------------*/
/*    If the destination is a directory, put the originating          */
/*    original file name at the end of the path                       */
/*--------------------------------------------------------------------*/

   if ((spolName[strlen(spolName) - 1] == '/') ||
       ((stat(spolName , &statbuf) == 0) && (statbuf.st_mode & S_IFDIR)))
   {
      char *slash = strrchr( fName, '/');

      if ( slash == NULL )
         slash = fName;
      else
         slash ++ ;

      printmsg(3, "srfile: Destination \"%s\" is directory, \
appending file name \"%s\"", spolName, slash);

      if (spolName[strlen(spolName) - 1] != '/')
         strcat(spolName, "/");

      strcat( spolName, slash );
   } /* if */

   printmsg(0, "Receiving \"%s\" as \"%s\" (%s)", fName, tName, spolName);

/*--------------------------------------------------------------------*/
/*                    Validate receiving of the file                  */
/*--------------------------------------------------------------------*/

   if (!ValidateFile( spolName , ALLOW_WRITE ))
   {
      return XFER_FILEDONE;      /* Look for next file from master   */
   } /* if */

/*--------------------------------------------------------------------*/
/*        Send the request to the remote and get the response         */
/*--------------------------------------------------------------------*/

   if (!pktsendstr( databuf ))
      return XFER_LOST;

   if (!pktgetstr((char *)databuf))
      return XFER_LOST;

   if ((*databuf != 'R') || ((databuf[1] != 'N') && (databuf[1] != 'Y')))
   {
      printmsg(0,"Invalid response from remote: %.50s",
                  databuf);
      return XFER_ABORT;
   }

   if (databuf[1] != 'Y')     /* Otherwise reject file transfer?     */
   {                          /* Yes --> Look for next file          */
      printmsg(0, "srfile: Remote host denied access to file %s, reason %s",
         fName, databuf[2] ? (char *) &databuf[2] : "unknown" );
      return XFER_FILEDONE;
   }

/*--------------------------------------------------------------------*/
/*    We should verify the directory exists if the user doesn't       */
/*    specify the -d option, but I've got enough problems this        */
/*    week; we'll just auto-create using FOPEN()                      */
/*--------------------------------------------------------------------*/

   xfer_stream = FOPEN(spolName, "w", IMAGE_MODE);
                           /* Allow auto-create of directory      */
   if (xfer_stream == NULL)
   {
      printmsg(0, "srfile: cannot create %s", spolName);
      printerr(spolName);
      return XFER_ABORT;
   }

/*--------------------------------------------------------------------*/
/*                     Set buffering for the file                     */
/*--------------------------------------------------------------------*/

   if (setvbuf( xfer_stream, NULL, vbufsize ? _IOFBF : _IONBF, vbufsize))
   {
      printmsg(0, "srfile: Cannot unbuffer file %s (%s).",
          tName, spolName);
      printerr(spolName);
      REMOVE(spolName);
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_ABORT;
   } /* if */

   spool = KWFalse;            /* Do not rename file at completion */
   lName = spolName;          /* Use full name for local logging      */

   (*filepkt)(KWFalse, 0);     /* Init for file transfer              */

   return XFER_RECVDATA;      /* Now start receiving the data     */

} /* srfile */

/*--------------------------------------------------------------------*/
/*    s i n i t                                                       */
/*                                                                    */
/*    Send Initiate:  send this host's parameters and get other       */
/*    side's back.                                                    */
/*--------------------------------------------------------------------*/

XFER_STATE sinit( void )
{

   if ((*openpk)( KWTrue ))    /* Initialize in caller mode           */
      return XFER_ABORT;
   else {
      buf_init();
      return XFER_MASTER;
   } /* else */

} /* sinit */

/* ********************** MISC SUB SUB PROTOCOL ************************ */

/*--------------------------------------------------------------------*/
/*    s c h k d i r                                                   */
/*                                                                    */
/*    Scan spooling directory for C.* files for the other system      */
/*--------------------------------------------------------------------*/

XFER_STATE schkdir( const KWBoolean outbound, const char callgrade )
{
   XFER_STATE c;

   if ( hostp->hsecure->sendfiles || outbound )
                                 /* Send our work to other host?      */
   {
      c = scandir(rmtname,callgrade);
                                 /* Determine if data for the host    */
      scandir( NULL,callgrade ); /* Reset directory search pointers   */
   }
   else {
      hostp->status.hstatus = HS_CALLED;/* Update host status flags  */
      c = XFER_NOLOCAL;       /* Do not send data on inbound call    */
   }

   switch ( c )
   {
      case XFER_ABORT:        /* Internal error opening file         */
         return XFER_ABORT;

      case XFER_NEXTGRADE:    /* No work for host                    */
      case XFER_NOLOCAL:      /* No work for host                    */
         if (! pktsendstr("HY") )
            return XFER_LOST;

         if (!pktgetstr((char *)databuf))
            return XFER_LOST; /* Didn't get response, die quietly    */
         else {
            hostp->status.hstatus = HS_CALLED;/* Update host status  */
            return XFER_ENDP; /* Got response, we're out of here     */
         }

      case XFER_REQUEST:
         if (! pktsendstr("HN") )
            return XFER_LOST;
         else {
            printmsg( bflag[F_SHOWSPOOL] ? 2 : 1,
                      "schkdir: Switch into master mode" );
            return XFER_MASTER;
         }

      default:
         panic();
         return XFER_ABORT;

   } /* switch */

} /* schkdir */

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
      REMOVE(tempName);
      spool = KWFalse;
   }
   return XFER_EXIT;

} /* endp */

/* ********************** RECIEVE PROTOCOL ********************* */

/*--------------------------------------------------------------------*/
/*    r i n i t                                                       */
/*                                                                    */
/*    Receive Initialization                                          */
/*--------------------------------------------------------------------*/

XFER_STATE rinit( void )
{

   if ((*openpk)( KWFalse ) == DCP_OK )  /* Initialize in callee mode */
   {
      buf_init();
      return XFER_SLAVE;
   }
   else
      return XFER_LOST;

} /* rinit */

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

   *cmdopts = *dName = '\0';

   sscanf(databuf, "%c %s %s %s %s %s",
         &type, fName, tName, spolName, cmdopts, dName);

   if ( !strlen( dName ))
      strcpy( dName, "D.0");

   spolName[ sizeof userid - 1] = '\0';
   strcpy( userid, spolName );

/*--------------------------------------------------------------------*/
/*                           Reset counters                           */
/*--------------------------------------------------------------------*/

   ftime(&startTime);
   bytes = 0;

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
   char fileName[FILENAME_MAX];
   size_t subscript;
   struct  stat    statbuf;

/*--------------------------------------------------------------------*/
/*       Determine if the file can go into the spool directory        */
/*--------------------------------------------------------------------*/

   if ( isupper(*tName) &&
        (tName[1] == '.') &&
        (strchr(tName,'/') == NULL ) &&
        (strchr(tName,'\\') == NULL ))
      spool = KWTrue;
   else
      spool = KWFalse;

   strcpy( fileName, tName );

   if ( ! spool )
      expand_path( fileName, securep->pubdir, securep->pubdir , NULL );

/*--------------------------------------------------------------------*/
/*         Don't allow transfers into spool of anonymous host         */
/*--------------------------------------------------------------------*/

   if ( spool && equal(rmtname, ANONYMOUS_HOST ))
   {
      if (!pktsendstr("SN2")) /* Report access denied to requestor   */
         return XFER_LOST;
      else
         return XFER_FILEDONE;   /* Look for next file from master   */
   } /* if */

/*--------------------------------------------------------------------*/
/*       Check if the name is a directory name (end with a '/')       */
/*--------------------------------------------------------------------*/

   subscript = strlen( fileName ) - 1;

   if ((fileName[subscript] == '/') ||
       ((stat(fileName , &statbuf) == 0) && (statbuf.st_mode & S_IFDIR)))
   {
      char *slash = strrchr(fName, '/');
      if (slash  == NULL)
         slash = fName;
      else
         slash++;

      printmsg(3, "rrfile: destination is directory \"%s\", adding \"%s\"",
               fileName, slash);

      if ( fileName[ subscript ] != '/')
         strcat(fileName, "/");
      strcat(fileName, slash);
   } /* if */

/*--------------------------------------------------------------------*/
/*          Let host munge filename as appropriate                    */
/*--------------------------------------------------------------------*/

   importpath(spolName, fileName, rmtname);

/*--------------------------------------------------------------------*/
/*       If the name has a path and we don't allow it, reject the     */
/*       transfer.  We also reject attempts to send call files,       */
/*       because they would bypass security.                          */
/*--------------------------------------------------------------------*/

   if (( !spool && !ValidateFile( spolName , ALLOW_WRITE )) ||
       ( spool && (*tName == 'C' )))
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
   {
      char *p = tmpnam( tempName );
      denormalize( p );
      xfer_stream = fopen( p, "wb");
   }
#else

/*--------------------------------------------------------------------*/
/*    MS C 6.0 doesn't generate the name for the current directory,   */
/*    so we cheat and use our own temporary file name generation      */
/*    routine.                                                        */
/*--------------------------------------------------------------------*/

   {
      char *savetemp = E_tempdir;   /* Save the real temp directory  */

      E_tempdir = E_spooldir;       /* Generate this file in spool   */
      mktempname(tempName, "tmp");  /* Get the file name             */
      E_tempdir = savetemp;         /* Restore true directory name   */

      denormalize( tempName );
      xfer_stream = fopen( tempName , "wb");

   }
#endif

   else if (strchr( cmdopts,'d'))
      xfer_stream = FOPEN( spolName, "w", IMAGE_MODE);
   else {
      denormalize( spolName );
      xfer_stream = fopen( spolName, "wb");
   }

   if (xfer_stream == NULL)
   {
      printmsg(0, "rrfile: cannot open file %s (%s).",
           fileName, spool ? tempName : spolName);
      printerr(spool ? tempName : spolName);
      if (!pktsendstr("SN4"))    /* Report cannot create file     */
         return XFER_LOST;       /* School is out, die            */
      else
         return XFER_FILEDONE;   /* Tell them to send next file   */
   } /* if */

/*--------------------------------------------------------------------*/
/*               The file is open, now try to buffer it               */
/*--------------------------------------------------------------------*/

   if (setvbuf( xfer_stream, NULL, vbufsize ? _IOFBF : _IONBF, vbufsize))
   {
      printmsg(0, "rrfile: Cannot unbuffer file %s (%s).",
          fileName, spool ? tempName : spolName);
      printerr(spool ? tempName : spolName);
      fclose(xfer_stream);
      xfer_stream = NULL;
      pktsendstr("SN4");             /* Report cannot create file     */
      return XFER_ABORT;
   } /* if */

/*--------------------------------------------------------------------*/
/*    Announce we are receiving the file to console and to remote     */
/*--------------------------------------------------------------------*/

   printmsg((spool && !bflag[F_SHOWSPOOL]) ? 2 : 0 ,
               "Receiving \"%s\" as \"%s\" (%s)",
               fName,
               fileName,
               spolName);

   if (spool)
      printmsg(2,"Using temp name %s",tempName);

   if (!pktsendstr("SY"))
   {
      fclose(xfer_stream);
      xfer_stream = NULL;
      return XFER_LOST;
   }

   lName = spool ? tName : spolName;   /* choose name to log          */

   (*filepkt)(KWFalse, 0);          /* Init for file transfer         */

   return XFER_RECVDATA;            /* Switch to data state           */

} /* rrfile */

/*--------------------------------------------------------------------*/
/*    r s f i l e                                                     */
/*                                                                    */
/*    Receive File Header for a file remote has requested us to       */
/*    send                                                            */
/*--------------------------------------------------------------------*/

XFER_STATE rsfile( void )
{
   char fileName[FILENAME_MAX];
   struct  stat    statbuf;
   size_t subscript;

   expand_path( strcpy(fileName, fName ) ,
                securep->pubdir ,
                securep->pubdir ,
                NULL );

/*--------------------------------------------------------------------*/
/*               Let host munge filename as appropriate               */
/*--------------------------------------------------------------------*/

   strcpy( spolName, fileName );    /* Assume remote can type ...     */
                                    /* don't munge the file they want */
                                    /* sent                           */

/*--------------------------------------------------------------------*/
/*       Check if the name is a directory name (end with a '/')       */
/*--------------------------------------------------------------------*/

   subscript = strlen( fileName ) - 1;

   if ((fileName[subscript] == '/') ||
       ((stat(spolName , &statbuf) == 0) && (statbuf.st_mode & S_IFDIR)))
   {
      printmsg(3, "rsfile: source is directory \"%s\", rejecting",
               spolName);

      if (!pktsendstr("RN2"))    /* Report cannot send file       */
         return XFER_LOST;       /* School is out, die            */
      else
         return XFER_FILEDONE;   /* Tell them to send next file   */
   } /* if */

/*--------------------------------------------------------------------*/
/*                Check the access to the file desired                */
/*--------------------------------------------------------------------*/

   if ( !ValidateFile( spolName , ALLOW_READ ))
   {
      if (!pktsendstr("RN2")) /* Report access denied to requestor   */
         return XFER_LOST;
      else
         return XFER_FILEDONE;   /* Look for next file from master   */
   } /* if */

/*--------------------------------------------------------------------*/
/*            The filename is transformed, try to open it             */
/*--------------------------------------------------------------------*/

   xfer_stream = FOPEN( spolName, "r" , IMAGE_MODE);
                              /* Open stream to transmit       */
   if (xfer_stream == NULL)
   {
      printmsg(0, "rsfile: Cannot open file %s (%s).", fName, spolName);
      printerr(spolName);

      if (!pktsendstr("RN2"))    /* Report cannot send file       */
         return XFER_LOST;       /* School is out, die            */
      else
         return XFER_FILEDONE;   /* Tell them to send next file   */

   } /* if */

   if (setvbuf( xfer_stream, NULL, vbufsize ? _IOFBF : _IONBF, vbufsize))
   {
      printmsg(0, "rsfile: Cannot unbuffer file %s (%s).", fName, spolName);
      pktsendstr("RN2");         /* Tell them we cannot handle it */
      printerr(spolName);
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

   printmsg(0, "Sending \"%s\" (%s) as \"%s\"", fName, spolName, tName);

   lName = spolName;       /* Remember name of file to log            */

   fileSize = (unsigned long) filelength( fileno( xfer_stream ) );
   (*filepkt)(KWTrue, fileSize);
                           /* Init for file transfer            */

   return XFER_SENDDATA;   /* Switch to send data state        */

} /* rsfile */

/*--------------------------------------------------------------------*/
/*    r d a t a                                                       */
/*                                                                    */
/*    Receive Data                                                    */
/*--------------------------------------------------------------------*/

XFER_STATE rdata( void )
{
   short    len;
   int    used = 0;

   do {

      if ((*getpkt)((char *) (databuf + used), &len) != DCP_OK)
      {
         fclose(xfer_stream);
         xfer_stream = NULL;
         return XFER_LOST;
      }
      else
         used += len;

   }  while (((int) (used + r_pktsize) <= (int) xferBufSize) && len);

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
      return XFER_RECVDATA;      /* Remain in data state              */

} /* rdata */

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
   char *rName = spool ? tempName : spolName;
                              /* Name to delete if we have a problem  */

/*--------------------------------------------------------------------*/
/*            Close out the file, checking for I/O errors             */
/*--------------------------------------------------------------------*/

   fclose(xfer_stream);
   if (ferror (xfer_stream ))
   {
      response = cn;          /* Report we had a problem             */
      printerr( rName );
   }

   xfer_stream = NULL;        /* To make sure!                       */

/*--------------------------------------------------------------------*/
/*    If it was a spool file, rename it to its permanent location     */
/*--------------------------------------------------------------------*/

   if (spool && equal(response,cy))
   {
      REMOVE( spolName );     /* Should be safe, since we only do it
                                 for spool files                     */

      if ( RENAME(tempName, spolName ))
      {
         printmsg(0,"reof: Unable to rename %s to %s",
                  tempName, spolName);
         response = cn;
         printerr(spolName);
      } /* if ( RENAME(tempName, spolName )) */
      spool = KWFalse;
   } /* if (equal(response,cy) && spool) */

   if (!pktsendstr(response)) /* Announce we accepted the file       */
      return XFER_LOST;       /* No ACK?  Return, if so              */

   if ( !equal(response, cy) )   /* If we had an error, delete file  */
   {
      printmsg(0,"reof: Deleting corrupted file %s", rName );
      REMOVE(rName );
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
      ticks = (now.time - startTime.time) * 1000 +
               ((long) now.millitm - (long) startTime.millitm);
      printmsg(2, "Transfer completed, %ld chars/sec",
                  (long) ((bytes * 1000) / (ticks ? ticks : 1) ));

      if (bflag[F_SYSLOG])
      {
         tmx = localtime(&now.time);
         seq++;
         if ( bflag[F_MULTITASK] )
            syslog = FOPEN(syslogName, "a",TEXT_MODE);

         if ( syslog == NULL )
            printerr(syslogName);
#ifndef _Windows
         else if ((bflag[F_MULTITASK] && setvbuf( syslog, NULL, _IONBF, 0)))
            printerr(syslogName);
#endif
         else {
            fprintf( syslog,
                   "%s!%s %c %s (%d/%d-%02d:%02d:%02d) (C,%d,%d) [%s]"
                           " <- %ld / %ld.%02d secs\n",
                   hostp->via,
                   userid,
                   type,
                   lName,
                   (tmx->tm_mon+1),
                   tmx->tm_mday,
                   tmx->tm_hour,
                   tmx->tm_min,
                   tmx->tm_sec,
                   pid,
                   seq,
                   M_device,
                   bytes,
                   ticks / 1000 ,
                   (int) ((ticks % 1000) / 10) );

            if ( bflag[F_MULTITASK] )
            {
               fclose( syslog );
               syslog = NULL;
            }
         }

      } /* if (bflag[F_SYSLOG]) */

   } /* if (bflag[F_SYSLOG] || (debuglevel > 2 )) */

/*--------------------------------------------------------------------*/
/*                      Return success to caller                      */
/*--------------------------------------------------------------------*/

   return XFER_FILEDONE;    /* go get the next file to process */

} /* reof */

/*--------------------------------------------------------------------*/
/*                           MISC ROUTINES                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    p k t s e n d s t r                                             */
/*                                                                    */
/*    Transmit a control packet                                       */
/*--------------------------------------------------------------------*/

static KWBoolean pktsendstr( char *s )
{

   printmsg(2, ">>> %s", s);

/*--------------------------------------------------------------------*/
/*    We flush here because we know we're in control and not          */
/*    waiting for remote data we could miss.                          */
/*--------------------------------------------------------------------*/

   if ( (! bflag[ F_MULTITASK ]) || (debuglevel > 2) )
      fflush( logfile );         /* Known safe place  to flush log    */

   if((*wrmsg)(s) != DCP_OK )
      return KWFalse;

   remote_stats.bsent += strlen(s)+1;

   return KWTrue;

} /* pktsendstr */

/*--------------------------------------------------------------------*/
/*    p k t g e t s t r                                               */
/*                                                                    */
/*    Receive a control packet                                        */
/*--------------------------------------------------------------------*/

static KWBoolean pktgetstr( char *s)
{
   if ((*rdmsg)(s) != DCP_OK )
     return KWFalse;

   remote_stats.breceived += strlen( s ) + 1;
   printmsg(2, "<<< %s", s);

   return KWTrue;
} /* pktgetstr */

/*--------------------------------------------------------------------*/
/*    b u f _ i n i t                                                 */
/*                                                                    */
/*    Alocate buffers for file transfer                               */
/*--------------------------------------------------------------------*/

static void buf_init( void )
{

   size_t newXferBufSize;

   if (MAXPACK >= (size_t) M_xfer_bufsize)
      newXferBufSize = (size_t) MAXPACK;
   else
      newXferBufSize = (size_t) M_xfer_bufsize;

   if (databuf == NULL)
      databuf = malloc( newXferBufSize );
   else if ( newXferBufSize != xferBufSize )
      databuf = realloc( databuf, newXferBufSize );

   checkref( databuf );

   xferBufSize = newXferBufSize;

#ifdef _Windows
   vbufsize = BUFSIZ;            /* Use normal buffering under Windows  */
#elif defined(BIT32ENV)
   vbufsize = (16 * 1024);       /* Buffer nicely under OS/2, NT        */
#else
   vbufsize = 0;
#endif

/*--------------------------------------------------------------------*/
/*            Additional common initialization for logging            */
/*--------------------------------------------------------------------*/

   pid = (int) getpid();
   seq = (( seq + 99 ) / 100) * 100;


   if ((syslogName == NULL) && bflag[F_SYSLOG])
   {
      mkfilename(tempName, E_logdir, SYSLOG);
      syslogName = newstr(tempName);
   }

} /* buf_init */
