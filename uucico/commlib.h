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
 *    $Id: lib.h 1.5 1993/04/04 21:51:00 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
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
        char *type;
        commrefi openline;
        commrefu sread;
        commrefi swrite;
        commrefv ssendbrk, closeline, SIOSpeed, flowcontrol, hangup;
        commrefB GetSpeed;
        commrefb CD;
} COMMSUITE;

/*--------------------------------------------------------------------*/
/*       Define function to select communications driver functions;   */
/*       returns TRUE on success.                                     */
/*--------------------------------------------------------------------*/

boolean chooseCommunications( const char *suite );


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

extern int (*openlinep)(char *name, BPS baud, const boolean direct);

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

/*--------------------------------------------------------------------*/
/*       Declare macros which define the prev-generic driver names    */
/*       (and are easier to type).                                    */
/*--------------------------------------------------------------------*/

#define openline(name, baud, direct)   (*openlinep)(name, baud, direct)
#define sread(buffer, wanted, timeout) (*sreadp)(buffer, wanted, timeout)
#define swrite(buffer, wanted)         (*swritep)(buffer, wanted )
#define ssendbrk( duration )           (*ssendbrkp)(duration)
#define closeline()                    (*closelinep)()
#define flowcontrol(onoroff)           (*flowcontrolp)(onoroff)
#define SIOSpeed(baud)                 (*SIOSpeedp)(baud)
#define hangup()                       (*hangupp)()
#define GetSpeed()                     (*GetSpeedp)()
#define CD()                           (*CDp)()

extern boolean port_active;         // Port active flag for error handler
extern boolean traceEnabled;        // Enable comm port trace

#endif
