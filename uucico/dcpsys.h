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
void  wmsg(const char *msg, const boolean synch);

extern short (*sendpkt)(char *data, short len);
extern short (*getpkt)(char *data, short *len);
extern short (*openpk)(const boolean caller);
extern short (*closepk)(void);
extern short (*wrmsg)(char *data);
extern short (*rdmsg)(char *data);
extern short (*eofpkt)(void);
extern short (*filepkt)(void);

boolean CallWindow( const char callgrade );
