#ifndef COMMLIB_H
#define COMMLIB_H

/*--------------------------------------------------------------------*/
/*       c o m m l i b . h                                            */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended.  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlib.h 1.3 1993/09/20 04:53:57 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.h $
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
/*       Declare pointers to the functions we use the                 */
/*       communications driver                                        */
/*--------------------------------------------------------------------*/

typedef int (*commrefi)();
typedef unsigned int (*commrefu)();
typedef void    (*commrefv)();
typedef boolean (*commrefb)();
typedef BPS     (*commrefB)();

/*--------------------------------------------------------------------*/
/*         Define table for looking up communications functions       */
/*--------------------------------------------------------------------*/

typedef struct _COMMSUITE {
        char     *type;
        commrefi activeopenline;
        commrefi passiveopenline;
        commrefu sread;
        commrefi swrite;
        commrefv ssendbrk, closeline, SIOSpeed, flowcontrol, hangup;
        commrefB GetSpeed;
        commrefb CD;
        commrefb WaitForNetConnect;
        boolean  network;
} COMMSUITE;

/*--------------------------------------------------------------------*/
/*       Define function to select communications driver functions;   */
/*       returns TRUE on success.                                     */
/*--------------------------------------------------------------------*/

boolean chooseCommunications( const char *suite );

boolean IsNetwork(void);         // Report if suite is network oriented

/*--------------------------------------------------------------------*/
/*       Trace functions for communications routines                  */
/*--------------------------------------------------------------------*/

boolean traceStart( const char *port );

void traceStop( void );

void traceData( const char *data,
                const short len,
                const boolean output);

/*--------------------------------------------------------------------*/
/*           Declare the functions used by various routines           */
/*--------------------------------------------------------------------*/

extern int (*activeopenlinep)(char *name, BPS baud, const boolean direct);

extern int (*passiveopenlinep)(char *name, BPS baud, const boolean direct);

extern unsigned int (*sreadp)(char *buffer,
                          unsigned int wanted,
                          unsigned int timeout);

extern int (*swritep)(const char *data, unsigned int len);

extern void (*ssendbrkp)(unsigned int duration);

extern void (*closelinep)(void);

extern void (*SIOSpeedp)(BPS baud);

extern void (*flowcontrolp)( boolean );

extern void (*hangupp)( void );

extern BPS (*GetSpeedp)( void );

extern boolean (*CDp)( void );

extern boolean (*WaitForNetConnectp)( const unsigned int timeout);

/*--------------------------------------------------------------------*/
/*       Declare macros which define the prev-generic driver names    */
/*       (and are easier to type).                                    */
/*--------------------------------------------------------------------*/

#define activeopenline(name, baud, direct) (*activeopenlinep)(name, baud, direct)
#define passiveopenline(name, baud, direct)(*passiveopenlinep)(name, baud, direct)
#define sread(buffer, wanted, timeout) (*sreadp)(buffer, wanted, timeout)
#define swrite(buffer, wanted)         (*swritep)(buffer, wanted )
#define ssendbrk( duration )           (*ssendbrkp)(duration)
#define closeline()                    (*closelinep)()
#define flowcontrol(onoroff)           (*flowcontrolp)(onoroff)
#define SIOSpeed(baud)                 (*SIOSpeedp)(baud)
#define hangup()                       (*hangupp)()
#define GetSpeed()                     (*GetSpeedp)()
#define CD()                           (*CDp)()
#define WaitForNetConnect(timeout)     (*WaitForNetConnectp)(timeout)

extern boolean portActive;          // Port active flag for error handler
extern boolean traceEnabled;        // Enable comm port trace

boolean IsNetwork(void);

/*--------------------------------------------------------------------*/
/*                    Declare network buffer size                     */
/*--------------------------------------------------------------------*/

#if defined(__OS2__) || defined(WIN32) // 32 bit compiler?
#define MAXPACK 4096          /* Max packet size we can handle       */
#elif defined(_Windows)
#define MAXPACK 1024          /* Needed for 't' protocol blocks      */
#else
#define MAXPACK 512           /* Max packet size we can handle       */
#endif

#endif
