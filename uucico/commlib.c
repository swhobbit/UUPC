/*--------------------------------------------------------------------*/
/*       c o m m l i b . C                                            */
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
 *    $Id: commlib.c 1.17 1994/01/01 19:17:54 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.c $
 * Revision 1.17  1994/01/01  19:17:54  ahd
 * Annual Copyright Update
 *
 * Revision 1.16  1993/12/26  16:20:17  ahd
 * Build int14 version by default under DOS, not articomm
 * Buffer int14 and articomm
 *
 * Revision 1.15  1993/12/24  05:12:54  ahd
 * Use far buffer for master communications buffer
 *
 * Revision 1.14  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.14  1993/11/20  14:48:53  ahd
 * Add support for passing port name/port handle/port speed/user id to child
 *
 * Revision 1.13  1993/11/16  05:37:01  ahd
 * Up 16 bit buffer size
 *
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

#ifndef BIT32ENV
#include <malloc.h>        /* _fmalloc, etc.                         */
#endif

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
        ref_GetComHandle        GetComHandle;
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
char UUFAR *commBuffer = NULL;

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
ref_GetComHandle GetComHandlep;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static FILE *traceStream;    /* Stream used for trace file            */

static short   traceMode;    /* Flag for last data (input/output)     */
                             /* written to trace log                  */

static boolean network = FALSE;  /* Current communications suite is   */
                                 /* network oriented                  */

currentfile();

int dummyGetComHandle( void );

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
          (ref_WaitForNetConnect) 0,
#if defined(BIT32ENV) || defined(FAMILYAPI)
          nGetComHandle,
          FALSE,                       /* Not network based           */
          TRUE,                        /* Buffered under OS/2 and Windows NT  */
#else
          dummyGetComHandle,
          FALSE,                       /* Not network based           */
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
          (ref_WaitForNetConnect) 0,
          dummyGetComHandle,
          FALSE,                       /* Not network oriented        */
          FALSE,                       /* Not buffered                 */
          NULL                         /* No network device name      */
        },
#ifdef ARTICOMM
        { "articomm",                  /* MS-DOS ARTISOFT INT14 driver  */
          iopenline, iopenline, isread, iswrite,
          issendbrk, icloseline, iSIOSpeed, iflowcontrol, ihangup,
          iGetSpeed,
          iCD,
          (ref_WaitForNetConnect) 0,
          dummyGetComHandle,
          FALSE,                       /* Not network oriented        */
          TRUE,                        /* Buffered                    */
          NULL                         /* No network device name      */
        },
#else
        { "int14",                     /* MS-DOS Generic INT14 driver  */
          iopenline, iopenline, isread, iswrite,
          issendbrk, icloseline, iSIOSpeed, iflowcontrol, ihangup,
          iGetSpeed,
          iCD,
          (ref_WaitForNetConnect) 0,
          dummyGetComHandle,
          FALSE,                       /* Not network oriented        */
          TRUE,                        /* Buffered                    */
          NULL                         /* No network device name      */
        },

#endif /* ARTICOMM */

#endif

#if defined(TCPIP)
        { "tcp/ip",                    /* Win32 TCP/IP Winsock interface  */
          tactiveopenline, tpassiveopenline, tsread, tswrite,
          tssendbrk, tcloseline, tSIOSpeed, tflowcontrol, thangup,
          tGetSpeed,
          tCD,
          tWaitForNetConnect,
          dummyGetComHandle,
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
          dummyGetComHandle,
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
   GetComHandlep      = suite[subscript].GetComHandle;
   network            = suite[subscript].network;

/*--------------------------------------------------------------------*/
/*                  Override device name as required                  */
/*--------------------------------------------------------------------*/

   if ( suite[subscript].netDevice != NULL )
      M_device = suite[subscript].netDevice;

   if ( suite[subscript].buffered && ! commBufferLength)
   {

      commBufferLength = (MAXPACK * 3);   /* 3 buffers, reduces overhead */

#ifdef BIT32ENV
      commBuffer = malloc( commBufferLength );
#else
      commBuffer = _fmalloc( commBufferLength );
#endif

      checkref( commBuffer );

   } /* if */
   else if ( (! suite[subscript].buffered) && commBufferLength )
   {
      commBufferLength = 0;
#ifdef BIT32ENV
      free( commBuffer );
#else
      _ffree( commBuffer );
#endif
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

void traceData( const char UUFAR *data,
                const unsigned len,
                const boolean output)
{

#if defined(VERBOSE) || !defined(BIT32ENV)
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

#elif defined(BIT32ENV)

   fwrite(data, 1, len, traceStream ); /* Write out raw data           */

#else

   for (subscript = 0; subscript < len; subscript++)
   {
      fputc( data[subscript], traceStream );
   } /* for */

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


/*--------------------------------------------------------------------*/
/*       d u m m y G e t C o m H a n d l e                            */
/*                                                                    */
/*       Return invalid communications handle for unsupported         */
/*       environments                                                 */
/*--------------------------------------------------------------------*/

int dummyGetComHandle( void )
{
   return -1;
}
