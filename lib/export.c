/*--------------------------------------------------------------------*/
/*    e x p o r t . c                                                 */
/*                                                                    */
/*    File name mapping routines for UUPC/extended                    */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "lib.h"
#include "arbmath.h"
#include "export.h"
#include "import.h"
#include "usertabl.h"
#include "hostable.h"
#include "security.h"

currentfile();

void exportpath(char *canon, const char *host, char const *remote)
{
   const char *xhost;
   char *copy;
   char tempname[FILENAME_MAX];
   size_t subscript;
   unsigned char number[MAX_DIGITS];
   char *token, *out;

   static size_t range =  UNIX_END_C - UNIX_START_C + 1;
                              /* Determine unique number characters in
                                 the UNIX file names we are mapping  */
   size_t charsetsize;
            /* Number of allowed characters in
                              MS-DOS file names                   */

/*--------------------------------------------------------------------*/
/*                      Define our character set                      */
/*--------------------------------------------------------------------*/

   if ( E_charset == NULL )
      E_charset = DOSCHARS;

   charsetsize = strlen( E_charset );

/*--------------------------------------------------------------------*/
/*                Drop leading spool directory, if any                */
/*--------------------------------------------------------------------*/

   if (equalni(host, E_spooldir, strlen( E_spooldir )))
      xhost = host + strlen( E_spooldir ) + 1;
   else
      xhost = host;

   copy = strdup( xhost );
   checkref( copy );

/*--------------------------------------------------------------------*/
/*                        Drop the remote name                        */
/*--------------------------------------------------------------------*/

   token = strtok( copy, "/");

   if ((token == NULL) || !equaln( token, remote, strlen( token )))
   {
      printmsg(0,"exportpath: Badly formed host name \"%s\"",xhost);
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 Get the character leading the name                 */
/*--------------------------------------------------------------------*/

   token = strtok( NULL, "/");
   if ( (token == NULL) || (strlen(token) != 1))
   {
      printmsg(0,"exportpath: Badly formed host name \"%s\"",xhost);
      panic();
   }

   strcpy(canon, token);
   strcat(canon, ".");

/*--------------------------------------------------------------------*/
/*       Create a binary number which represents our file name        */
/*--------------------------------------------------------------------*/

   for (subscript = 0; subscript < MAX_DIGITS; subscript++ )
      number[subscript] = 0;  /* Initialize number to zero        */

   token = strtok( NULL, "/");   /* Get variable part of name        */
   while( (*token != '\0') && (*number == '\0'))
   {
      unsigned char digit;
      mult(number, charsetsize, MAX_DIGITS); /* Shift the number over   */
      digit = (unsigned char) (strchr( E_charset , *token++) - E_charset);
      add(number, digit , MAX_DIGITS); /* Add in new low order       */
      if (*token == '.')               /* Next character a period?   */
         token ++;                     /* Yes --> Ignore it          */
   } /* while */

   out = &tempname[FILENAME_MAX];
   *--out = '\0';          /* Terminate the string we will build  */

/*--------------------------------------------------------------------*/
/*         Here's the loop to actually do the base conversion         */
/*--------------------------------------------------------------------*/

      while(adiv( number, range, &subscript, MAX_DIGITS))
       *--out = (char) (subscript + UNIX_START_C);

/*--------------------------------------------------------------------*/
/*    We sort of lied above; the first character out of the           */
/*    conversion is not a character at all, but bits which say how    */
/*    many characters the remote and local file names get prefixed    */
/*    to the converted name.  Retrieve that information now           */
/*--------------------------------------------------------------------*/

      subscript = *out - UNIX_START_C;
                              /* Convert back to pure number         */
      token = canon + strlen( canon ); /* Remember end of string     */
      if (subscript > HOSTLEN)
      {
         subscript /= HOSTLEN;
         strcat( canon, remote );
      }
      else
         strcat( canon, E_nodename );
      token[ subscript ] = '\0';    /* Only use the length we were told */

/*--------------------------------------------------------------------*/
/*               Add in the variable name and we're done              */
/*--------------------------------------------------------------------*/

      strcat( canon, ++out );
      free( copy );

/*--------------------------------------------------------------------*/
/*                          Check the result                          */
/*--------------------------------------------------------------------*/

      importpath( tempname, canon, remote );
      if ( !equal( tempname, xhost ))
      {
         printmsg(0,
            "exportpath: **mapping error** input \"%s\","
            " result \"%s\", import \"%s\"",
            xhost, canon, tempname );
         panic();
      } /* if */

} /* exportpath */
