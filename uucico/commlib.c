/*--------------------------------------------------------------------*/
/*       c o m m l i b . C                                            */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended.  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlib.c 1.42 2001/03/12 13:54:49 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: commlib.c $
 *    Revision 1.42  2001/03/12 13:54:49  ahd
 *    Annual copyright update
 *
 *    Revision 1.41  2000/05/12 12:32:00  ahd
 *    Annual copyright update
 *
 *    Revision 1.40  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.39  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.38  1998/04/19 23:55:58  ahd
 *    *** empty log message ***
 *
 *    Revision 1.37  1998/03/01 01:39:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.36  1997/11/21 18:10:59  ahd
 *    Remove multiple port support for SMTP server
 *
 *    Revision 1.35  1997/06/03 03:25:31  ahd
 *    First compiling SMTPD
 *
 *    Revision 1.34  1997/05/11 18:15:50  ahd
 *    Allow faster SMTP delivery via fastsmtp flag
 *    Move TCP/IP dependent code from rmail.c to deliver.c
 *    Allow building rmail without SMTP or TCP/IP support
 *
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
        ref_activeopenline              activeopenline;
        ref_passiveopenline             passiveopenline;
        ref_sread                       sread;
        ref_swrite                      swrite;
        ref_ssendbrk                    ssendbrk;
        ref_closeline                   closeline;
        ref_SIOSpeed                    SIOSpeed;
        ref_flowcontrol                 flowcontrol;
        ref_hangup                      hangup;
        ref_GetSpeed                    GetSpeed;
        ref_CD                          CD;
        ref_WaitForNetConnect           WaitForNetConnect;
        ref_GetComHandle                GetComHandle;
        ref_SetComHandle                SetComHandle;
        ref_terminateCommunications     terminateCommunications;
        KWBoolean  network;
        KWBoolean  buffered;
        KWBoolean  tapi;
        char     *netDevice;           /* Network device name         */
} COMMSUITE;

#ifdef TCPIP
#include "ulibip.h"           /* Windows sockets on TCP/IP interface  */
#else
#ifdef TCPIP_ONLY
#error   TCPIP_ONLY defined, but TCPIP not defined.
#endif
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
ref_terminateCommunications   terminateCommunicationsp;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static FILE *traceStream;    /* Stream used for trace file            */

static int     traceMode;    /* Flag for last data (input/output)     */
                             /* written to trace log                  */

static KWBoolean network = KWFalse;  /* Current communications suite is  */
                                 /* network oriented                  */

static KWBoolean tapi = KWFalse; /* Current communications suite is  */
                                 /* based on MS Windows TAPI         */

RCSID("$Id: commlib.c 1.42 2001/03/12 13:54:49 ahd v1-13k $");

int dummyGetComHandle( void );

void dummySetComHandle( const int );

void dummyTerminateCommunications( void );

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
          dummyTerminateCommunications,
          KWFalse,                  /* Not network based              */
          KWTrue,                   /* Buffered under OS/2 and NT     */
#elif defined(_Windows)
          dummyGetComHandle,
          dummySetComHandle,
          dummyTerminateCommunications,
          KWFalse,                  /* Not network based              */
          KWTrue,                   /* Buffered under Windows 3.1 too */
#else
          dummyGetComHandle,
          dummySetComHandle,
          dummyTerminateCommunications,
          KWFalse,                  /* Not network based             */
          KWTrue,                   /* Unbuffered for DOS            */
#endif
          KWFalse,                  /* Not MS-Windows TAPI           */
          NULL                      /* No network device name        */
        },

#ifdef TAPI_SUPPORT
        { "tapi",                 /* Win32 Telephone interface       */
          nopenline, nopenline, nsread, nswrite,
          /* Note that mcloseline is unique! */
          nssendbrk, mcloseline, nSIOSpeed, nflowcontrol, nhangup,
          nGetSpeed,
          nCD,
          dummyWaitForNetConnect,
          nGetComHandle,
          nSetComHandle,
          dummyTerminateCommunications,
          KWFalse,                  /* Not network based              */
          KWTrue,                   /* Buffered under OS/2 and NT     */
          KWTrue,                   /* Is MS-Windows TAPI             */
          NULL                      /* No network device name         */
        },
#endif

#if !defined(BIT32ENV) && !defined(_Windows) && !defined(FAMILYAPI)

        { "fossil",                    /* MS-DOS FOSSIL driver        */
          fopenline, fopenline, fsread, fswrite,
          fssendbrk, fcloseline, fSIOSpeed, fflowcontrol, fhangup,
          fGetSpeed,
          fCD,
          dummyWaitForNetConnect,
          dummyGetComHandle,
          dummySetComHandle,
          dummyTerminateCommunications,
          KWFalse,                     /* Not network oriented        */
          KWFalse,                     /* Not buffered                */
          KWFalse,                  /* Not MS-Windows TAPI           */
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
          dummyTerminateCommunications,
          KWFalse,                     /* Not network oriented        */
          KWTrue,                      /* Buffered                    */
          KWFalse,                  /* Not MS-Windows TAPI           */
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
          dummyTerminateCommunications,
          KWFalse,                     /* Not network oriented        */
          KWTrue,                      /* Buffered                    */
          KWFalse,                  /* Not MS-Windows TAPI           */
          NULL                         /* No network device name      */
        },

#endif /* ARTICOMM */

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
          tTerminateCommunications,
          KWTrue,                      /* Network oriented            */
          KWTrue,                      /* Uses internal buffer        */
          KWFalse,                  /* Not MS-Windows TAPI           */
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
   terminateCommunicationsp =
                        suite[subscript].terminateCommunications;
   network            = suite[subscript].network;
   tapi               = suite[subscript].tapi;

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
   int newMode;

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

   newMode = GetComHandle() << 1 | output;
   if ( traceMode != newMode )
   {
      fprintf( traceStream, "\n%d %s:",
               GetComHandle(),
               (const char *) (output ? "Write" : "Read" ));

      traceMode = newMode;
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
/*       I s T A P I                                                  */
/*                                                                    */
/*       Report if current communications suite is TAPI oriented      */
/*--------------------------------------------------------------------*/

KWBoolean
IsTAPI(void)
{
   return tapi;            /* Preset when suite initialized           */
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

#if     _MSC_VER > 1000
#pragma warning(disable:4100) /* Ignore unused formal parameters */
#endif

void dummySetComHandle( const int foo )
{
}

void dummyTerminateCommunications( void )
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
