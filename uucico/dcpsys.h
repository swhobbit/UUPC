/*--------------------------------------------------------------------*/
/*    dcpsys.h                                                        */
/*                                                                    */
/*    System functions for dcp (UUPC/extended data communications)    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: dcpsys.h 1.9 1994/12/27 20:46:08 ahd Exp $
 *
 *    Revision history:
 */

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

CONN_STATE sysend(void);

void resetGrade( void );

char nextGrade( const char grade );

XFER_STATE scandir(char *remote, const char grade );

int   rmsg(char *msg, const KWBoolean synch, unsigned int msgtime, int max_len);

void  wmsg(const char *msg, const KWBoolean synch);

extern short (*sendpkt)(char *data, short len);

extern short (*getpkt)(char *data, short *len);

extern short (*openpk)(const KWBoolean caller);

extern short (*closepk)(void);

extern short (*wrmsg)(char *data);

extern short (*rdmsg)(char *data);

extern short (*eofpkt)(void);

extern short (*filepkt)(const KWBoolean master, const unsigned long bytes);

KWBoolean CallWindow( const char callgrade );
