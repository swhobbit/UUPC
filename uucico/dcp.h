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
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcp.h 1.11 1994/01/01 19:22:49 ahd Exp $
 *
 *    $Log: dcp.h $
 * Revision 1.11  1994/01/01  19:22:49  ahd
 * Annual Copyright Update
 *
 * Revision 1.10  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.10  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.9  1993/09/29  04:56:11  ahd
 * Suspend port by port name, not modem file name
 *
 * Revision 1.8  1993/09/27  00:50:57  ahd
 * Control of serial port in passive mode by K. Rommel
 *
 * Revision 1.7  1993/09/21  01:43:46  ahd
 * Move MAXPACK to commlib.h to allow use as standard comm buffer size
 *
 *      Mon May 15 19:54:43 1989 change portactive to port_active
 *      Mon May 15 19:51:13 1989 Add portactive flag
 *      19 Mar 1990  Add hostable.h header                           ahd
 *
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

typedef enum {
      CONN_INITSTAT   = 'A',  /* Load host status file               */
      CONN_INITIALIZE,        /* Select system to call, if any       */
      CONN_CHECKTIME,         /* Check time to dial another          */
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
      CONN_DROPLINE,          /* Hangup the telephone                */
      CONN_WAIT,              /* Wait until told to continue         */
      CONN_EXIT }             /* Exit state machine loop             */
      CONN_STATE ;

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

typedef short   (*procref)();

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
