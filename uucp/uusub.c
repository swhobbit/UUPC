/*--------------------------------------------------------------------*/
/*    u u s u b . c                                                   */
/*                                                                    */
/*    Report summary of UUPC activity                                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Copyright (c) 1989-1994 by Kendra Electronic Wonderworks        */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: uusub.c 1.9 1994/01/24 03:19:33 ahd Exp $
 *
 *    $Log: uusub.c $
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
static char *status( hostatus current_status );
static char *format( long l);

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

void main( int argc , char **argv)
{

   int         option;
   boolean clear_stats = FALSE;
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
         case 'c':               /* clear stats                       */
            clear_stats = TRUE;
            break;
         case 's':               /* only named host                    */
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

} /* main */

/*--------------------------------------------------------------------*/
/*    s h o w s t a t s                                               */
/*                                                                    */
/*    Display information on all hosts                                */
/*--------------------------------------------------------------------*/

static void showstats( const char *name )
{
   struct HostTable *host;
   boolean firsthost = TRUE;
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
         firsthost = FALSE;
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
   checkref( host->hstats );
   line( host->hostname,
      status( host->hstatus ),
      when( host->hstats->lconnect ),
      when( host->hstats->ltime ),
      format( host->hstats->connect ),
      format( host->hstats->bsent ),
      format( host->hstats->breceived ),
      format( host->hstats->fsent ),
      format( host->hstats->freceived ),
      format( host->hstats->errors  ));
} /* showhost */

/*--------------------------------------------------------------------*/
/*                            Subroutines                             */
/*--------------------------------------------------------------------*/

static char *when( time_t t )
{
   column += 13;
   return dater( t, &output[column]);
} /* when */

static char *format( long l)
{
   if (l == 0)
      return "";

   column += 12;
   if ( l <= 99999)
      sprintf( &output[ column ], "%ld", l);
   else if ( (l/1000) <= 9999)
      sprintf( &output[ column ], "%ldK", l / 1000);
   else
      sprintf( &output[ column ], "%ldM", l / 1000000);
   return &output[column];

} /* format */

static char *status( hostatus current_status )
{
   switch ( current_status )
   {
      default:
       return "??????";

      case  phantom:          /* Entry not fully initialized           */
            return "noinit";

      case  localhost:        /* This entry is for ourselves           */
            return "local";

      case  gatewayed:        /* This entry is delivered to via        */
                              /* an external program on local sys     */
            return "gatway";

      case  nocall:           /* real host: never called              */
         return "NEVER";

      case autodial:          /* Calling now                          */
         return "DIALNG";

      case nodevice:          /* Device open failed                    */
         return "NODEV";

      case startup_failed:
         return "NSTART";

      case  inprogress:       /* Call now active                       */
         return "INPROG";

      case invalid_device:    /* Bad systems file entry               */
         return "INVDEV";

      case  callback_req:     /* System must call us back              */
          return "CALLBK";

      case  dial_script_failed:
                              /* Hardcoded auto-dial failed           */
         return "NDIALS";

      case  dial_failed:      /* Hardcoded auto-dial failed           */
         return "NODIAL";

      case  script_failed:    /* script in L.SYS failed               */
         return "NSCRPT";

      case  max_retry:        /* Have given up calling this sys        */
         return "MAXTRY";

      case  too_soon:         /* In retry mode: too soon to call      */
         return "TOSOON";

      case  succeeded:        /* self-explanatory                     */
      case  called:           /* self-explanatory                     */
         return "SUCESS";

      case  wrong_host:       /* Call out failed: wrong system        */
         return "WRGHST";

      case  unknown_host:     /* Call in cailed: unknown system        */
         return "UNKNWN";

      case  wrong_time:       /* Unable to call because of time        */
         return "WRGTIM";

      case  call_failed:      /* Connection dropped in mid-call        */
         return "FAILED";
   } /* switch */

} /* status */
