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
 *    $Id: commlib.c 1.4 1993/07/13 01:13:32 ahd Exp $
 *
 *    Revision history:
 *    $Log: commlib.c $
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
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                       UUPC/extended includes                       */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "commlib.h"

#include "ulib.h"             // Native communications interface

#ifndef WIN32
#ifndef FAMILYAPI
#include "ulibfs.h"           // FOSSIL interface
#include "ulib14.h"           // ARTISOFT INT14 interface
#endif
#endif

#define NATIVE "internal"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

boolean portActive;         /* Port active flag for error handler   */
boolean traceEnabled;        // Trace active flag

commrefi openlinep, swritep;
commrefu sreadp;
commrefv ssendbrkp, closelinep, SIOSpeedp, flowcontrolp, hangupp;
commrefB GetSpeedp;
commrefb CDp;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static FILE *traceStream;    // Stream used for trace file
static short   traceMode;    // Flag for last data (input/output)
                             // written to trace log

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
        { NATIVE,                      // Default for any opsys
          nopenline, nsread, nswrite,
          nssendbrk, ncloseline, nSIOSpeed, nflowcontrol, nhangup,
          nGetSpeed,
          nCD },
#ifndef WIN32
#ifndef _Windows
#ifndef FAMILYAPI
        { "fossil",                    // MS-DOS FOSSIL driver
          fopenline, fsread, fswrite,
          fssendbrk, fcloseline, fSIOSpeed, fflowcontrol, fhangup,
          fGetSpeed,
          fCD },
        { "articomm",                  // MS-DOS ARTISOFT INT14 driver
          iopenline, isread, iswrite,
          issendbrk, icloseline, iSIOSpeed, iflowcontrol, ihangup,
          iGetSpeed,
          iCD },
#endif
#endif
#endif
        { NULL }                       // End of list
   };

   int subscript = 0;

/*--------------------------------------------------------------------*/
/*                   Search for name in suite table                   */
/*--------------------------------------------------------------------*/

   while (( name  != NULL ) && (suite[subscript].type != NULL ))
   {
      if ( equali(name,suite[subscript].type))
         break;                           // Success!
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

   openlinep     = suite[subscript].openline;
   sreadp        = suite[subscript].sread;
   swritep       = suite[subscript].swrite;
   ssendbrkp     = suite[subscript].ssendbrk;
   closelinep    = suite[subscript].closeline;
   SIOSpeedp     = suite[subscript].SIOSpeed;
   flowcontrolp  = suite[subscript].flowcontrol;
   hangupp       = suite[subscript].hangup;
   GetSpeedp     = suite[subscript].GetSpeed;
   CDp           = suite[subscript].CD;

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

   traceMode  = 2;               // Make sure first trace includes
                                 // prefix with direction

   return TRUE;                  // Success to caller

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
                const short len,
                const boolean output)
{
#ifdef VERBOSE
   int subscript;
#endif

   if ( ! traceEnabled )
      return;

   if ( traceMode != (int) output )
   {
      fputs(output ? "\nWrite: " : "\nRead:  ",traceStream );
      traceMode = (int) output;
   }

#ifdef VERBOSE
   for (subscript = 0; subscript < len; subscript++)
   {
      fprintf( traceStream, "%2.2x", data[subscript] );
   } /* for */
#else

   fwrite(data, 1, len, traceStream ); // Write out raw data

#endif

} /* traceData */
