/*--------------------------------------------------------------------*/
/*          Program:    ulibtest.c           28 Nov 1992              */
/*          Author:     Andrew H. Derbyshire                          */
/*                      Kendra Electronic Wonderworks                 */
/*                      P.O. Box 132                                  */
/*                      Arlington, MA 02174-0002 USA                  */
/*          Internet:   help@kew.com                                  */
/*          Function:   Driver program for testing UUPC/extended      */
/*                      ULIB communications packages                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Copyright (c) 1992 by Kendra Electronic Wonderworks;         */
/*       all rights reserved except those explicitly granted          */
/*       through the UUPC/extended license.                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id$
 *
 *    $Log$
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "ulib.h"
#include "catcher.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

static char text[] =
"Only the first letter of each command is examined; operands are\n"
"are seperated by white space.  Defaults for commands are in\n"
"parentheses.  Commands may be issued out of sequence, in which case\n"
"the results are unpredictable.\n\n"
"open [port [speed [direct]]]\t(COM1, 2400, and 0 (modem))\n"
"close\n"
"send [text]\t\t\t(this help text)\n"
"Send [file]\t\t\t(console input)\n"
"receive [timeout]\t\t(5 seconds for up to 512 characters)\n"
"Receive [timeout] [file]\t(5 seconds, received.dat)\n"
"hangup\n"
"?\n"
"debuglevel (4)\n"
"quit\n";

static int opened = 0;

currentfile();

/*--------------------------------------------------------------------*/
/*    h e l p                                                         */
/*                                                                    */
/*    Print help text                                                 */
/*--------------------------------------------------------------------*/

static void help( void )
{
   puts(text);
}

/*--------------------------------------------------------------------*/
/*    o p e n i t                                                     */
/*                                                                    */
/*    Open communications port                                        */
/*--------------------------------------------------------------------*/

static void openit( char *buf )
{
   char *token,
        *port = "COM1";
    BPS speed = 2400;
    int direct = 0;

   token  = strtok( buf , WHITESPACE );
   if ( token != NULL )
   {
      port = token;
      token  = strtok( NULL , WHITESPACE );
   }

   if ( token != NULL )
   {
      speed = atoi(token);
      token  = strtok( NULL , WHITESPACE );
   }

   if ( token != NULL )
      direct = atoi(token);

   strupr( port );
   printf("openline( %s, %d, %d ) -- ", port, speed, direct );
   if ( openline( port, speed,  direct))
      printf("failed\n");
   else {
      printf("succeeded\n");
      opened = 1;
   }

} /* openit */

/*--------------------------------------------------------------------*/
/*    s e n d i t                                                     */
/*                                                                    */
/*    Send text to communications port                                */
/*--------------------------------------------------------------------*/

static void sendit( char *buf )
{
   char *first = buf;
   int len;

   if ( ! opened )
   {
      puts("Port isn't open ... issue o command first");
      return;
   }

   while( (first != NULL ) && *first && ! isprint(*first))
      first++;

   if (( first == NULL ) || ! (*first))
      first = text;
   else
      strcat( first, "\r\n" );

   len = strlen( first );
   printf( "swrite( <text>, %d) -- ", len );
   len = swrite( first, len );
   printf("%d characters written\n", len);

} /* sendit */

/*--------------------------------------------------------------------*/
/*    r e c v e i v e i t                                             */
/*                                                                    */
/*    Receive data to console from serial port                        */
/*--------------------------------------------------------------------*/

static void receiveit( char *buf )
{

   char *token;
   int timeout = 5;
   int len = 512;
   int actual = 0;

   if ( ! opened )
   {
      puts("Port isn't open ... issue o command first");
      return;
   }

   token  = strtok( buf , WHITESPACE );
   if ( token != NULL )
   {
      timeout = atoi( token );
      token  = strtok( NULL , WHITESPACE );
   }

   if ( token != NULL )
      len = atoi( token );

   token = malloc( len + 1 );

   while( (actual < len))
   {
      if ( actual )
         len = actual;
      printf("sread( <buffer>,  %d, %d ) -- ", len, timeout );
      actual = sread( token, len, timeout );
      printf( "%d characters available\n", actual);
      timeout = 0;

      if ( terminate_processing )
         return;

      if ( !actual )
         break;
   }

   if ( actual )
   {
      token[actual] = '\0';
      puts(token);
   } /* if */

   free( token );

} /* receiveit */

/*--------------------------------------------------------------------*/
/*    s e n d f i l e                                                 */
/*                                                                    */
/*    Send text to communications port from file                      */
/*--------------------------------------------------------------------*/

