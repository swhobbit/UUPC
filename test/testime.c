/*--------------------------------------------------------------------*/
/*       Program:    testtime.c           2/29/92                     */
/*       Author:     Andrew H. Derbyshire                             */
/*                   P. O. Box 132                                    */
/*                   Arlington, MA 02174                              */
/*                   Internet: ahd@kew.com                            */
/*       Function:   Test for bug (system hung) in Borland C++ 2.0    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    It appears Borland C++ 2.0 function mktime() hangs on leap      */
/*    day; this program tests that theory.                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                       Standard include files                       */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
/*                    Internal (local) prototypes                     */
/*--------------------------------------------------------------------*/

void static report( const char *date_string );

/*--------------------------------------------------------------------*/
/*    m a i n                                                         */
/*                                                                    */
/*    Main program, of course                                         */
/*--------------------------------------------------------------------*/

void main( void )
{

/*--------------------------------------------------------------------*/
/*                       Announce our existence                       */
/*--------------------------------------------------------------------*/

   printf("testtime.c: Copyright (c) 1992 by Andrew H. Derbyshire\n"
          "May be distributed freely if no money is charged for the\n"
          "program and source is included\n\n");

/*--------------------------------------------------------------------*/
/*                         Run the test cases                         */
/*--------------------------------------------------------------------*/

   report("date 02/28/92");

   report("date 03/01/92");

   report("date 02/29/92");         /* Fails on this call            */

   report("rem");                   /* Current date                  */

} /* main */

/*--------------------------------------------------------------------*/
/*    r e p o r t                                                     */
/*                                                                    */
/*    Process one test case (date)                                    */
/*--------------------------------------------------------------------*/

void static report( const char *date_string )
{
   struct tm  *time_record;
   time_t secs = time( NULL );
   time_t save;

/*--------------------------------------------------------------------*/
/*                       Save real system time                        */
/*--------------------------------------------------------------------*/

   time(&save);

/*--------------------------------------------------------------------*/
/*             Set DOS system date through SYSTEM command             */
/*--------------------------------------------------------------------*/

   printf("Executing command: %s\n",date_string);
   system(date_string);

/*--------------------------------------------------------------------*/
/*                         Get adjusted time                          */
/*--------------------------------------------------------------------*/

   time( &secs ) ;
   time_record = localtime(&secs);

/*--------------------------------------------------------------------*/
/*                           Restore clock                            */
/*--------------------------------------------------------------------*/

   stime( &save );

/*--------------------------------------------------------------------*/
/*                      Display the altered time                      */
/*--------------------------------------------------------------------*/

   printf("Input time:\t%s",ctime( &secs ));

   secs = mktime(time_record);   /* This call HANGS on 02/29/92      */

   printf("Output time:\t%s\n",ctime( &secs ));

} /* report */
