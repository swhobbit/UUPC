/*--------------------------------------------------------------------*/
/*    s c r i p t . c                                                 */
/*                                                                    */
/*    Script processing routines for UUPC/extended                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Originally by John H. DuBois III  3/31/90                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: script.c 1.15 1995/08/27 23:34:11 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: script.c $
 *    Revision 1.15  1995/08/27 23:34:11  ahd
 *    Correct compile warnings
 *
 *    Revision 1.14  1995/02/21 02:47:44  ahd
 *    The compiler warnings war never ends!
 *
 *    Revision 1.13  1995/01/07 16:39:42  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.12  1994/12/22 00:36:04  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1994/05/23 22:46:32  ahd
 *    Use atol() for getting speed changes
 *
 *        Revision 1.10  1994/02/19  05:09:59  ahd
 *        Use standard first header
 *
 *        Revision 1.10  1994/02/19  05:09:59  ahd
 *        Use standard first header
 *
 * Revision 1.9  1994/01/01  19:20:47  ahd
 * Annual Copyright Update
 *
 * Revision 1.8  1993/12/30  03:26:21  ahd
 * Trap empty failure strings
 *
 * Revision 1.7  1993/12/24  05:12:54  ahd
 * Support for checking echoing of transmitted characters
 *
 * Revision 1.6  1993/11/30  04:18:14  ahd
 * Share buffer between input and output
 *
 * Revision 1.6  1993/11/30  04:18:14  ahd
 * Share buffer between input and output
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dcp.h"
#include "dcpsys.h"
#include "hostable.h"
#include "modem.h"
#include "script.h"
#include "security.h"
#include "ssleep.h"
#include "catcher.h"
#include "usrcatch.h"
#include "commlib.h"

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

static KWBoolean Match( char *Search,
                      char *Buffer,
                      size_t *SearchPos);

static size_t MatchInit( const char *MatchStr );

static KWBoolean writestr(register char *s,
                     unsigned int timeout,
                     char **failure);

static void flushScriptBuffer( void );

static KWBoolean slowWrite( char *s, size_t len, char **failure);

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

static char scriptBuffer[MAXMATCH]; /* Shared between input & output  */
                                 /* Can be shorter then longest send  */
                                 /* string, as longer strings are     */
                                 /* sent without buffering            */

static size_t scriptBufferLen = 0;

static unsigned int waitForEcho = 0;   /* Timeout for each echo      */

static KWBoolean echoMode = KWFalse;     /* Looking for echo char      */

/*--------------------------------------------------------------------*/
/*       e x p e c t s t r                                            */
/*                                                                    */
/*       wait for a pattern on input                                  */
/*                                                                    */
/*       expectstr reads characters from input using sread, and       */
/*       compares them to a search string.  It reads characters       */
/*       until either the search string has been seen on the input    */
/*       or a specified timeout interval has passed without any       */
/*       characters being read.                                       */
/*                                                                    */
/*      Global variables: none.                                       */
/*                                                                    */
/*      Input parameters:                                             */
/*      Search is the string that is searched for on the input.       */
/*      Timeout is the timeout interval passed to sread.              */
/*                                                                    */
/*      Output parameters: none.                                      */
/*                                                                    */
/*      Return value:                                                 */
/*      KWTrue is returned if the search string is found on input.     */
/*      KWFalse is returned if sread times out.                        */
/*--------------------------------------------------------------------*/

