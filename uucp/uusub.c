/*--------------------------------------------------------------------*/
/*    u u s u b . c                                                   */
/*                                                                    */
/*    Report summary of UUPC activity                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) 1989-1997 by Kendra Electronic Wonderworks        */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uusub.c 1.17 1996/11/18 04:46:49 ahd Exp $
 *
 *    $Log: uusub.c $
 *    Revision 1.17  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.16  1996/01/01 21:34:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.15  1995/07/21 13:18:16  ahd
 *    Change hostatus type to HOSTSTATUS
 *
 *    Revision 1.14  1995/03/11 15:49:23  ahd
 *    Clean up compiler warnings, modify dcp/dcpsys/nbstime for better msgs
 *
 *    Revision 1.13  1995/01/07 16:41:48  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.12  1994/12/22 00:44:57  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1994/02/20 19:11:18  ahd
 *    IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.10  1994/02/19  05:15:33  ahd
 * Use standard first header
 *
 * Revision 1.9  1994/01/24  03:19:33  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1994/01/01  19:27:44  ahd
 * Annual Copyright Update
 *
 * Revision 1.7  1993/10/12  01:34:47  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.6  1993/10/03  20:43:08  ahd
 * Normalize comments to C++ double slash
 *
 * Revision 1.5  1993/07/06  11:02:06  ahd
 * Load host status information after parsing args
 *
 * Revision 1.4  1993/05/09  12:44:25  ahd
 * Reset collection time start before writing it out to disk
 *
 * Revision 1.3  1993/04/11  00:35:46  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/19  03:03:24  ahd
 * drop rcsid
 *
 */

#include "uupcmoah.h"

#include "hostable.h"
#include "dater.h"
#include "hostrset.h"
#include "hostatus.h"
#include "getopt.h"
#include "security.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                            Local macros                            */
/*--------------------------------------------------------------------*/

#define line( a, b, c, d, e, f, g, h, i, j ) \
      printf("%-8.8s %-6.6s %-11.11s %-11.11s %5.5s %5.5s %5.5s %5.5s %5.5s %5.5s\n" ,\
      a, b, c, d, e, f, g, h, i ,j )

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void showstats( const char *name );
static void showhost( struct HostTable *host);
static char *when( time_t t );
static char *status( HOSTSTATUS current_status );
static char *format( unsigned long l);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char output[10 * 12];
static size_t column ;

currentfile();

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report flags used by program                                    */
/*--------------------------------------------------------------------*/

static         void    usage(void)
{
      fprintf(stderr, "Usage: uusub\t[-c] [-s <nodename>] [-x debug]\n");
}

/*--------------------------------------------------------------------*/
/*                            main program                            */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#endif

main( int argc , char **argv)
{

   int         option;
   KWBoolean clear_stats = KWFalse;
   char *name = nil(char);

/*--------------------------------------------------------------------*/
/*               Announce ourselves to a waiting world                */
/*--------------------------------------------------------------------*/

   debuglevel = 0;

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

   banner( argv );

/*--------------------------------------------------------------------*/
/*                        Process option flags                        */
/*--------------------------------------------------------------------*/

   while ((option = getopt(argc, argv, "cs:x:")) != EOF)  {
      switch(option)  {
         case 'c':               /* clear stats                      */
            clear_stats = KWTrue;
            break;
         case 's':               /* only named host                  */
            name = optarg;
            break;
         case 'x':
            debuglevel = atoi(optarg);
            break;
         default:
            usage();
            exit(1);
            break;
      }
   }

   if (optind != argc)
   {
      puts("Extra parameter(s) at end.");
      exit(4);
   }

/*--------------------------------------------------------------------*/
/*       Load system configuration and then the UUPC host stats       */
/*--------------------------------------------------------------------*/

   if (!configure( B_UUSTAT ))
      panic();

   HostStatus();

/*--------------------------------------------------------------------*/
/*            Now display or clear the loaded information             */
/*--------------------------------------------------------------------*/

   if ( (name != NULL) && (checkreal( name ) == BADHOST) )
       printf("Unknown host \"%s\"\n", name );
   else if (clear_stats)
   {
       if ( name == NULL )
         time( &start_stats );
       HostReset((const char *)name);
   }
   else
       showstats((const char *)name);

   return 0;

} /* main */

/*--------------------------------------------------------------------*/
/*    s h o w s t a t s                                               */
/*                                                                    */
/*    Display information on all hosts                                */
/*--------------------------------------------------------------------*/

