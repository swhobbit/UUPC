#ifndef COMMLIB_H
#define COMMLIB_H

/*--------------------------------------------------------------------*/
/*       c o m m l i b . h                                            */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended.  */
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
 *    $Id: commlib.h 1.13 1994/01/01 19:22:43 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.h $
 * Revision 1.13  1994/01/01  19:22:43  ahd
 * Annual Copyright Update
 *
 * Revision 1.12  1993/12/24  05:22:49  ahd
 * Use UUFAR pointer for comm buffers
 *
 * Revision 1.11  1993/11/20  14:50:25  ahd
 * Add prototype for GetComHandle
 *
 * Revision 1.11  1993/11/20  14:50:25  ahd
 * Add prototype for GetComHandle
 *
 * Revision 1.10  1993/11/06  17:57:46  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.9  1993/10/12  01:35:12  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.8  1993/10/07  22:32:45  ahd
 * Add commBuffer information
 *
 * Revision 1.7  1993/10/02  23:51:15  ahd
 * Reduce MAXPACK for 32 bit environments to 1024 bytes because
 * of NT aborts  with 4096
 *
 * Revision 1.6  1993/09/29  04:56:11  ahd
 * Suspend port by port name, not modem file name
 *
 * Revision 1.5  1993/09/25  03:13:04  ahd
 * Add priority altering prototypes
 *
 * Revision 1.4  1993/09/21  01:43:46  ahd
 * Move MAXPACK to commlib.h to allow use as standard comm buffer size
 *
 * Revision 1.3  1993/09/20  04:53:57  ahd
 * TCP/IP support from Dave Watt
 * 't' protocol support
 * OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
 * Revision 1.2  1993/05/30  15:27:22  ahd
 * Additional multiple comm driver support
 *
 * Revision 1.1  1993/05/30  00:11:03  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*       Declare typedefs for pointers to the functions we use        */
/*       in the communications drivers                                */
/*--------------------------------------------------------------------*/

typedef int (*ref_activeopenline)(char *name, BPS baud, const boolean direct);

typedef int (*ref_passiveopenline)(char *name, BPS baud, const boolean direct);

typedef unsigned int (*ref_sread)(char UUFAR *buffer,
                                  unsigned int wanted,
                                  unsigned int timeout);

typedef int (*ref_swrite)(const char UUFAR *data, unsigned int len);

typedef void (*ref_ssendbrk)(unsigned int duration);

typedef void (*ref_closeline)(void);

typedef void (*ref_SIOSpeed)(BPS baud);

typedef void (*ref_flowcontrol)(boolean);

typedef void (*ref_hangup)( void );

typedef BPS (*ref_GetSpeed)( void );

typedef boolean (*ref_CD)( void );

typedef boolean (*ref_WaitForNetConnect)(const unsigned int timeout);

typedef int (*ref_GetComHandle)( void );

/*--------------------------------------------------------------------*/
/*       Define function to select communications driver functions;   */
/*       returns TRUE on success.                                     */
/*--------------------------------------------------------------------*/

boolean chooseCommunications( const char *suite );

boolean IsNetwork(void);         /* Report if suite is network oriented  */

/*--------------------------------------------------------------------*/
/*       Trace functions for communications routines                  */
/*--------------------------------------------------------------------*/

boolean traceStart( const char *port );

void traceStop( void );

void traceData( const char UUFAR *data,
                const unsigned len,
                const boolean output);

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
#define CD()                           (*CDp)()
#define WaitForNetConnect(timeout)     (*WaitForNetConnectp)(timeout)
#define GetComHandle()                 (*GetComHandlep)()

extern size_t commBufferLength;
extern size_t commBufferUsed;
extern char UUFAR *commBuffer;

extern boolean portActive;          /* Port active flag for error handler  */
extern boolean traceEnabled;        /* Enable comm port trace         */

boolean IsNetwork(void);

/*--------------------------------------------------------------------*/
/*                     Priority setting functions                     */
/*--------------------------------------------------------------------*/

void setPrty( const KEWSHORT priorityIn, const KEWSHORT prioritydeltaIn );

void resetPrty( void );

/*--------------------------------------------------------------------*/
/*                    Declare network buffer size                     */
/*--------------------------------------------------------------------*/

#define MAXPACK 1024          /* Needed for 't' protocol blocks      */

#endif
