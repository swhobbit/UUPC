/*--------------------------------------------------------------------*/
/*    u u n a m e . c                                                 */
/*                                                                    */
/*    Known site lister for UUPC/extended                             */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*         System include files                                       */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*         Local include files                                        */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "getopt.h"
#include "hlib.h"
#include "hostable.h"
#include "security.h"
#include "timestmp.h"

currentfile();

/*--------------------------------------------------------------------*/
/*                             Verb list                              */
/*--------------------------------------------------------------------*/

typedef enum {
   LIST_DEFAULT = 1,
   LIST_LOCAL,
   LIST_DOMAIN
   } COMMAND_CLASS;

/*--------------------------------------------------------------------*/
/*                        Internal prototypes                         */
/*--------------------------------------------------------------------*/

static void all( void );

static void local( void );

static void domain( void );

static void usage( void );

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

void main(int  argc, char  **argv)
{
   int c;
   COMMAND_CLASS command = LIST_DEFAULT;
   extern char *optarg;
   extern int   optind;

/*--------------------------------------------------------------------*/
/*     Report our version number and date/time compiled               */
/*--------------------------------------------------------------------*/

   debuglevel = 0;

   banner( argv );

#if defined(__CORE__)
   copywrong = strdup(copyright);
   checkref(copywrong);
#endif

/*--------------------------------------------------------------------*/
/*        Process our arguments                                       */
/*--------------------------------------------------------------------*/

   while ((c = getopt(argc, argv, "dlx:")) !=  EOF)
      switch(c) {
      case 'x':
         debuglevel = atoi( optarg );
         break;

      case 'd':
         command = LIST_DOMAIN;
         break;

      case 'l':
         command = LIST_LOCAL;
         break;

      case '?':
         usage();
   }

   if (optind != argc) {
      puts("Extra parameter(s) at end.");
      exit(2);
   }

   if (!configure( B_UUSTAT ))
      exit(1);   /* system configuration failed */

/*--------------------------------------------------------------------*/
/*                   Execute the requested command                    */
/*--------------------------------------------------------------------*/

   switch ( command )
   {
      case LIST_DEFAULT:
             all( );
             break;

      case LIST_LOCAL:
             local( );
             break;

      case LIST_DOMAIN:
             domain( );
             break;

      default:
             panic();

   } /* switch */

   exit(0);

} /* main */

/*--------------------------------------------------------------------*/
/*    a l l                                                           */
/*                                                                    */
/*    Display all connected systems names                             */
/*--------------------------------------------------------------------*/

static void all( )
{
   struct HostTable *hostp = nexthost( TRUE );

/*--------------------------------------------------------------------*/
/*                  Scan one or all host directories                  */
/*--------------------------------------------------------------------*/

   while  (hostp !=  BADHOST )
   {
      printf("%s\n", hostp->hostname);

/*--------------------------------------------------------------------*/
/*    If processing all hosts, step to the next host in the queue     */
/*--------------------------------------------------------------------*/

      hostp = nexthost( FALSE );

   } /* while */

} /* all */

/*--------------------------------------------------------------------*/
/*    l o c a l                                                       */
/*                                                                    */
/*    Display local systems node name                                 */
/*--------------------------------------------------------------------*/

static void local( )
{
   printf("%s\n", E_nodename);
} /* local */

/*--------------------------------------------------------------------*/
/*    d o m a i n                                                     */
/*                                                                    */
/*    Display local systems domain name                               */
/*--------------------------------------------------------------------*/

static void domain( )
{
   printf("%s\n", E_domain);
} /* domain */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report how to use program                                       */
/*--------------------------------------------------------------------*/

static void usage( void )
{

   fputs("Usage:\tuuname\t[-l|-d]\n\
\tDefault is to display all known uucp systems.\n\
\t-d\t\tDisplays local node's domain name.\n\
\t-l\t\tDisplays local node's uucp name.\n",
            stdout );

   exit(1);

} /* usage */
