#ifndef DCP_H
#define DCP_H

/*--------------------------------------------------------------------*/
/*    d c p . h                                                       */
/*                                                                    */
/*    UUCICO main program definitions                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (C) Richard H. Lamb 1985, 1986, 1987                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcp.h 1.21 1998/03/01 01:42:13 ahd v1-12v $
 *
 *    $Log: dcp.h $
 *    Revision 1.21  1998/03/01 01:42:13  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1997/04/24 01:36:43  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.19  1996/01/01 21:25:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.18  1995/03/11 15:50:10  ahd
 *    Add unique set for after clock being set
 *
 *    Revision 1.17  1995/02/12 23:39:45  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.16  1994/12/31 03:40:35  ahd
 *    Correct processing for systems with grade restrictions when
 *    calling system "any"
 *
 *    Revision 1.15  1994/12/27 20:46:08  ahd
 *    Smoother call grading'
 *
 *    Revision 1.14  1994/12/22 00:39:17  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1994/12/09 03:51:22  ahd
 *    Add new states for running UUXQT after line closes
 *
 * Revision 1.12  1994/01/24  03:05:48  ahd
 * Annual Copyright Update
 */

#define SMALL_PACKET 64       /* Max packet size most UUCP's can
                                 handle                              */
#ifndef RECV_BUF
#define RECV_BUF (MAXPACK*8)  /* 512 * 8 = 4096 = COMMFIFO buffer size  */
                              /* for DOS                              */
#endif

#define DCP_ERROR   10
#define DCP_EMPTY   11

/*--------------------------------------------------------------------*/
/*    Define high level state machine levels.  These levels define    */
/*    processing during initialization, connection and                */
/*    termination, but not during actual file transfers               */
/*--------------------------------------------------------------------*/

typedef enum
{
      CONN_INITSTAT   = 'A',  /* Load host status file               */
      CONN_INITIALIZE,        /* Select system to call, if any       */
      CONN_CHECKTIME,         /* Check time to dial another          */
      CONN_NOGRADE,           /* Grade prevents call, just flag
                                 as bad time                         */
      CONN_MODEM,             /* Load modem to process a system      */
      CONN_DIALOUT,           /* Actually dial modem to system       */
      CONN_HOTMODEM,          /* Initialize for a modem already
                                 off-hook                            */
      CONN_ANSWER,            /* Wait for phone to ring and user to
                                 login                               */
      CONN_LOGIN,             /* Modem is connected, do a login      */
      CONN_HOTLOGIN,          /* Modem is connected, initialize user
                                 from command line w/o login         */
      CONN_PROTOCOL,          /* Exchange protocol information       */
      CONN_SERVER,            /* Process files after dialing out     */
      CONN_CLIENT,            /* Process files after being called    */
      CONN_TERMINATE,         /* Terminate procotol                  */
      CONN_TIMESET,           /* Set of system clock complete        */
      CONN_DROPLINE,          /* Hangup the telephone                */
      CONN_CLEANUP,           /* Cleanup after line termination      */
      CONN_UUXQT,             /* Execute UUXQT                       */
      CONN_WAIT,              /* Wait until told to continue         */
      CONN_EXIT,              /* Exit state machine loop             */
      CONN_NO_RETURN          /* Special for for modem.c processing  */
} CONN_STATE ;

/*--------------------------------------------------------------------*/
/*                        File transfer states                        */
/*--------------------------------------------------------------------*/

typedef enum {
      XFER_SENDINIT = 'a',    /* Initialize outgoing protocol        */
      XFER_MASTER,            /* Begin master mode                   */
      XFER_FILEDONE,          /* Receive or transmit is complete     */
      XFER_NEXTJOB,           /* Look for work in local queue        */
      XFER_REQUEST,           /* Process work in local queue         */
      XFER_PUTFILE,           /* Send a file to remote host at our
                                 request                             */
      XFER_GETFILE,           /* Retrieve a file from a remote host
                                 at our request                      */
      XFER_SENDDATA,          /* Remote accepted our work, send data */
      XFER_SENDEOF,           /* File xfer complete, send EOF        */
      XFER_NEXTGRADE,         /* Process next lowest grade in local  */
      XFER_NOLOCAL,           /* No local work, remote have any?     */
      XFER_SLAVE,             /* Begin slave mode                    */
      XFER_RECVINIT,          /* Initialize Receive protocol         */
      XFER_RECVHDR,           /* Receive header from other host      */
      XFER_GIVEFILE,          /* Send a file to remote host at their
                                 request                             */
      XFER_TAKEFILE,          /* Retrieve a file from a remote host
                                 at their request                    */
      XFER_RECVDATA,          /* Receive file data from other host   */
      XFER_RECVEOF,           /* Close file received from other host */
      XFER_NOREMOTE,          /* No remote work, local have any?     */
      XFER_LOST,              /* Lost the other host, flame out      */
      XFER_ABORT,             /* Internal error, flame out           */
      XFER_ENDP,              /* End the protocol                    */
      XFER_EXIT               /* Return to caller                    */
      } XFER_STATE ;

#define DCP_RETRY   (-2)
#define DCP_FAILED  (-1)
#define DCP_OK      0

extern size_t s_pktsize;        /* send packet size for this protocol  */
extern size_t r_pktsize;        /* receive packet size for this protocol*/
extern FILE *syslog;            /* syslog file pointer                 */
extern char workfile[FILENAME_MAX];
                                /* name of current workfile            */
extern FILE *fwork;             /* current work file pointer    */
extern FILE *xfer_stream;       /* current disk file stream for file
                                   being transfered                    */
extern FILE *fsys;

extern char *Rmtname;           /* system we WANT to talk to    */

extern char rmtname[20];        /* system we end up talking to  */

extern struct HostTable *hostp;
extern struct HostStats remote_stats;
                                /* host status, as defined by hostatus */

int    dcpmain(int  argc,char  * *argv);

#endif /* __DCP */