int expectstr(char *Search, unsigned int Timeout, char **failure)
{
   char buf[BUFSIZ];
   int result;
   time_t quit = time( NULL ) + (time_t) Timeout;
   register char *ptr = buf;

   if ( ! echoMode )
      printmsg(2, "wanted \"%s\" in %ld seconds",
                  Search,
                  (long) Timeout );

   if (!strlen(Search))                      /* expects nothing */
       return KWTrue;

   StrMatch(Search,'\0', failure);    /* set up search string */

   do {

      if (ptr == &buf[BUFSIZ-1])
        ptr = buf;          /* Save last character for term \0  */

      if (sread(ptr , 1, (int) (quit - time(NULL))) < 1)
      {
                              /* The scan failed?                    */
         char *s;

         if ( terminate_processing || raised )
            return 0;

         while ( ptr > buf )
         {
            if (*(--ptr) > ' ')
               break;    /* Locate the last printable char      */
         }

         *(ptr+1) = '\0';   /* Terminate the string             */

         for ( s = buf; (*s > '\0') && (*s <= ' '); s++ )
         {
         }
                            /* Locate the first printable char  */

         while ( ptr-- > s )/* Zap control chars                */
            if (*ptr < ' ')
               *ptr = '?';

         if (( debuglevel < 2 ) || echoMode )
         {
            if ( (strlen( Search ) == 1) && iscntrl( *Search ) )
               printmsg(1, "wanted \"^%c\"", Search + 64 );
            else
               printmsg(1, "wanted \"%s\"", Search);
         }

         printmsg(1, "got ??? \"%s\"",s );
         return KWFalse;

      } /* if (sread(ptr , 1, Timeout) < 1) */

      *ptr &= 0x7f;

      result = StrMatch(Search, *ptr++, failure);

   } while (!result);

   return result;

} /* expectstr */

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
      memset(scriptBuffer,'\0',sizeof scriptBuffer);    /* Clear buffer */
      PutPos = 0;

      SearchPosition = MatchInit( MatchStr );

      alternates = 0;
      if ( failure != NULL )
      {

         while (failure[alternates] != NULL )
         {
            if ( strlen( failure[alternates] ) )
               SearchPos[alternates] = MatchInit( failure[alternates] );
            else {
               printmsg(0,"Empty NOCONNECT string is invalid!");
               panic();
            }

            alternates++;         /* Step to next string in the list  */

         } /* while (failure[alternates] != NULL ) */

      } /* if ( failure != NULL ) */

      return 0;

   } /* if (MatchStr) */

