/*--------------------------------------------------------------------*/
/*    dcpsys.h                                                        */
/*                                                                    */
/*    System functions for dcp (UUPC/extended data communications)    */
/*                                                                    */
/*    Change History:                                                 */
/*                                                                    */
/*    08 Sep 90   -  Create via Microsoft C compiler /Zg       ahd    */
/*--------------------------------------------------------------------*/

#define  TIMEOUT     -1

#define MSGTIME       20      /* Timeout for many operations         */
#define PROTOCOL_TIME 40      /* Timeout for protocol exchange msgs  */

/*--------------------------------------------------------------------*/
/*              systems (L.sys) file fields definitions               */
/*--------------------------------------------------------------------*/

typedef enum {
   FLD_REMOTE  =   0,
   FLD_CCTIME,
   FLD_TYPE,
   FLD_SPEED,
   FLD_PHONE,
   FLD_PROTO,
   FLD_EXPECT
   } FLDS;


extern char *flds[60];
extern int  kflds;

CONN_STATE getsystem( const char sendgrade );
CONN_STATE startup_server( const char recvgrade );
CONN_STATE startup_client( char *sendgrade );
CONN_STATE     sysend(void);

XFER_STATE scandir(char *remote, const char grade );

int   rmsg(char *msg, const boolean synch, unsigned int msgtime, int max_len);
void  wmsg(char *msg, const boolean synch);

extern int (*sendpkt)(char *data, int len);
extern int (*getpkt)(char *data, int *len);
extern int (*openpk)(const boolean caller);
extern int (*closepk)(void);
extern int (*wrmsg)(char *data);
extern int (*rdmsg)(char *data);
extern int (*eofpkt)(void);
extern int (*filepkt)(void);
