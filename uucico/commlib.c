/*--------------------------------------------------------------------*/
/*       c o m m l i b . C                                            */
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
 *    $Id: commlib.c 1.12 1993/11/06 17:56:09 rhg Exp $
 *
 *    Revision history:
 *    $Log: commlib.c $
 * Revision 1.12  1993/11/06  17:56:09  rhg
 * Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 * Revision 1.11  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.10  1993/10/07  22:51:00  ahd
 * Allocate communications input buffer for selected suites
 *
 * Revision 1.9  1993/10/02  23:13:29  ahd
 * Allow suppressing TCPIP support
 *
 * Revision 1.8  1993/09/27  00:45:20  ahd
 * Allow named pipes under OS/2 16 bit
 *
 * Revision 1.7  1993/09/24  03:43:27  ahd
 * Add os/2 named pipes
 *
 * Revision 1.6  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
 *
 * Revision 1.5  1993/07/22  23:22:27  ahd
 * First pass at changes for Robert Denny's Windows 3.1 support
 *
 * Revision 1.4  1993/07/13  01:13:32  ahd
 * Don't print NULL communications suite name!
 *
 * Revision 1.3  1993/07/11  14:38:32  ahd
 * Display chosen suite
 *
 * Revision 1.2  1993/05/30  15:25:50  ahd
 * Multiple driver support
 *
 * Revision 1.1  1993/05/30  00:01:47  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                       UUPC/extended includes                       */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "commlib.h"
#include "dcp.h"
#include "hostable.h"
#include "usertabl.h"
#include "security.h"
#include "modem.h"

#include "ulib.h"             /* Native communications interface      */

#if !defined(BIT32ENV) && !defined(FAMILYAPI) && !defined(_Windows)
#include "ulibfs.h"           /* DOS FOSSIL interface                 */
#include "ulib14.h"           /* DOS ARTISOFT INT14 interface         */
#endif

/*--------------------------------------------------------------------*/
/*         Define table for looking up communications functions       */
/*--------------------------------------------------------------------*/

typedef struct _COMMSUITE {
        char     *type;
        ref_activeopenline      activeopenline;
        ref_passiveopenline     passiveopenline;
        ref_sread               sread;
        ref_swrite              swrite;
        ref_ssendbrk            ssendbrk;
        ref_closeline           closeline;
        ref_SIOSpeed            SIOSpeed;
        ref_flowcontrol         flowcontrol;
        ref_hangup              hangup;
        ref_GetSpeed            GetSpeed;
        ref_CD                  CD;
        ref_WaitForNetConnect   WaitForNetConnect;
        boolean  network;
        boolean  buffered;
        char     *netDevice;           /* Network device name         */
} COMMSUITE;

/*--------------------------------------------------------------------*/
/*       Use the NOTCPIP to suppress the TCP/IP when you don't        */
/*       have WINSOCK.H                                               */
/*--------------------------------------------------------------------*/

#if defined(WIN32) || defined(_Windows)
#ifndef NOTCPIP
#include "ulibip.h"           /* Windows sockets on TCP/IP interface  */
#define TCPIP
#endif
#endif

#if defined(__OS2__) || defined(FAMILYAPI)
#include "ulibnmp.h"          /* OS/2 named pipes interface           */
#endif

#define NATIVE "internal"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

boolean portActive;         /* Port active flag for error handler   */
boolean traceEnabled;        /* Trace active flag                     */
size_t commBufferLength = 0;
size_t commBufferUsed   = 0;
char *commBuffer = NULL;

ref_activeopenline activeopenlinep;
ref_passiveopenline passiveopenlinep;
ref_sread sreadp;
ref_swrite swritep;
ref_ssendbrk ssendbrkp;
ref_closeline closelinep;
ref_SIOSpeed SIOSpeedp;
ref_flowcontrol flowcontrolp;
ref_hangup hangupp;
ref_GetSpeed GetSpeedp;
ref_CD CDp;
ref_WaitForNetConnect WaitForNetConnectp;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static FILE *traceStream;    /* Stream used for trace file            */

static short   traceMode;    /* Flag for last data (input/output)     */
                             /* written to trace log                  */

