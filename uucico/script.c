/*--------------------------------------------------------------------*/
/*    s c r i p t . c                                                 */
/*                                                                    */
/*    Script processing routines for UUPC/extended                    */
/*                                                                    */
/*    John H. DuBois III  3/31/90                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dcp.h"
#include "dcpsys.h"
#include "hostable.h"
#include "hlib.h"
#include "modem.h"
#include "script.h"
#include "security.h"
#include "ssleep.h"
#include "catcher.h"
#include "ulib.h"

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

#define MAXMATCH 64              /* max length of search string; must
                                    be a power of 2                  */
#define QINDMASK (MAXMATCH - 1)  /* bit mask to get queue index      */

#define EOTMSG "\004\r\004\r"

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

static int StrMatch(char *MatchStr, char C, char **failure);
                                 /* Internal match routine           */

static boolean Match( char *Search,
                      char *Buffer,
                      size_t *SearchPos);

static size_t MatchInit( const char *MatchStr );

static boolean writestr(register char *s);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

/*
 *       e x p e c t s t r
 *
 *       wait for a pattern on input
 *
 *
 *       expectstr reads characters from input using sread, and
 *       compares them to a search string.  It reads characters until
 *       either the search string has been seen on the input or a
 *       specified timeout interval has passed without any characters
 *       being read.
 *
 *      Global variables: none.
 *
 *      Input parameters:
 *      Search is the string that is searched for on the input.
 *      Timeout is the timeout interval passed to sread.
 *
 *      Output parameters: none.
 *
 *      Return value:
 *      TRUE is returned if the search string is found on input.
 *      FALSE is returned if sread times out.
 */

int expectstr(char *Search, unsigned int Timeout, char **failure)
{
   char buf[BUFSIZ];
   int result;
   time_t quit = time( NULL ) + Timeout;
   register char *ptr = buf;

   printmsg(2, "wanted \"%s\"", Search);

   if (!strlen(Search))                      /* expects nothing */
       return TRUE;

   StrMatch(Search,'\0', failure);    /* set up search string */

   do {
      if (ptr == &buf[BUFSIZ-1])
        ptr = buf;          /* Save last character for term \0  */

      if (sread(ptr , 1, (int) (quit - time(NULL))) < 1)
      {
                              /* The scan failed?                    */
         char *s;

         if ( terminate_processing )
            return 0;

         while ( ptr > buf )
            if (*(--ptr) > ' ')
               break;    /* Locate the last printable char      */

         *(ptr+1) = '\0';   /* Terminate the string             */

         for ( s = buf; (*s > '\0') && (*s <= ' '); s++ );
                            /* Locate the first printable char  */

         while ( ptr-- > s )/* Zap control chars                */
            if (*ptr < ' ')
               *ptr = '?';

         if ( debuglevel < 2 )
            printmsg(1, "wanted \"%s\"", Search);

         printmsg(1, "got ??? \"%s\"",s );
         return FALSE;

      } /* if (sread(ptr , 1, Timeout) < 1) */

      *ptr &= 0x7f;

      result = StrMatch(Search, *ptr++, failure);
   } while (!result);

   return result;

} /*expectstr*/


/*
 *      StrMatch: Incrementally search for a string.
 *      John H. DuBois III  3/31/90
 *      StrMatch searches for a string in a sequence of characters.
 *      The string to search for is passed in an initial setup call
 *      (input character in this call is \0)
 *      Further calls with the search string pass one
 *      character per call.
 *      The characters are built up into an input string.
 *      After each character is added to the input string,
 *      the search string is compared to the last length(search string)
 *      characters of the input string to determine whether the search
 *      string has been found.
 *
 *      Global variables: none.
 *
 *      Input parameters:
 *      MatchStr is the string to search for.
 *      C is the character to add to the input string.
 *      It is ignored on a setup call.
 *
 *      Output parameters: None.
 *
 *      Return value:
 *      On the setup call, -1 is returned if the search string is
 *      longer than the input string buffer.  Otherwise, 0 is returned.
 *
 *      On comparison calls,
 *          1 is returned if the search string has been found.
 *          > 1 is returned if a failure string has been found.
 *          Otherwise 0 is returned.
 */


