/*--------------------------------------------------------------------*/
/*       c o m m l i b . C                                            */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended.  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlib.c 1.33 1997/05/11 04:28:26 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.c $
 *    Revision 1.33  1997/05/11 04:28:26  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.32  1997/04/24 01:32:59  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.31  1996/01/01 21:19:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.30  1995/02/26 02:51:34  ahd
 *    Clean up memory allocations to not require #ifdef
 *    Enable buffering for ulibwin.C
 *
 *    Revision 1.29  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.28  1995/01/07 16:37:46  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.27  1994/12/22 04:13:38  ahd
 *    Correct 't' protocol processing to use 512 messages with no header
 *
 *    Revision 1.26  1994/12/22 00:32:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.25  1994/05/07 21:45:33  ahd
 *    Correct CD() processing to be sticky -- once it fails, it
 *    keeps failing until reset by close or hangup.
 *
 *        Revision 1.24  1994/04/27  00:02:15  ahd
 *        Pick one: Hot handles support, OS/2 TCP/IP support,
 *                  title bar support
 *
 *        Revision 1.23  1994/03/11  01:49:45  ahd
 *        Move the Mother of All Headers out of #ifdef
 *
 * Revision 1.22  1994/02/26  17:21:15  ahd
 * Change BINARY_MODE to IMAGE_MODE to avoid IBM C/SET 2 conflict
 *
 * Revision 1.21  1994/02/22  02:59:10  ahd
 * Always insure buffer is at least BUFSIZ long, in case any
 * random buffer wants to snarf BUFSIZ data.
 *
 * Revision 1.20  1994/02/19  05:13:33  ahd
 * Use standard first header
 *
 * Revision 1.19  1994/02/13  04:54:35  ahd
 * Report address when tracing data
 *
 * Revision 1.18  1994/01/24  03:03:52  ahd
 * Annual Copyright Update
 *
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

#include "uupcmoah.h"

#ifndef BIT32ENV
#include <malloc.h>        /* _fmalloc, etc.                         */
#endif

/*--------------------------------------------------------------------*/
/*                       UUPC/extended includes                       */
/*--------------------------------------------------------------------*/

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
        ref_SetComHandle        SetComHandle;
        KWBoolean  network;
        KWBoolean  buffered;
        char     *netDevice;           /* Network device name         */
} COMMSUITE;

#ifdef TCPIP
#include "ulibip.h"           /* Windows sockets on TCP/IP interface  */
#else
#ifdef TCPIP_ONLY
#error   TCPIP_ONLY defined, but TCPIP not defined.
#endif
#endif

#if defined(__OS2__) || defined(FAMILYAPI)
#include "ulibnmp.h"          /* OS/2 named pipes interface           */
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

KWBoolean portActive;        /* Port active flag for error handler   */
KWBoolean traceEnabled;      /* Trace active flag                    */
size_t commBufferLength = 0;
size_t commBufferUsed   = 0;
char UUFAR *commBuffer = NULL;
KWBoolean  carrierDetect;    /* Modem is not connected    */
KWBoolean  reportModemCarrierDirect;   /* Report true status         */

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
ref_SetComHandle SetComHandlep;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static FILE *traceStream;    /* Stream used for trace file            */

static short   traceMode;    /* Flag for last data (input/output)     */
                             /* written to trace log                  */

static KWBoolean network = KWFalse;  /* Current communications suite is  */
                                 /* network oriented                  */

currentfile();

int dummyGetComHandle( void );

void dummySetComHandle( const int );

KWBoolean dummyWaitForNetConnect(const unsigned int timeout);

/*--------------------------------------------------------------------*/
/*       c h o o s e C o m m u n i c a t i o n s                      */
/*                                                                    */
/*       Choose communications suite to use                           */
/*--------------------------------------------------------------------*/

