#ifndef __DCP
#define __DCP
/*
 *      History:77,17
 *      Mon May 15 19:54:43 1989 change portactive to port_active
 *      Mon May 15 19:51:13 1989 Add portactive flag
 *      19 Mar 1990  Add hostable.h header                           ahd
 *
 */

/*
        For best results in visual layout while viewing this file, set
        tab stops to every 4 columns.
*/

/*
        dcp.h
*/

/* DCP a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */

#define MAXPACK 512           /* Max packet size we can handle       */
#define SMALL_PACKET 64       /* Max packet size most UUCP's can
                                 handle                              */
#ifndef RECV_BUF
#define RECV_BUF 4096         /* COMMFIFO Buffer size                */
#endif

#define ERROR   10
#define EMPTY   11

/*--------------------------------------------------------------------*/
/*    Define high level state machine levels.  These levels define    */
/*    processing during initialization, connection and                */
/*    termination, but not during actual file transfers               */
/*--------------------------------------------------------------------*/

typedef enum {
      CONN_INITSTAT   = 'A',  /* Load host status file               */
      CONN_INITIALIZE,        /* Select system to call, if any       */
      CONN_CALLUP1,           /* Dial out to another system          */
      CONN_CALLUP2,           /* Dial out to another system          */
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

#define RETRY   (-2)
#define FAILED  (-1)
#define OK              0

typedef int     (*procref)();

typedef struct {
        char type;
        procref getpkt, sendpkt, openpk, closepk, rdmsg, wrmsg, eofpkt,
                  filepkt;
} Proto;


extern size_t s_pktsize;        /* send packet size for this protocol   */
extern size_t r_pktsize;        /* receive packet size for this protocol*/
extern FILE *syslog;            /* syslog file pointer                  */
extern char workfile[FILENAME_MAX];
                                /* name of current workfile             */
extern FILE *fwork;             /* current work file pointer    */
extern FILE *xfer_stream;       /* current disk file stream for file
                                   being transfered                     */
extern FILE *fsys;

extern char *Rmtname;           /* system we WANT to talk to    */

extern char rmtname[20];        /* system we end up talking to  */

extern char s_systems[FILENAME_MAX];
                                /* full-name of systems file    */
extern struct HostTable *hostp;
extern struct HostStats remote_stats;
                                 /* host status, as defined by hostatus */

int    dcpmain(int  argc,char  * *argv);

#endif /* __DCP */