static int StrMatch(char *MatchStr, char C, char **failure)
{
/*
 *      The input string is stored in a circular buffer of MAXMATCH
 *      characters.  If the search string is found in the input,
 *      then the last character added to the buffer will be the last
 *      character of the search string.  Therefore, the string
 *      compare will always start SearchLen characters behind the
 *      position where characters are added to the buffer.
 */

   static char Buffer[MAXMATCH];       /* Input string buffer */
   static size_t PutPos;               /* Where to add chars to buffer */

   static size_t SearchPos[MAXLIST];
   static size_t SearchPosition;
                                       /* Buffer loc to start compare */
   static size_t alternates;
   size_t subscript;

/*--------------------------------------------------------------------*/
/*                       Handle initialize call                       */
/*--------------------------------------------------------------------*/

   if ( C == '\0')
   {                                   /* Set up call */
      memset(Buffer,'\0',MAXMATCH);    /* Clear buffer */
      PutPos = 0;

      SearchPosition = MatchInit( MatchStr );

      alternates = 0;
      if ( failure != NULL )
      {
         while (failure[alternates] != NULL )
         {
            SearchPos[alternates] = MatchInit( failure[alternates] );
            alternates++;
         } /* while (failure[alternates] != NULL ) */

      } /* if ( failure != NULL ) */

      return 0;
   } /* if (MatchStr) */

/*--------------------------------------------------------------------*/
/*                       Look for primary match                       */
/*--------------------------------------------------------------------*/

   Buffer[ PutPos++ & QINDMASK] = C;

   if (Match( MatchStr, Buffer, &SearchPosition))
   {
      printmsg(2, "got that");
      return 1;
   }

/*--------------------------------------------------------------------*/
/*                     Look for alternate matches                     */
/*--------------------------------------------------------------------*/

   if ( alternates > 0 )
   {
      subscript = alternates;

      while ( subscript-- )
      {
         if (Match( failure[subscript], Buffer, &SearchPos[subscript]))
         {
            printmsg(0,"got \"%s\" (failure)",failure[subscript]);
            return 2;
         }
      } /* while ( subscript-- ) */

   } /* if ( alternates > 0 ) */

   return 0;

} /* StrMatch */

/*--------------------------------------------------------------------*/
/*    m a t c h                                                       */
/*                                                                    */
/*    Match a single string                                           */
/*--------------------------------------------------------------------*/

static boolean Match( char *Search,
                      char *Buffer,
                      size_t *SearchPos)
{
   int BufInd;             /* Index to input string buffer for string
                              compare */
   char *SearchInd;        /* Index to search string for string
                              compare */

   *SearchPos += 1;
   for (BufInd = *SearchPos, SearchInd = Search; *SearchInd; SearchInd++)
   {
     if (Buffer[BufInd++ & QINDMASK] != *SearchInd)
        return FALSE;
   }

   return TRUE;

} /* Match */

/*--------------------------------------------------------------------*/
/*    M a t c h I n i t                                               */
/*                                                                    */
/*    Initialize one set of parameters for MATCH                      */
/*--------------------------------------------------------------------*/

static size_t MatchInit( const char *MatchStr )
{
   size_t SearchLen = strlen(MatchStr);

   if (SearchLen > MAXMATCH)
   {
      printmsg(0,"StrMatch: String to match '%s' is too long.\n",
           MatchStr);
      panic();
   }

   return MAXMATCH - SearchLen;

} /* MatchInit */

/*--------------------------------------------------------------------*/
/*    w r i t e s t r                                                 */
/*                                                                    */
/*    Send a string to the port during login                          */
/*--------------------------------------------------------------------*/

static boolean writestr(register char *s)
{
   register char last = '\0';
   boolean nocr  = FALSE;
   unsigned char digit;

   if equal(s,"BREAK")
   {
      ssendbrk(0);
      return TRUE;               /* Don't bother with a CR after this   */
   }
   while (*s) {
      if (last == '\\') {
         last = *s;
         switch (*s) {
         case 'd':   /* delay */
         case 'D':
            ssleep(2);
            break;
         case 'c':   /* don't output CR at end of string */
         case 'C':
            nocr = TRUE;
            break;
         case 'r':   /* carriage return */
         case 'R':
         case 'm':
         case 'M':
            slowwrite("\r", 1);
            break;
         case 'n':   /* new line */
         case 'N':
            slowwrite("\n", 1);
            break;
         case 'p':   /* delay */
         case 'P':
            ddelay(400);
            break;
         case 'b':   /* backspace */
         case 'B':
            slowwrite("\b", 1);
            break;
         case 't':   /* tab */
         case 'T':
            slowwrite("\t", 1);
            break;
         case 's':   /* space */
         case 'S':
            slowwrite(" ", 1);
            break;
         case 'z':   /* set serial port speed */
         case 'Z':
            SIOSpeed(atoi(++s));
            while (isdigit(*(s+1)))
               s++;
            break;

         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
            digit = 0;
            while( (*s >= '0') && (*s < '8'))
               digit = (unsigned char) (digit * 8 + *s++ - '0');
            s--;              /* Backup before non-numeric char      */
            slowwrite((char *) &digit,1);
            break;

         default: /* ordinary character */
            slowwrite(s, 1);
            last = '\0';      /* Zap any repeated backslash (\)      */
         }
      }
      else if (*s != '\\') /* backslash */
         slowwrite(s, 1);
      else
         last = *s;
      s++;
   }

   return nocr;

} /*writestr*/


/*
   s e n d s t r

   Send line of login sequence
*/

void sendstr(char *str)
{
   printmsg(2, "sending \"%s\"", str);

   if (equaln(str, "BREAK", 5)) {
      int   nulls;
      nulls = atoi(&str[5]);
      if (nulls <= 0 || nulls > 10)
         nulls = 3;
      ssendbrk(nulls);  /* send a break signal */
      return;
   }

   if (equal(str, "EOT")) {
      slowwrite(EOTMSG, strlen(EOTMSG));
      return;
   }

   if (equal(str, "\"\""))
      *str = '\0';

   if (!equal(str,"")) {
      if (!writestr(str)) {
         slowwrite("\r", 1);
      }
   } else
      slowwrite("\r", 1);
   return;

} /*sendstr*/