static void showstats( const char *name )
{
   struct HostTable *host;
   KWBoolean firsthost = KWTrue;
   static const char *dashes = "-----------";

   printf("Host information collected since %s\n",ctime( &start_stats ));

   line("Host","Host ",  "Date Last",  "Last Conn","Secs" , "Bytes", "Bytes",
         "Files", "Files", "Total");
   line("Name","Status ","Connected ","Attempt", "Conn",  "Sent",  "Recvd",
        "Sent",  "Recvd", "Errs");
   line(dashes,dashes,dashes,dashes,dashes,dashes,dashes,dashes,
         dashes,dashes);

   if (name != NULL)
      showhost ( checkreal(name) );
   else
      while  ((host = nexthost( firsthost )) != BADHOST)
      {
         firsthost = KWFalse;
         showhost ( host );
      } /* while */

} /* showstats */

/*--------------------------------------------------------------------*/
/*    s h o w h o s t                                                 */
/*                                                                    */
/*    Display information on a single host                            */
/*--------------------------------------------------------------------*/

static void showhost( struct HostTable *host)
{
   column = 0;

   line( host->hostname,
      status( (HOSTSTATUS) host->status.hstatus ),
      when( host->status.lconnect ),
      when( host->status.ltime ),
      format( host->status.connect ),
      format( host->status.bsent ),
      format( host->status.breceived ),
      format( host->status.fsent ),
      format( host->status.freceived ),
      format( host->status.errors  ));

} /* showhost */

/*--------------------------------------------------------------------*/
/*                            Subroutines                             */
/*--------------------------------------------------------------------*/

static char *when( time_t t )
{
   column += 13;
   return dater( t, &output[column]);
} /* when */

static char *format( unsigned long l)
{
   if (l == 0)
      return "";

   column += 12;

   if ( l <= 99999)
      sprintf( &output[ column ], "%lu", l);
   else if ( (l/1000) <= 9999)
      sprintf( &output[ column ], "%luK", l / 1000);
   else
      sprintf( &output[ column ], "%luM", l / 1000000);

   return &output[column];

} /* format */

static char *status( HOSTSTATUS current_status )
{
   switch ( current_status )
   {
      default:
       return "??????";

      case HS_PHANTOM:        /* Entry not fully initialized         */
            return "noinit";

      case HS_LOCALHOST:      /* This entry is for ourselves         */
            return "local";

      case HS_GATEWAYED:      /* This entry is delivered to via      */
                              /* an external program on local sys    */
            return "gatway";

      case HS_SMTP:           /* SMTP gateway host                   */
            return "SMTP-G";

      case HS_NOCALL:         /* real host: never called             */
         return "NEVER";

      case HS_AUTODIAL:       /* Calling now                         */
         return "DIALNG";

      case HS_NODEVICE:       /* Device open failed                  */
         return "NODEV";

      case HS_STARTUP_FAILED:
         return "NSTART";

      case HS_INPROGRESS:     /* Call now active                     */
         return "INPROG";

      case HS_INVALID_DEVICE: /* Bad systems file entry              */
         return "INVDEV";

      case HS_CALLBACK_REQ:   /* System must call us back            */
          return "CALLBK";

      case HS_DIAL_SCRIPT_FAILED:
                              /* Hardcoded auto-dial failed          */
         return "NDIALS";

      case HS_DIAL_FAILED:    /* Hardcoded auto-dial failed          */
         return "NODIAL";

      case HS_SCRIPT_FAILED:  /* script in L.SYS failed              */
         return "NSCRPT";

      case HS_MAX_RETRY:      /* Have given up calling this sys      */
         return "MAXTRY";

      case HS_TOO_SOON:       /* In retry mode: too soon to call     */
         return "TOSOON";

      case HS_SUCCEEDED:      /* self-explanatory                    */
      case HS_CALLED:         /* self-explanatory                    */
         return "SUCESS";

      case HS_WRONG_HOST:     /* Call out failed: wrong system       */
         return "WRGHST";

      case HS_UNKNOWN_HOST:   /* Call in cailed: unknown system      */
         return "UNKNWN";

      case HS_WRONG_TIME:     /* Unable to call because of time      */
         return "WRGTIM";

      case HS_CALL_FAILED:    /* Connection dropped in mid-call      */
         return "FAILED";
   } /* switch */

} /* status */