static boolean network = FALSE;  /* Current communications suite is   */
                                 /* network oriented                  */

currentfile();

/*--------------------------------------------------------------------*/
/*       c h o o s e C o m m u n i c a t i o n s                      */
/*                                                                    */
/*       Choose communications suite to use                           */
/*--------------------------------------------------------------------*/

boolean chooseCommunications( const char *name )
{
   static COMMSUITE suite[] =
   {
        { NATIVE,                      /* Default for any opsys        */
          nopenline, nopenline, nsread, nswrite,
          nssendbrk, ncloseline, nSIOSpeed, nflowcontrol, nhangup,
          nGetSpeed,
          nCD,
          (ref_WaitForNetConnect) NULL,
          FALSE,                       /* Not network based           */
#if defined(BIT32ENV) || defined(FAMILYAPI)
          TRUE,                        /* Buffered under OS/2 and Windows NT  */
#else
          TRUE,                        /* Unbuffered for DOS, Windows 3.x  */
#endif
          NULL                         /* No network device name      */
        },
#if !defined(BIT32ENV) && !defined(_Windows) && !defined(FAMILYAPI)
        { "fossil",                    /* MS-DOS FOSSIL driver        */
          fopenline, fopenline, fsread, fswrite,
          fssendbrk, fcloseline, fSIOSpeed, fflowcontrol, fhangup,
          fGetSpeed,
          fCD,
          (ref_WaitForNetConnect) NULL,
          FALSE,                       /* Not network oriented        */
          FALSE,                       /* Not buffered                 */
          NULL                         /* No network device name      */
        },
        { "articomm",                  /* MS-DOS ARTISOFT INT14 driver  */
          iopenline, iopenline, isread, iswrite,
          issendbrk, icloseline, iSIOSpeed, iflowcontrol, ihangup,
          iGetSpeed,
          iCD,
          (ref_WaitForNetConnect) NULL,
          FALSE,                       /* Not network oriented        */
          FALSE,                       /* Not buffered                 */
          NULL                         /* No network device name      */
        },
#endif

#if defined(TCPIP)
        { "tcp/ip",                    /* Win32 TCP/IP Winsock interface  */
          tactiveopenline, tpassiveopenline, tsread, tswrite,
          tssendbrk, tcloseline, tSIOSpeed, tflowcontrol, thangup,
          tGetSpeed,
          tCD,
          tWaitForNetConnect,
          TRUE,                        /* Network oriented            */
          TRUE,                        /* Uses internal buffer        */
          "tcptty",                    /* Network device name         */
        },
#endif

#if defined(__OS2__) || defined(FAMILYAPI)
        { "namedpipes",                /* OS/2 named pipes            */
          pactiveopenline, ppassiveopenline, psread, pswrite,
          pssendbrk, pcloseline, pSIOSpeed, pflowcontrol, phangup,
          pGetSpeed,
          pCD,
          pWaitForNetConnect,
          TRUE,                        /* Network oriented            */
          TRUE,                        /* Uses internal buffer        */
          "pipe",                      /* Network device name         */
        },
#endif
        { NULL }                       /* End of list                 */
   };

   int subscript = 0;

/*--------------------------------------------------------------------*/
/*                   Search for name in suite table                   */
/*--------------------------------------------------------------------*/

   while (( name  != NULL ) && (suite[subscript].type != NULL ))
   {
      if ( equali(name,suite[subscript].type))
         break;                           /* Success!                 */
      else
         subscript++;
   } /* while */

   if ( suite[subscript].type == NULL )
   {
      printmsg(0,"chooseCommunications: Invalid suite name %s",
                  name );
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*       We have a valid suite, define the routines to use and        */
/*       return to caller                                             */
/*--------------------------------------------------------------------*/

   activeopenlinep    = suite[subscript].activeopenline;
   passiveopenlinep   = suite[subscript].passiveopenline;
   sreadp             = suite[subscript].sread;
   swritep            = suite[subscript].swrite;
   ssendbrkp          = suite[subscript].ssendbrk;
   closelinep         = suite[subscript].closeline;
   SIOSpeedp          = suite[subscript].SIOSpeed;
   flowcontrolp       = suite[subscript].flowcontrol;
   hangupp            = suite[subscript].hangup;
   GetSpeedp          = suite[subscript].GetSpeed;
   CDp                = suite[subscript].CD;
   WaitForNetConnectp = suite[subscript].WaitForNetConnect;
   network            = suite[subscript].network;

/*--------------------------------------------------------------------*/
/*                  Override device name as required                  */
/*--------------------------------------------------------------------*/

   if ( suite[subscript].netDevice != NULL )
      M_device = suite[subscript].netDevice;

   if ( suite[subscript].buffered && ! commBufferLength)
   {

#ifdef BIT32ENV
      commBufferLength = (MAXPACK * 4);      /* Generous to reduce I/O's  */
#else
      commBufferLength = (MAXPACK * 2) + 20; /* 2 packet plus headers    */
#endif

      commBuffer = malloc( commBufferLength );
      checkref( commBuffer );

   } /* if */
   else if ( (! suite[subscript].buffered) && commBufferLength )
   {
      commBufferLength = 0;
      free( commBuffer );
      commBuffer = NULL;
   }
   commBufferUsed = 0;

   printmsg(equal(suite[subscript].type, NATIVE) ? 5 : 4,
            "chooseCommunications: Chose suite %s",
            suite[subscript].type );

   return TRUE;

} /* chooseCommunications */

/*--------------------------------------------------------------------*/
/*       t r a c e S t a r t                                          */
/*                                                                    */
/*       Begin communicatons line tracing                             */
/*--------------------------------------------------------------------*/

boolean traceStart( const char *port )
{
   char *linelog;
   time_t now;

   if ( ! traceEnabled )
      return FALSE;

   linelog = normalize( "LineData.Log" );

   if ( traceStream != NULL )
   {
      printmsg(0,"traceOn: Trace file %s already open!", linelog);
      panic();
   }

   traceStream = FOPEN( linelog ,"a", BINARY_MODE);

   if ( traceStream == NULL )
   {
      printerr( linelog );
      printmsg(0, "Unable to open trace file, tracing disabled");
      traceEnabled = FALSE;
      return FALSE;
   }

   time( &now );

   fprintf(traceStream,"Trace begins for port %s at %s",
           port, ctime( &now ));

   printmsg(4,"Tracing communications port %s in file %s",
            port, linelog );

   traceMode  = 2;               /* Make sure first trace includes     */
                                 /* prefix with direction              */

   return TRUE;                  /* Success to caller                 */

} /* traceStart */

/*--------------------------------------------------------------------*/
/*       t r a c e S t o p                                            */
/*                                                                    */
/*       Terminate communications line tracing                        */
/*--------------------------------------------------------------------*/

void traceStop( void )
{
   if ( traceStream != NULL )
   {
      time_t now = time( NULL );
      fprintf(traceStream,"\nTrace complete at %s",  ctime( &now ));
      fclose( traceStream );
      traceStream = NULL;
   }

} /* traceStop */

/*--------------------------------------------------------------------*/
/*       t r a c e D a t a                                            */
/*                                                                    */
/*       Write traced data to the log                                 */
/*--------------------------------------------------------------------*/

void traceData( const char *data,
                const unsigned len,
                const boolean output)
{
#ifdef VERBOSE
   unsigned subscript;
#endif


   if ( ! traceEnabled || ! len )
      return;

   printmsg(network ? 4 : 15, "traceData: %u bytes %s",
               len,
               output ? "written" : "read" );

   if ( traceMode != (short) output )
   {
      fputs(output ? "\nWrite: " : "\nRead:  ",traceStream );
      traceMode = (short) output;
   }

#ifdef VERBOSE
   for (subscript = 0; subscript < len; subscript++)
   {
      fprintf( traceStream, "%2.2x", data[subscript] );
   } /* for */
#else

   fwrite(data, 1, len, traceStream ); /* Write out raw data           */

#endif

} /* traceData */

/*--------------------------------------------------------------------*/
/*       I s N e t w o r k                                            */
/*                                                                    */
/*       Report if current communications suite is network oriented   */
/*--------------------------------------------------------------------*/

boolean IsNetwork(void)
{
   return network;         /* Preset when suite initialized           */
}
