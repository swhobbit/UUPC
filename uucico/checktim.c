/*--------------------------------------------------------------------*/
/*    c h e c k t i m . c                                             */
/*                                                                    */
/*    Time of day validation routine for UUPC/extended                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: checktim.c 1.5 1994/02/19 05:04:24 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: checktim.c $
 *    Revision 1.5  1994/02/19 05:04:24  ahd
 *    Use standard first header
 *
 * Revision 1.4  1994/01/01  19:17:48  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.3  1993/10/12  01:32:46  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.2  1993/09/20  04:46:34  ahd
 * OS/2 2.x support (BC++ 1.0 support)
 * TCP/IP support from Dave Watt
 * 't' protocol support
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include "checktim.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*--------------------------------------------------------------------*/
/*                          Local functions                           */
/*--------------------------------------------------------------------*/

static char checkone( char *input, size_t hhmm, int weekday );

/*--------------------------------------------------------------------*/
/*   The following day values are based on the fact the               */
/*   localtime() call returns the day of the week as a value zero     */
/*   (0) through six (6), which is converted into the bit number      */
/*   and then AND'ed against the date mask.                           */
/*--------------------------------------------------------------------*/

#define SUN 0x80
#define MON 0x40
#define TUE 0x20
#define WED 0x10
#define THU 0x08
#define FRI 0x04
#define SAT 0x02
#define NEVER 0x00
#define WEEKEND (SAT | SUN)
#define WEEKDAY (MON | TUE | WED | THU | FRI)
#define ANY (WEEKEND | WEEKDAY)

/*--------------------------------------------------------------------*/
/*   Table of values for schedules.  Based on values given for        */
/*   legal schedule keywords in "Managing uucp and Usenet" by         */
/*   O'Reilly & Associates.  Multiple entries for a single keyword    */
/*   are processed in logical OR fashion, just as multiple entries    */
/*   for the same host in L.sys are treated in a logical OR           */
/*   fashion.                                                         */
/*                                                                    */
/*   Timing windows are adjusted five minutes away from higher        */
/*   telephone rates in an attempt to avoid massive charges           */
/*   because of inaccurate PC clocks and the fact that a telephone    */
/*   call generally requires more than one minute if the system is    */
/*   trying to do useful work.                                        */
/*                                                                    */
/*   Does not support multiple keywords in one token                  */
/*   (TuFr0800-0805), but allows multiple tokens                      */
/*   (Tu0800-805,Fr0800-0805) on one line.                            */
/*                                                                    */
/*   The NonPeak keyword has been corrected to the current (May       */
/*   1989) NonPeak hours for Telenet's PC-Pursuit.  However, keep     */
/*   in mind the PC-Pursuit customer agreement specifies that you     */
/*   can't use PC-Pursuit to network multiple PC's, so thou shalt     */
/*   not use PC-Pursuit from a central mail server.  *sigh*           */
/*                                                                    */
/*   I also have Reach-Out America from ATT, for which night rates    */
/*   begin at 10:00 pm.  It is duly added to the table.               */
/*--------------------------------------------------------------------*/

static struct Table {
   char *keyword;
   int wdays;
   unsigned int start;
   unsigned int end;

} table[] = {
   { "Any",     ANY,         0, 2400},
   { "Wk",      WEEKDAY,     0, 2400},
   { "Su",      SUN,         0, 2400},
   { "Mo",      MON,         0, 2400},
   { "Tu",      TUE,         0, 2400},
   { "We",      WED,         0, 2400},
   { "Th",      THU,         0, 2400},
   { "Fr",      FRI,         0, 2400},
   { "Sa",      SAT,         0, 2400},
   { "Evening", WEEKDAY,  1705,  755},
   { "Evening", WEEKEND,     0, 2400},
   { "Night",   WEEKDAY,  2305,  755},
   { "Night",   SAT,         0, 2400},
   { "Night",   SUN,      2305, 1655},
   { "NonPeak", WEEKDAY,  1805,  655}, /* Subject to change at TELENET's whim */
   { "NonPeak", WEEKEND,     0, 2400},
   { "ROA",     WEEKDAY,  2205,  755}, /* Reach Out America (sm) (AT&T) */
   { "ROA",     SAT,         0, 2400}, /* Reach Out America (sm) (AT&T) */
   { "ROA",     SUN,      2205, 1655}, /* Reach Out America (sm) (AT&T) */
   { "Never",   NEVER,       0, 2400},
   {  nil(char) }
}; /* table */

/*--------------------------------------------------------------------*/
/*    c h e c k t i m e                                               */
/*                                                                    */
/*    Validate a time of day field; returns lowest grade              */
/*--------------------------------------------------------------------*/

char checktime(const char *xtime)
{

   struct tm *tm_now;
   time_t secs_now;
   size_t hhmm;
   char  buf[BUFSIZ];
   char  *token;
   char  *nexttoken;
   char  bestgrade = '\0';    /* No grade found yet                  */
   int   weekday;

   strcpy(buf,xtime);         /* Copy time to local buffer we can alter */
   time(&secs_now);
   tm_now = localtime(&secs_now);
                                       /* Create structure with time   */
   weekday = SUN >> tm_now->tm_wday;   /* Get day of week as single bit */
   hhmm = tm_now->tm_hour*100 + tm_now->tm_min;
   nexttoken = buf;           /* First pass, look at start of buffer   */

   while ((bestgrade < ALL_GRADES) &&
          ((token = strtok(nexttoken,",")) != NULL))
   {
      char grade = checkone( token, hhmm, weekday);

      if ( bestgrade < grade )
            bestgrade = grade;

      nexttoken = NULL;       /* Continue parsing same string          */

   } /* while (!(dial) && ((token = strtok(nexttoken,",")) != NULL) ) */

/*--------------------------------------------------------------------*/
/*            Report our results and return to the caller             */
/*--------------------------------------------------------------------*/

   return (bestgrade);

} /*checktime*/

