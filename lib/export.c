/*--------------------------------------------------------------------*/
/*    e x p o r t . c                                                 */
/*                                                                    */
/*    File name mapping routines for UUPC/extended                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2000 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: export.c 1.16 1999/01/08 02:20:43 ahd Exp $
 *
 *    Revision history:
 *    $Log: export.c $
 *    Revision 1.16  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.15  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.14  1998/03/01 01:23:33  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1997/03/31 06:59:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1996/01/01 20:51:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.11  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.10  1994/12/22 00:08:21  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/03/13 17:22:20  ahd
 *    Add debugging statement
 *
 *     Revision 1.8  1994/02/19  04:41:08  ahd
 *     Use standard first header
 *
 *     Revision 1.7  1994/02/19  04:05:37  ahd
 *     Use standard first header
 *
 *     Revision 1.6  1994/02/19  03:49:46  ahd
 *     Use standard first header
 *
 *     Revision 1.5  1994/02/18  23:08:59  ahd
 *     Use standard first header
 *
 *     Revision 1.4  1994/01/01  19:01:46  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.3  1993/10/12  00:41:51  ahd
 *     Normalize comments
 *
 *     Revision 1.2  1993/10/09  15:46:15  rhg
 *     ANSIify the source
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

#include "arbmath.h"
#include "export.h"
#include "import.h"
#include "usertabl.h"
#include "hostable.h"
#include "security.h"

RCSID("$Id: export.c 1.16 1999/01/08 02:20:43 ahd Exp $");

/*--------------------------------------------------------------------*/
/*       e x p o r t p a t h                                          */
/*                                                                    */
/*       Convert a local environment name to UNIX format name         */
/*--------------------------------------------------------------------*/

void exportpath(char *canon, const char *host, const char *remote)
{
   const char *xhost;
   char *copy;
   char tempname[FILENAME_MAX];
   unsigned subscript;
   unsigned char number[MAX_DIGITS];
   char *token, *out;

   static size_t range =  UNIX_END_C - UNIX_START_C + 1;
                              /* Determine unique number characters in
                                 the UNIX file names we are mapping   */
   size_t charsetsize;
            /* Number of allowed characters in
                              MS-DOS file names                   */

#ifdef UDEBUG
   printmsg(5,"Exporting %s for %s", host, remote );
#endif

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

   token = strtok( NULL, "/");   /* Get variable part of name         */
   while( (*token != '\0') && (*number == '\0'))
   {
      unsigned char digit;
      mult(number, charsetsize, MAX_DIGITS); /* Shift the number over */
      digit = (unsigned char) (strchr( E_charset , *token++) - E_charset);
      add(number, digit , MAX_DIGITS); /* Add in new low order        */
      if (*token == '.')               /* Next character a period?    */
         token ++;                     /* Yes --> Ignore it           */
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

      subscript = (unsigned) (*out - UNIX_START_C);
                              /* Convert back to pure number          */
      token = canon + strlen( canon ); /* Remember end of string      */
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