KWBoolean chooseCommunications( const char *name,
                                const KWBoolean carrierDetectParam,
                                char  **deviceNamePtr )
{
   static COMMSUITE suite[] =
   {
#ifndef TCPIP_ONLY

        { SUITE_NATIVE,                /* Default for any opsys       */
          nopenline, nopenline, nsread, nswrite,
          nssendbrk, ncloseline, nSIOSpeed, nflowcontrol, nhangup,
          nGetSpeed,
          nCD,
          dummyWaitForNetConnect,
#if defined(BIT32ENV) || defined(FAMILYAPI)
          nGetComHandle,
          nSetComHandle,
          KWFalse,                  /* Not network based              */
          KWTrue,                   /* Buffered under OS/2 and NT     */
#elif defined(_Windows)
          dummyGetComHandle,
          dummySetComHandle,
          KWFalse,                  /* Not network based              */
          KWTrue,                   /* Buffered under Windows 3.1 too */
#else
          dummyGetComHandle,
          dummySetComHandle,
          KWFalse,                  /* Not network based             */
          KWTrue,                   /* Unbuffered for DOS            */
#endif
          NULL                      /* No network device name        */
        },

#if !defined(BIT32ENV) && !defined(_Windows) && !defined(FAMILYAPI)

        { "fossil",                    /* MS-DOS FOSSIL driver        */
          fopenline, fopenline, fsread, fswrite,
          fssendbrk, fcloseline, fSIOSpeed, fflowcontrol, fhangup,
          fGetSpeed,
          fCD,
          dummyWaitForNetConnect,
          dummyGetComHandle,
          dummySetComHandle,
          KWFalse,                     /* Not network oriented        */
          KWFalse,                     /* Not buffered                */
          NULL                         /* No network device name      */
        },

#if defined(ARTICOMM)
        { "articomm",                  /* MS-DOS ARTISOFT INT14 driver*/
          iopenline, iopenline, isread, iswrite,
          issendbrk, icloseline, iSIOSpeed, iflowcontrol, ihangup,
          iGetSpeed,
          iCD,
          dummyWaitForNetConnect,
          dummyGetComHandle,
          dummySetComHandle,
          KWFalse,                     /* Not network oriented        */
          KWTrue,                      /* Buffered                    */
          NULL                         /* No network device name      */
        },
#else
        { "int14",                     /* MS-DOS Generic INT14 driver  */
          iopenline, iopenline, isread, iswrite,
          issendbrk, icloseline, iSIOSpeed, iflowcontrol, ihangup,
          iGetSpeed,
          iCD,
          dummyWaitForNetConnect,
          dummyGetComHandle,
          dummySetComHandle,
          KWFalse,                     /* Not network oriented        */
          KWTrue,                      /* Buffered                    */
          NULL                         /* No network device name      */
        },

#endif /* ARTICOMM */

#endif

#if defined(__OS2__) || defined(FAMILYAPI)
        { "namedpipes",                /* OS/2 named pipes            */
          pactiveopenline, ppassiveopenline, psread, pswrite,
          pssendbrk, pcloseline, pSIOSpeed, pflowcontrol, phangup,
          pGetSpeed,
          pCD,
          pWaitForNetConnect,
          pGetComHandle,
          pSetComHandle,
          KWTrue,                      /* Network oriented            */
          KWTrue,                      /* Uses internal buffer        */
          "pipe",                      /* Network device name         */
        },
#endif
#endif /* not defined TCPIP_ONLY */

#if defined(TCPIP)
        { SUITE_TCPIP,                 /* Win32 TCP/IP Winsock interface  */
          tactiveopenline, tpassiveopenline, tsread, tswrite,
          tssendbrk, tcloseline, tSIOSpeed, tflowcontrol, thangup,
          tGetSpeed,
          tCD,
          tWaitForNetConnect,
          tGetComHandle,
          tSetComHandle,
          KWTrue,                      /* Network oriented            */
          KWTrue,                      /* Uses internal buffer        */
          "tcptty",                    /* Network device name         */
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
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*       We have a valid suite, define the routines to use            */
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
   SetComHandlep      = suite[subscript].SetComHandle;
   network            = suite[subscript].network;

   reportModemCarrierDirect = carrierDetectParam;

/*--------------------------------------------------------------------*/
/*                  Override device name as required                  */
/*--------------------------------------------------------------------*/

   if (deviceNamePtr && ( suite[subscript].netDevice != NULL ))
      *deviceNamePtr = suite[subscript].netDevice;

   if ( suite[subscript].buffered && ! commBufferLength)
   {

      commBufferLength = (MAXPACK * 3);   /* 3 buffers, reduces overhead */
      if ( BUFSIZ > commBufferLength )
         commBufferLength = BUFSIZ;

      commBuffer = MALLOC( commBufferLength );

      checkref( commBuffer );

   } /* if */
   else if ( (! suite[subscript].buffered) && commBufferLength )
   {
      commBufferLength = 0;
      FREE( commBuffer );
      commBuffer = NULL;
   }
   commBufferUsed = 0;

   printmsg(equal(suite[subscript].type, SUITE_NATIVE) ? 5 : 4,
            "chooseCommunications: Chose suite %s",
            suite[subscript].type );

   carrierDetect = KWFalse;
   return KWTrue;

} /* chooseCommunications */

/*--------------------------------------------------------------------*/
/*       t r a c e S t a r t                                          */
/*                                                                    */
/*       Begin communicatons line tracing                             */
/*--------------------------------------------------------------------*/

KWBoolean traceStart( const char *port )
{
   char *linelog;
   time_t now;

/*--------------------------------------------------------------------*/
/*               Perform a little common house keeping                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                  Don't trace if not requested to                   */
/*--------------------------------------------------------------------*/

   if ( ! traceEnabled )
      return KWFalse;

   linelog = normalize( "LineData.Log" );

   if ( traceStream != NULL )
   {
      printmsg(0,"traceOn: Trace file %s already open!", linelog);
      panic();
   }

   traceStream = FOPEN( linelog ,"a", IMAGE_MODE);

   if ( traceStream == NULL )
   {
      printerr( linelog );
      printmsg(0, "Unable to open trace file, tracing disabled");
      traceEnabled = KWFalse;
      return KWFalse;
   }

   time( &now );

   fprintf(traceStream,"Trace begins for port %s at %s",
           port, ctime( &now ));

   printmsg(4,"Tracing communications port %s in file %s",
            port, linelog );

   traceMode  = 2;               /* Make sure first trace includes     */
                                 /* prefix with direction              */

   return KWTrue;                 /* Success to caller                 */

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
                const KWBoolean output)
{

#if defined(VERBOSE) || !defined(BIT32ENV)
   unsigned subscript;
#endif

   if ( ! traceEnabled || ! len )
      return;

   printmsg(network ? 4 : 15, "traceData: %u bytes %s "

#if defined(BIT32ENV)
               "%p",
#else
               "%Fp",
#endif
               len,
               output ? "written from" : "read into",
               data);

   if ( traceMode != (short) output )
   {
      fputs(output ? "\nWrite: " : "\nRead:  ",traceStream );
      traceMode = (short) output;
   }

#if defined(VERBOSE)

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

KWBoolean IsNetwork(void)
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

#if defined(__TURBOC__)
#pragma argsused
#endif

void dummySetComHandle( const int foo )
{
}

#if defined(__TURBOC__)
#pragma argsused
#endif

KWBoolean dummyWaitForNetConnect(const unsigned int timeout)
{
   return KWFalse;
}

/*--------------------------------------------------------------------*/
/*       C D                                                          */
/*                                                                    */
/*       Wrapper function script for determining if carrier detect    */
/*       is active                                                    */
/*--------------------------------------------------------------------*/

KWBoolean
CD( void )
{
   if (reportModemCarrierDirect)
      return (*CDp)();
   else
      return KWTrue;

} /* CD */