static void sendfile( char *buf )
{
   char *fname = strtok( buf, WHITESPACE );

   FILE *stream;

   if ( ! opened )
   {
      puts("Port isn't open ... issue o command first");
      return;
   }

   if ( fname == NULL )
      fname = "CON";

   stream = fopen( fname, "rb" );

   if ( stream == NULL )
   {
      perror( fname );
      return;
   }

   printf("Reading data from %s:\n",fname );

   for ( ;; )
   {
      char buf[BUFSIZ];
      int len;
      int actual;

      len = fread( buf, sizeof *buf, sizeof buf, stream );
      if ( ferror( stream ))
      {
         perror( fname );
         return;
      }

      if ( feof( stream ))
      {
         puts("EOF\n");
         return;
      }

      printf( "swrite( <text>, %d) -- ", len );
      actual = swrite( buf, len );
      printf("%d characters written\n", actual);

      if ( terminate_processing )
        return;

      if ( actual != len)
         break;
   }

} /* sendfile */

/*--------------------------------------------------------------------*/
/*    r e c e i v e f i l e                                           */
/*                                                                    */
/*    Receive data to console from serial port                        */
/*--------------------------------------------------------------------*/

static void receivefile( char *buf )
{

   char *token;
   char *fname = "received.dat";
   int timeout = 5;
   FILE *stream;
   int len = BUFSIZ;

   if ( ! opened )
   {
      puts("Port isn't open ... issue o command first");
      return;
   }

   token  = strtok( buf , WHITESPACE );
   if ( token != NULL )
   {
      timeout = atoi( token );
      token  = strtok( NULL , WHITESPACE );
   }

   if ( token != NULL )
   {
      len = atoi( token );
      token  = strtok( NULL , WHITESPACE );
   }

   if ( token != NULL )
      fname = token;

   stream  = fopen( fname, "ab" );
   if ( stream == NULL )
   {
      perror( fname );
      return;
   }

   for ( ;; )
   {

      char buf[512];
      int actual = 0;

      while( (len > actual) && (len) )
      {
         if ( actual )
         {
            len = actual;
            actual = 0;
            timeout = 0;
         }

         printf("sread( <buffer>,  %d, %d ) -- ", len, timeout );
         actual = sread( buf, len, timeout );
         printf( "%d characters available\n", actual);

         if ( terminate_processing )
             return;

      } /* while */

      if ( len > 0 )
      {
         actual = fwrite( buf, sizeof *buf, len, stream );

         if ( actual != len )
         {
            perror( fname );
            return;
         } /* if */

      } /* if */
      else
         break;

   } /* for */

   fclose( stream );

} /* receivefile */

static void shutdown( void )
{
   if ( opened )
      closeline();
}

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    main program                                                    */
/*--------------------------------------------------------------------*/

void main( int argc, char ** argv )
{
   char buf[BUFSIZ];
   int done = 0;
   char *next, *command;

   banner( argv );

   if (!configure( B_MUA ))
      exit(1);    /* system configuration failed */

   debuglevel = 4;

/*--------------------------------------------------------------------*/
/*                        Trap control C exits                        */
/*--------------------------------------------------------------------*/

    if( signal( SIGINT, ctrlchandler ) == SIG_ERR )
    {
        printmsg( 0, "Couldn't set SIGINT\n" );
        panic();
    }

   if (!configure( B_MUA ))
      exit(1);    /* system configuration failed */

/*--------------------------------------------------------------------*/
/*                 Insure comm port is closed at exit                 */
/*--------------------------------------------------------------------*/

   atexit( shutdown );
   interactive_processing = FALSE;     /* Quit immediately           */

   while ( ! done )
   {
      if ( terminate_processing )
         break;

      if ( opened )
         printf("Opened, debuglevel %d, CD reports: %s\n",
                  debuglevel,
                  CD() ? "TRUE" : "FALSE" );
      else
         printf("Closed, debuglevel %d\n", debuglevel );

      printf("Enter command (? for help): ");

      if ( terminate_processing )
         break;

      if ( fgets( buf, sizeof buf, stdin ) == NULL)
         break;

      fputc('\n',stdout );
      command = strtok( buf, WHITESPACE );
      next    = strtok( NULL, "\r\n" );

      switch( *command )
      {
         case 'o':
            openit( next );
            break;

         case 'c':
            printf("closeline() --");
            closeline();
            opened = 0;
            fputc('\n', stdout);
            break;

         case 's':
            sendit( next );
            break;

         case 'r':
            receiveit( next );
            break;

         case 'S':
            sendfile( next );
            break;

         case 'R':
            receivefile( next );
            break;

         case 'h':
            printf("hangup() --");
            hangup();
            fputc('\n', stdout);
            break;

         case '?':
            help();
            break;

         case 'q':
            done = 1;
            break;

         case 'd':
            next = strtok( next, WHITESPACE );
            if ( next != NULL )
               debuglevel = atoi( next );
            else
               debuglevel = 4;
            break;

         default:
            printf("Invalid command '%c', try ? for help\n",
                     *command );
            break;

      } /* switch */
   } /* for */

   exit(0);

} /* main */