/*--------------------------------------------------------------------*/
/*                       Look for primary match                       */
/*--------------------------------------------------------------------*/

   scriptBuffer[ PutPos++ & QINDMASK] = C;

   if (Match( MatchStr, scriptBuffer, &SearchPosition))
   {
      if ( ! echoMode )
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
         if (Match( failure[subscript], scriptBuffer, &SearchPos[subscript]))
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

static KWBoolean Match( char *Search,
                      char *scriptBuffer,
                      size_t *SearchPos)
{
   size_t BufInd;          /* Index to input string buffer for string
                              compare */
   char *SearchInd;        /* Index to search string for string
                              compare */

   *SearchPos += 1;
   for (BufInd = *SearchPos, SearchInd = Search; *SearchInd; SearchInd++)
   {
     if (scriptBuffer[BufInd++ & QINDMASK] != *SearchInd)
        return KWFalse;
   }

   return KWTrue;

} /* Match */

/*--------------------------------------------------------------------*/
/*    M a t c h I n i t                                               */
/*                                                                    */
/*    Initialize one set of parameters for MATCH                      */
/*--------------------------------------------------------------------*/

static size_t MatchInit( const char *MatchStr )
{
   size_t SearchLen = strlen(MatchStr);

   if (SearchLen > sizeof scriptBuffer)
   {
      printmsg(0,"StrMatch: String to match '%s' is too long.",
           MatchStr);
      panic();
   }

   return sizeof scriptBuffer - SearchLen;

} /* MatchInit */

/*--------------------------------------------------------------------*/
/*    w r i t e s t r                                                 */
/*                                                                    */
/*    Send a string to the port during login                          */
/*--------------------------------------------------------------------*/

static KWBoolean writestr(register char *s,
                        unsigned int timeout,
                        char **failure)
{
   register char last = '\0';
   KWBoolean writeCR = KWTrue;
   unsigned char digit;

   while (*s)
   {
      if (last == '\\') {
         last = *s;
         switch (*s) {
         case 'd':   /* delay */
         case 'D':
            flushScriptBuffer();
            ssleep(2);
            break;

         case 'e':   /* Echo checking off                         */
            echoCheck( 0 );
            break;

         case 'E':   /* echo checking on                          */
            echoCheck( timeout );
            break;

         case 'c':   /* don't output CR at end of string */
         case 'C':
            writeCR = KWFalse;
            break;

         case 'r':   /* carriage return */
         case 'R':
         case 'm':
         case 'M':
            if (!slowWrite("\r", 1, failure))
               return KWFalse;
            break;

         case 'n':   /* new line */
         case 'N':
            if (!slowWrite("\n", 1, failure))
               return KWFalse;
            break;

         case 'p':   /* delay */
         case 'P':
            flushScriptBuffer();
            ddelay(400);
            break;

         case 'b':   /* backspace */
         case 'B':
            if (slowWrite("\b", 1, failure))
               return KWFalse;
            break;

         case 't':   /* tab */
         case 'T':
            if (!slowWrite("\t", 1, failure))
               return KWFalse;
            break;

         case 's':   /* space */
         case 'S':
            if (!slowWrite(" ", 1, failure))
               return KWFalse;
            break;

         case 'z':   /* set serial port speed */
         case 'Z':
            flushScriptBuffer();
            SIOSpeed((BPS) atol(++s));
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
            if (!slowWrite((char *) &digit,1, failure))
               return KWFalse;
            break;

         default: /* ordinary character */
            if (!slowWrite(s, 1, failure))
               return KWFalse;
            last = '\0';      /* Zap any repeated backslash (\)      */
         }
      }
      else if (*s != '\\') /* backslash */
      {
         if ( !slowWrite(s, 1, failure))
            return KWFalse;
      }
      else
         last = *s;
      s++;

   }  /* while */

   if ( writeCR )
      if ( !slowWrite( "\r", 1 , failure))
         return KWFalse;

   flushScriptBuffer();          /* Handle any queued data on net    */

   return KWTrue;                 /* Return success to caller         */

} /* writestr */

/*--------------------------------------------------------------------*/
/*       s e n d s t r                                                */
/*                                                                    */
/*       Send line of login sequence                                  */
/*--------------------------------------------------------------------*/

KWBoolean  sendstr(char *str, unsigned int timeout, char **failure)
{
   KWBoolean success;

   printmsg(2, "sending \"%s\"", str);

   if (equaln(str, "BREAK", 5))
   {
      unsigned nulls = (unsigned) atoi(&str[5]);

      if (nulls <= 0 || nulls > 10)
         nulls = 3;

      ssendbrk(nulls);  /* send a break signal */

      return KWTrue;

   }  /* if (equaln(str, "BREAK", 5)) */

   if ( waitForEcho )
   {
      echoCheck( timeout );
   }
   echoMode = KWTrue;

   if (equal(str, "EOT"))
   {
      success = slowWrite(EOTMSG, strlen(EOTMSG), failure);
      flushScriptBuffer();
   }
   else {
      if (equal(str, "\"\""))
         *str = '\0';

      success = writestr(str, timeout, failure);

   } /* else */

   echoMode = KWFalse;

   if ( ! success )
      printmsg(0,"sendstr: Did not receive echo of string \"%s\"",str);

   return success;

} /* sendstr */

/*--------------------------------------------------------------------*/
/*       e c h o C h e c k                                            */
/*                                                                    */
/*       Enable/disable checking for echoed characters                */
/*--------------------------------------------------------------------*/

void echoCheck( const unsigned int timeout )
{
   if ( (waitForEcho && !timeout) || (timeout && !waitForEcho) )
      printmsg(2,"echoCheck: %sabled", timeout ? "en" : "dis" );

   if ( waitForEcho )
      flushScriptBuffer();

   waitForEcho = timeout;

} /* echoCheck */

/*--------------------------------------------------------------------*/
/*    s l o w w r i t e                                               */
/*                                                                    */
/*    Write characters to the serial port at a configurable           */
/*    snail's pace.                                                   */
/*--------------------------------------------------------------------*/

static KWBoolean slowWrite( char *s, size_t len, char **failure)
{

/*--------------------------------------------------------------------*/
/*       We don't have to flush before the char delay because the     */
/*       character delay is constant for one connection-- on or off   */
/*--------------------------------------------------------------------*/

   if ( M_charDelay || waitForEcho )
   while ( len-- )
   {
      swrite( s , 1 );
      if ( M_charDelay )
         ddelay(M_charDelay);

      if ( waitForEcho )
      {
         char exp[2];

         exp[0] = *s;
         exp[1] = '\0';          /* Terminate the string             */

         if (!expectstr(exp, waitForEcho, failure ))
            return KWFalse;

      } /* if ( waitForEcho ) */

      s++;

   } /* while ( len-- ) */
   else {

      if ( (scriptBufferLen + len) > sizeof scriptBuffer )
         flushScriptBuffer();

      if ( len == 1 )
         scriptBuffer[ scriptBufferLen++ ] = *s;
      else if ( len >= sizeof scriptBuffer )
         swrite( s , len );
      else {
         memcpy( scriptBuffer + scriptBufferLen, s, len );
         scriptBufferLen += len;
      } /* else */

   } /* else */

   return KWTrue;

} /* slowWrite */

/*--------------------------------------------------------------------*/
/*       f l u s h S c r i p t B u f f e r                            */
/*                                                                    */
/*       Flush queued network I/O                                     */
/*--------------------------------------------------------------------*/

static void flushScriptBuffer( void )
{
   if ( scriptBufferLen )
   {
      swrite( scriptBuffer, scriptBufferLen );
      scriptBufferLen = 0;
   }

} /* flushScriptBuffer */
