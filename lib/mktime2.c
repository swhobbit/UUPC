/*--------------------------------------------------------------------*/
/*    paranoid version of mktime() for Borland C++                    */
/*                                                                    */
/*    Written by Gary Blaine (TeamB) and posted to CompuServe         */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
 #include <time.h>
 #include <dos.h>
 #include <mem.h>

static int   day_tab[2][12] = {
   {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static int _Cdecl isleap(int year);

time_t _Cdecl mktime(struct tm * tm)
{
   struct tm   tmp;
   int         temp;
   int         days;
   time_t      gmt_seconds;
   struct date date;
   struct time time;

   tzset();

   if( tm->tm_sec  < 0  ||
       tm->tm_min  < 0  ||
       tm->tm_hour < 0  ||
       tm->tm_mday < 0  ||
       tm->tm_mon  < 0  ||
       tm->tm_year < 0 )
      return(-1);

   tmp = *tm;
   temp = tmp.tm_mon % 12;
   tmp.tm_year += tmp.tm_mon / 12 + 1900;
   tmp.tm_mon = temp;
   if(tmp.tm_year < 0)
      return(-1);
   temp = tmp.tm_sec % 60;
   tmp.tm_min += tmp.tm_sec / 60;
   tmp.tm_sec = temp;
   if(tmp.tm_min < 0)
      return(-1);
   temp = tmp.tm_min % 60;
   tmp.tm_hour += tmp.tm_min / 60;
   tmp.tm_min = temp;
   if(tmp.tm_hour < 0)
      return(-1);
   temp = tmp.tm_hour % 24;
   tmp.tm_mday += tmp.tm_hour / 24;
   tmp.tm_hour = temp;
   if(tmp.tm_mday < 0)
      return(-1);
   while(tmp.tm_mday  >  (days = day_tab[isleap(tmp.tm_year)][tmp.tm_mon]))
      {
      tmp.tm_mday -= days;
      tmp.tm_mon++;
      if(tmp.tm_mon > 11)
         {
         tmp.tm_mon = 0;
         if(++tmp.tm_year < -1)
            return(-1);
         }
      }
   if(tmp.tm_year < 1980)
      return(-1);
   time.ti_hour = tmp.tm_hour;
   time.ti_min = tmp.tm_min;
   time.ti_sec = tmp.tm_sec;
   time.ti_hund = 0;
   date.da_year = tmp.tm_year;
   date.da_mon = tmp.tm_mon + 1;
   date.da_day = tmp.tm_mday;
   gmt_seconds = dostounix(&date, &time);
   memcpy( tm, localtime(&gmt_seconds), sizeof( struct tm ) );
   return(gmt_seconds);
}

static int _Cdecl isleap(int year)
{
   return(year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
}
