/*--------------------------------------------------------------------*/
/*    c o m m 3 4 . c                                                 */
/*                                                                    */
/*    Set/report communications port addresses                        */
/*                                                                    */
/*    Modified to report addresses and report usage by Andrew H.      */
/*    Derbyshire.  Changes Copyright (c) 1992, Andrew H.              */
/*    Derbyshire.                                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: E:\SRC\UUPC\UTIL\RCS\COMM34.C 1.1 1992/11/15 04:29:22 ahd Exp $
 *
 *    Revision history:
 *    $Log: COMM34.C $
 * Revision 1.1  1992/11/15  04:29:22  ahd
 * Initial revision
 *
 * Revision 1.2  1992/04/27  00:38:58  ahd
 * Add RCS header
 *
 */

static char rcsid[] = "$Id: COMM34.C 1.1 1992/11/15 04:29:22 ahd Exp $";

/*
 * Compiler: Microsoft C 5.1
 *
 * COM34.C - set COM3 - COM4 addresses
 *
 * This program inserts the addresses specified by argv[1] - argv[2]
 * into the BIOS table at 0040:0004 (COM3 - COM4)
 *
 * Usage: COMM4 xxx yyy
 * where  xxx is COM3 address (hex), yyy is COM4 address (hex)
 */

/*--------------------------------------------------------------------*/
/*                      C standard include files                      */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "timestmp.h"

#define MK_FP(seg,ofs)  ((void far *)(((unsigned long)(seg) << 16) | (ofs)))

static struct addr far *ports = (struct addr far *) MK_FP(0x40, 0);

unsigned htoi( char * buf );
static void status( void );
static void usage( void );

struct addr
{
   unsigned com[4];
};

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program                                                    */
/*--------------------------------------------------------------------*/

void main(int argc, char **argv)
{
   unsigned htoi();

/*--------------------------------------------------------------------*/
/*                        Report our parentage                        */
/*--------------------------------------------------------------------*/

   banner( argv );

   switch( argc )
   {
      case 3:
         ports->com[3] = htoi(argv[2]);

      case 2:
         ports->com[2] = htoi(argv[1]);
         break;

      default:
         usage();
   }

/*--------------------------------------------------------------------*/
/*                     Report new status and exit                     */
/*--------------------------------------------------------------------*/

   status();
   exit( 0 );

} /* main */

/*--------------------------------------------------------------------*/
/*    s t a t u s                                                     */
/*                                                                    */
/*    Report current serial port status                               */
/*--------------------------------------------------------------------*/

static void status( void )
{
   int port = 0;

   printf("Communications ports: ");

   while( port < 4 )
   {
      if (port)
         printf(", ");

      if (ports->com[port])
         printf("COM%d = %x",port+1, ports->com[port] );
      else
         printf("COM%d = (none)",port+1);

      port++;
   } /* while */

   putchar('\n');

} /* status */

/*--------------------------------------------------------------------*/
/*    h t o i                                                         */
/*                                                                    */
/*    Convert printable hex to integer                                */
/*--------------------------------------------------------------------*/

unsigned htoi(char *buf)
{
   unsigned sum;
   char c;

   for (sum = 0; (c = *buf) != '\0'; buf++)
   {
      if ((c >= 'a') && (c <= 'f'))
         c = c - 'a' + 'A';

      if ((c >= 'A') && (c <= 'F'))
         sum = (sum * 16) + (c - 'A' + 10);

      else if ((c >= '0') && (c <= '9'))
         sum = (sum * 16) + (c - '0');

      else
      {
         printf("Invalid port address %s, program exiting.\n",buf);
         usage();
      }
   }
   return(sum);

} /* htoi */

/*--------------------------------------------------------------------*/
/*    u s a g e                                                       */
/*                                                                    */
/*    Report program usage                                            */
/*--------------------------------------------------------------------*/

static void usage( void )
{
   printf(
      "This program alters the BIOS information for COM ports 3 and 4 for\n"
      "use by UUPC/extended, MS-Kermit and other programs, and reports the\n"
      "addresses of all installed COM ports.  Usage:\n\n"
      "\tCOMM34\taddr3 [addr4]\n\n"
      "Where addr3 is the hexadecimal address for port 3 (usually 2E8) and\n"
      "Where addr4 is the optional hexadecimal address for port 4 (usually\n"
      "2E0).\n\n");

   status();

   exit(1);
}