/*--------------------------------------------------------------------*/
/*    c h e c k o n e                                                 */
/*                                                                    */
/*    Process one time field for checktime                            */
/*--------------------------------------------------------------------*/

static char checkone( char *input, size_t hhmm, int weekday )
{
   char  tdays[20];           /* String version of user tokens       */
   char  tstart[20];
   char  tend[20];
   size_t istart;
   size_t iend;
   struct Table *tptr;
   boolean dial = FALSE;      /* Assume we cannot dial               */
   char  found = 0;           /* Did not yet find current keyword    */
   char grade = ALL_GRADES;   /* Default grade if none specified     */

   char *slash = strchr( input, '/' );

/*--------------------------------------------------------------------*/
/*     Parse a day/time combination from the L.SYS file         *     */
/*--------------------------------------------------------------------*/

   strcpy(tstart,"0000");  /* Set default times to all day           */
   strcpy(tend,"2400");

/*--------------------------------------------------------------------*/
/*                      Determine the call grade                      */
/*--------------------------------------------------------------------*/

   if ( slash != NULL )
   {

      if (strlen( slash ) != 2)
      {
         printmsg(0,"Invalid call grade in field: %s", input );
         panic();
      }

      *slash++ = '\0';     /* Terminate original string              */
      grade = *slash;      /* Save the grade the user wanted         */

   } /* if ( slash != NULL ) */

/*--------------------------------------------------------------------*/
/*         Get the period and time limits the user specified          */
/*--------------------------------------------------------------------*/

    sscanf(input,
           "%[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]"
           "%[0123456789]-%[0123456789]", tdays, tstart, tend);

/*--------------------------------------------------------------------*/
/*          Verify the lengths of the fields the user specified       */
/*--------------------------------------------------------------------*/

   if (strlen(tstart) >= sizeof tstart)
   {
      printf( "%d character buffer overflow \"%s\"\n",
               sizeof tstart , tstart );
      panic();
   }

   if (strlen(tend) >= sizeof tend)
   {
      printf( "%d character buffer overflow \"%s\"\n",
               sizeof tend , tend );
      panic();
   }

   if (strlen(tdays) >= sizeof tdays)
   {
      printf( "%d character buffer overflow \"%s\"\n",
               sizeof tdays, tdays );
      panic();
   }

   printmsg(8,"checkone: %s broken into \"%s\" from \"%s\" to \"%s\""
              " with grade %c",
            input,tdays,tstart,tend,grade );

   istart = atoi(tstart);  /* Convert start/end times to binary       */
   iend  = atoi(tend);

/*--------------------------------------------------------------------*/
/*    Handle case of midnight specified in such a way that the        */
/*    time wraps through the daylight hours; we'll take the           */
/*    conservative approach that the user really meant to start at    */
/*    midnight.                                                       */
/*--------------------------------------------------------------------*/

   if ((istart > iend) && (istart == 2400))
      istart = 0000;

/*--------------------------------------------------------------------*/
/*                  Search for the requested keyword                  */
/*--------------------------------------------------------------------*/

   for (tptr = table, found = FALSE;
         (tptr->keyword != nil(char)) && !dial; tptr++)
   {

/*--------------------------------------------------------------------*/
/*      We found the keyword, see if today qualifies for dialing      */
/*--------------------------------------------------------------------*/

      if (equal(tptr->keyword,tdays))
      {
         found = TRUE;     /* Win or Lose, keyword is valid          */
         if (weekday & tptr->wdays)    /* Can we dial out today?     */
         {                             /* Yes --> Check the time     */

/*--------------------------------------------------------------------*/
/*    This entry allows us to dial out today; now determine if the    */
/*    time slot in the table allows dialing.                          */
/*--------------------------------------------------------------------*/

            if (tptr->start > tptr->end)  /* span midnight?          */
               dial = (tptr->start <= hhmm) || (tptr->end >= hhmm);
            else
               dial = (tptr->start <= hhmm) && (tptr->end >= hhmm);

/*--------------------------------------------------------------------*/
/*    Now do a logical AND of that time with the time the user        */
/*    specified in L.sys for this particular system.                  */
/*--------------------------------------------------------------------*/

            if (istart > iend)            /* span midnight?          */
               dial = ((istart <= hhmm) || (iend >= hhmm)) && dial;
            else if (istart == iend)
               dial = (istart == hhmm) && dial;
            else
               dial = (istart <= hhmm) && (iend >= hhmm) && dial;
         } /* if */
      } /* if */
   } /* for */

   if (!found)
      printmsg(0,"checkone: keyword \"%s\" not found",input);

   printmsg(3,"checkone: call window \"%s\" %s",
         input,
         dial ? "open" :"closed");

/*--------------------------------------------------------------------*/
/*                Return success or failure to caller                 */
/*--------------------------------------------------------------------*/

   if ( dial )
      return grade;
   else
      return '\0';

} /* checkone */
