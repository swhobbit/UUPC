#ifndef COMMLIB_H
#define COMMLIB_H

/*--------------------------------------------------------------------*/
/*       c o m m l i b . h                                            */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended.  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlib.h 1.27 1999/01/04 03:55:33 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.h $
 *    Revision 1.27  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.26  1998/04/20 02:48:54  ahd
 *    Windows 32 bit GUI environment/TAPI support
 *
 *    Revision 1.25  1998/03/01 01:42:09  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.24  1997/11/21 18:11:31  ahd
 *    Remove multiple port support for SMTP server
 *
 *    Revision 1.23  1997/06/03 03:26:38  ahd
 *    First compiling SMTP daemon
 *
 *    Revision 1.22  1997/05/11 04:28:53  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.21  1997/04/24 01:36:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1996/01/01 21:25:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.19  1995/02/12 23:39:45  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.18  1995/01/07 16:45:02  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.17  1994/12/22 00:39:08  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1994/05/08 02:50:44  ahd
 *    Change name of carrier detect
 *
 *        Revision 1.15  1994/04/26  03:34:33  ahd
 *        hot (previously opened) handle support
 *
 * Revision 1.14  1994/01/24  03:05:37  ahd
 * Annual Copyright Update
 *
 */

/*--------------------------------------------------------------------*/
/*       Declare typedefs for pointers to the functions we use        */
/*       in the communications drivers                                */
/*--------------------------------------------------------------------*/

typedef int (*ref_activeopenline)(char *name, BPS baud, const KWBoolean direct);

typedef int (*ref_passiveopenline)(char *name, BPS baud, const KWBoolean direct);

typedef unsigned int (*ref_sread)(char UUFAR *buffer,
                                  unsigned int wanted,
                                  unsigned int timeout);

typedef int (*ref_swrite)(const char UUFAR *data, unsigned int len);

typedef void (*ref_ssendbrk)(unsigned int duration);

typedef void (*ref_closeline)(void);

typedef void (*ref_SIOSpeed)(BPS baud);

typedef void (*ref_flowcontrol)(KWBoolean);

typedef void (*ref_hangup)( void );

typedef void (*ref_terminateCommunications)( void );

typedef BPS (*ref_GetSpeed)( void );

typedef KWBoolean (*ref_CD)( void );

typedef KWBoolean (*ref_WaitForNetConnect)(const unsigned int timeout);

typedef int (*ref_GetComHandle)( void );

typedef void (*ref_SetComHandle)( const int );

/*--------------------------------------------------------------------*/
/*       Define function to select communications driver functions;   */
/*       returns KWTrue on success.                                   */
/*--------------------------------------------------------------------*/

KWBoolean chooseCommunications( const char *suite,
                                const KWBoolean directConnection,
                                char **deviceNamePtr );

KWBoolean IsNetwork(void);   /* Report if suite is network oriented  */
KWBoolean IsTAPI(void);      /* Report if suite is TAPI oriented     */

/*--------------------------------------------------------------------*/
/*       Trace functions for communications routines                  */
/*--------------------------------------------------------------------*/

KWBoolean traceStart( const char *port );

void traceStop( void );

void traceData( const char UUFAR *data,
                const unsigned len,
                const KWBoolean output);

/*--------------------------------------------------------------------*/
/*           Declare the functions used by various routines           */
/*--------------------------------------------------------------------*/

extern ref_activeopenline activeopenlinep;
extern ref_passiveopenline passiveopenlinep;
extern ref_sread sreadp;
extern ref_swrite swritep;
extern ref_ssendbrk ssendbrkp;
extern ref_closeline closelinep;
extern ref_SIOSpeed SIOSpeedp;
extern ref_flowcontrol flowcontrolp;
extern ref_hangup hangupp;
extern ref_GetSpeed GetSpeedp;
extern ref_CD CDp;
extern ref_WaitForNetConnect WaitForNetConnectp;
extern ref_GetComHandle GetComHandlep;
extern ref_SetComHandle SetComHandlep;
extern ref_terminateCommunications terminateCommunicationsp;

/*--------------------------------------------------------------------*/
/*       Declare macros which define the prev-generic driver names    */
/*       (and are easier to type).                                    */
/*--------------------------------------------------------------------*/

#define activeopenline(name, baud, direct) (*activeopenlinep)(name, baud, direct)
#define passiveopenline(name, baud, direct)(*passiveopenlinep)(name, baud, direct)
#define sread(buffer, wanted, timeout) (*sreadp)( buffer, wanted, timeout)
#define swrite(buffer, wanted)         (*swritep)( buffer, wanted )
#define ssendbrk( duration )           (*ssendbrkp)(duration)
#define closeline()                    (*closelinep)()
#define flowcontrol(onoroff)           (*flowcontrolp)(onoroff)
#define SIOSpeed(baud)                 (*SIOSpeedp)(baud)
#define hangup()                       (*hangupp)()
#define GetSpeed()                     (*GetSpeedp)()
#define WaitForNetConnect(timeout)     (*WaitForNetConnectp)(timeout)
#define GetComHandle()                 (*GetComHandlep)()
#define SetComHandle(newHandle)        (*SetComHandlep)(newHandle)
#define terminateCommunications()      (*terminateCommunicationsp)()

extern size_t commBufferLength;
extern size_t commBufferUsed;
extern char UUFAR *commBuffer;

extern KWBoolean portActive;         /* Port active flag for error handler  */
extern KWBoolean traceEnabled;       /* Enable comm port trace          */
extern KWBoolean carrierDetect;      /* Modem is connected  flag        */

KWBoolean IsNetwork(void);

KWBoolean CD( void );

/*--------------------------------------------------------------------*/
/*                     Priority setting functions                     */
/*--------------------------------------------------------------------*/

void setPrty( const KEWSHORT priorityIn, const KEWSHORT prioritydeltaIn );

void resetPrty( void );

/*--------------------------------------------------------------------*/
/*                    Declare network buffer size                     */
/*--------------------------------------------------------------------*/

#define MAXPACK 1024          /* Needed for 't' protocol blocks      */
#define SUITE_NATIVE "internal"
#define SUITE_TCPIP  "tcp/ip"

#endif
