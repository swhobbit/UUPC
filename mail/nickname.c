/*--------------------------------------------------------------------*/
/*    n i c k n a m e . c                                             */
/*                                                                    */
/*    Smart routing and nickname routines for UUPC/extend mail        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*                                                                    */
/*    Additional code                                                 */
/*       Copyright (c) Richard H. Lamb 1985, 1986, 1987               */
/*       Changes Copyright (c) Stuart Lynne 1987                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: nickname.c 1.29 1999/01/08 02:21:05 ahd Exp $
 *
 *    Revision history:
 *    $Log: nickname.c $
 *    Revision 1.29  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.28  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.27  1998/03/01 01:33:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.26  1997/05/11 04:27:40  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.25  1997/04/24 01:10:12  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.24  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.23  1996/01/01 23:50:26  ahd
 *    Don't scan nickname table for duplicate nicknames in linear fashion,
 *    merely check entire table for duplicates after sorting.
 *    Rename user functions previously known as 'user alias' to 'nickname',
 *    consistent with newer documentation.
 *
 *    Revision 1.22  1996/01/01 21:04:06  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.21  1995/11/30 03:06:56  ahd
 *    Trap truly invalid addresses in tokenizer
 *
 *    Revision 1.20  1995/01/30 04:08:36  ahd
 *    Additional compiler warning fixes
 *
 *    Revision 1.19  1995/01/07 16:18:27  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.18  1994/12/22 00:18:29  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.17  1994/10/23 23:29:44  ahd
 *    Add additional checks for format of aliases
 *
 * Revision 1.16  1994/04/24  20:35:08  ahd
 * Change case of userElements
 *
 * Revision 1.15  1994/03/07  06:09:51  ahd
 * Additional debugging messages controlled by UDEBUG
 * Shorten ReturnAddress line buffer to correct problem with length
 *
 * Revision 1.14  1994/02/28  01:02:06  ahd
 * Cosmetic formatting cleanups
 *
 * Revision 1.13  1994/02/20  19:07:38  ahd
 * IBM C/Set 2 Conversion, memory leak cleanup
 *
 * Revision 1.12  1994/02/19  04:17:41  ahd
 * Use standard first header
 *
 * Revision 1.12  1994/02/19  04:17:41  ahd
 * Use standard first header
 *
 * Revision 1.11  1994/01/01  19:12:06  ahd
 * Annual Copyright Update
 *
 * Revision 1.10  1993/12/23  03:16:03  rommel
 * OS/2 32 bit support for additional compilers
 *
 * Revision 1.9  1993/10/25  01:21:22  ahd
 * Rename Aliases to Nickname to make more unique for end users; allow
 * Aliases as obsolete alias for now.
 *
 * Revision 1.8  1993/10/12  01:32:08  ahd
 * Normalize comments to PL/I style
 *
 * Revision 1.6  1993/07/21  01:19:16  ahd
 * Incr elements after filling info from passwd, not before!
 *
 * Revision 1.5  1993/07/19  02:52:11  ahd
 * Don't load alias for empty names
 *
 * Revision 1.4  1993/05/06  03:41:48  ahd
 * Use expand_path to get reasonable correct drive for aliases file
 *
 * Revision 1.3  1993/04/11  00:33:05  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/22  21:06:14  ahd
 * Use strpool for memory allocation
 *
 *    02 Oct 89   Alter large strings/structures to use
 *                malloc()/free()                              ahd
 *    08 Feb 90   Correct failure of ExtractAddress to return
 *                non-names                                    ahd
 *    18 Mar 90   Move checkname() and associated routines into
 *                hostable.c                                   ahd
 *    22 Apr 90   Modify user_at_node to correctly handle .UUCP
 *                alias on local host.                         ahd
 *
 */

#include "uupcmoah.h"

#include <ctype.h>

#ifndef __TURBOC__
#include <search.h>
#endif

#include "hostable.h"
#include "security.h"
#include "usertabl.h"
#include "nickname.h"
#include "address.h"
#include "expath.h"

static size_t nicknameCount = 0;

static NICKNAME *nickname = NULL;

static size_t loadAliases( void ) ;

RCSID("$Id: nickname.c 1.29 1999/01/08 02:21:05 ahd Exp $");

/*--------------------------------------------------------------------*/
/*   n i c k C o m p a r e                                            */
/*                                                                    */
/*   Accepts indirect pointers to two strings and compares them using */
/*   stricmp (case insensitive string compare)                        */
/*--------------------------------------------------------------------*/

int nickCompare( const void *a, const void *b )
{
   return stricmp(((NICKNAME *)a)->anick,
         ((NICKNAME *)b)->anick);

}  /* nickCompare */

/*--------------------------------------------------------------------*/
/*    I n i t R o u t e r                                             */
/*                                                                    */
/*    Verify, initialize the global routing data                      */
/*--------------------------------------------------------------------*/

KWBoolean InitRouter()
{
   KWBoolean success = KWTrue;     /* Assume the input data is good      */
   struct HostTable *hostP = checkname( E_mailserv );

/*--------------------------------------------------------------------*/
/*          Verify that the user gave us a good name server           */
/*--------------------------------------------------------------------*/

   if (hostP == BADHOST)
   {
      printmsg(0, "mail server '%s' must be listed in SYSTEMS or HOSTPATH file",
                  E_mailserv);
      success = KWFalse;
   }
   else if ( hostP->status.hstatus >= HS_NOCALL )
      printmsg( 8, "mailserver '%s' is real system.", hostP->hostname );
   else if ( hostP->status.hstatus == HS_SMTP)
      printmsg(2,"mailserver '%s' is an SMTP relay via %s",
                 hostP->hostname,
                 hostP->via );
   else if ( hostP->status.hstatus == HS_GATEWAYED)
      printmsg(2,"mailserver '%s' is via gateway program '%s'",
                  hostP->hostname,
                  hostP->via );
   else if (hostP->status.hstatus == HS_LOCALHOST)  /* local system?     */
   {
      printmsg(0, "'%s' is name of this host and cannot be mail server",
            hostP->hostname);
      success = KWFalse;
   }
   else {
      printmsg(0, "'%s' has an invalid status of %d; check SYSTEMS and HOSTPATH file.",
            hostP->hostname,
            hostP->status.hstatus);
      success = KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return success;

} /* InitRouter */

/*--------------------------------------------------------------------*/
/*    E x t r a c t N a m e                                           */
/*                                                                    */
/*    Returns full name of user, and returns address if name          */
/*    is not available.                                               */
/*--------------------------------------------------------------------*/

void ExtractName(char *result, const char *input)
{
      static int recursion = 0;

      recursion++;

#ifdef UDEBUG
   printmsg(15, "ExtractName: Return address is %p", result );
#endif

      printmsg((recursion > 2) ? 1 : 8,
            "ExtractName: Getting name from '%s'", input);

      ExtractAddress(result, input, FULLNAMEONLY);   /* Get the full name     */

      if (!strlen(result))       /* Did we get the name?              */
      {                          /* No --> Get the e-mail address     */
         char addr[MAXADDR];
         char path[MAXADDR];
         char node[MAXADDR];
         char *fullname;

         ExtractAddress(addr, input, ADDRESSONLY);

         if ( !tokenizeAddress(addr, path, node, result))
         {
            printmsg(0,"%s: %s",
                     addr,
                     path );
            strcpy( result, "##invalid##" );
            return;
         }

         fullname = nicknameByAddr(node, result);

         if (fullname == NULL)
         {
            strcat(result, "@");
            strcat(result, node);
         }
         else
            strcpy(result, fullname);
      }

      printmsg((recursion > 2) ? 1: 8, "ExtractName: name is '%s'", result);

      recursion--;

}  /* ExtractName */

/*--------------------------------------------------------------------*/
/*    B u i l d A d d r e s s                                         */
/*                                                                    */
/*    Builds a standard address format, with nickname resolution as   */
/*    required.                                                       */
/*--------------------------------------------------------------------*/

void BuildAddress(char *result, const char *input)
{
   char addr[MAXADDR];
   char name[MAXADDR];
   char user[MAXADDR];
   char path[MAXADDR];
   char node[MAXADDR];
   char *fulladdr;

/*--------------------------------------------------------------------*/
/*   It must be a real address, possibly with a name attached; get    */
/*   the address portion, break the address into user and node, and   */
/*   then see if we know the person by address                        */
/*--------------------------------------------------------------------*/

      ExtractAddress(addr, input, ADDRESSONLY);
                                    /* Get user e-mail addr          */

      if ( ! tokenizeAddress(addr, path, node, user) )
      {
         printmsg(0,"%s: %s", addr, path );
         sprintf( "%s@%s", "##INVALID##", E_domain );
         return;
      }

      fulladdr = nicknameByAddr(node, user);  /* Alias for the address? */

      if (fulladdr != NULL)            /* Yes --> Use it             */
      {
         strcpy(result, fulladdr);
         return;
      } /* if */

/*--------------------------------------------------------------------*/
/*   We don't know the address yet; get the name the user provided,   */
/*   and then normalize the address                                   */
/*--------------------------------------------------------------------*/

      ExtractAddress(name, input, FULLNAMEONLY);
                                    /* Also get their name           */

      if (strlen(name))             /* Did we find a name for user?   */
      {                             /* Yes --> Return it              */
         char *s = strchr(node, '.');

         if ((s == NULL) || equalni( s, ".UUCP", 5))
                                    /* Simple name or UUCP domain?    */
         {                          /* Yes--> Use original address    */

            size_t pathlen = strlen(path);/* Save len of orig path    */

            if ((pathlen > strlen(addr)) &&
                (!equal(node, path)) && /* Target not a known host?    */
                equaln(addr, path, strlen(path)) && /* & host starts   */
                (addr[pathlen] == '!'))   /* ...the address?          */
               fulladdr = &addr[pathlen + 1];   /* Yes --> Drop it    */
            else
               fulladdr = addr;  /* No --> Use full address           */

            sprintf(result, "(%s) %s", name, addr);

         } /* (strchr(node, '.') == NULL) */
         else                    /* No --> Use RFC-822 format         */
            sprintf(result, "\"%s\" <%s@%s>", name, user, node);

      } /* if strlen(name) */
      else
         strcpy(result, addr);    /* No name, just use the original    */

} /* BuildAddress */

/*--------------------------------------------------------------------*/
/*    N i c k n a m e B y N i c k                                     */
/*                                                                    */
/*    Locate a mail address by search the nickname table.  Returns    */
/*    true if nickname found and has address, otherwise False.        */
/*--------------------------------------------------------------------*/

char *nicknameByNick(const char *nick)
{
   int   upper;
   int   lower;

   if (!nicknameCount)
      nicknameCount = loadAliases();

   upper = (int) nicknameCount - 1;
   lower = 0;

   while (upper >= lower)
   {
      int midpoint;
      int hit;

      midpoint = ( upper + lower ) / 2;
      hit = stricmp(nick, nickname[midpoint].anick);

/*--------------------------------------------------------------------*/
/*                   We found the nickname, return it                 */
/*--------------------------------------------------------------------*/

      if (!hit)
         return nickname[midpoint].afull;

/*--------------------------------------------------------------------*/
/*       Determine if we are high or low and reset the table          */
/*       limits as needed                                             */
/*--------------------------------------------------------------------*/

      if ( hit > 0 )
         lower = midpoint + 1;
      else
         upper = midpoint - 1;

   }

/*--------------------------------------------------------------------*/
/*                      Return failure to caller                      */
/*--------------------------------------------------------------------*/

   return NULL;

} /* nicknameByNick */

/*--------------------------------------------------------------------*/
/*    N i c k n a m e B y A d d r                                     */
/*                                                                    */
/*    Locate a mail address by search the nickname table.  Returns    */
/*    true if nickname found and has address, otherwise False         */
/*--------------------------------------------------------------------*/

char *nicknameByAddr(const char *node, const char *user)
{
   size_t current = 0;

   if (!nicknameCount)
      nicknameCount = loadAliases();

   while (current < nicknameCount)
   {
      int hit;

      hit = stricmp(node, nickname[current].anode);
      if (!hit)
      {

         hit = stricmp(user, nickname[current].auser);

         if (!hit)
            return nickname[current].afull;
      }

      current++;
   }

   return NULL;

} /* nicknameByAddr */

/*--------------------------------------------------------------------*/
/*    l o a d N i c k n a m e e s                                     */
/*                                                                    */
/*    Initializes the address nickname table; returns number of       */
/*    nicknames loaded                                                */
/*--------------------------------------------------------------------*/

size_t loadAliases(void)
{
   FILE *ff;
   char *token;
   size_t   elements = 0;
   size_t   max_elements = userElements + 20;
   size_t   subscript;
   NICKNAME target;
   char buf[BUFSIZ];

   checkuser( E_mailbox ); /* Force the table to be loaded            */
   nickname = calloc(max_elements, sizeof(*nickname));
   checkref(nickname);

/*--------------------------------------------------------------------*/
/*                   Actually load the nickname table                 */
/*--------------------------------------------------------------------*/

   if (E_nickname != NULL )   /* Did the user specify nicknames file?   */
   {
      char fname[FILENAME_MAX];

      strcpy( fname, E_nickname);

      expand_path( fname, E_homedir, E_homedir, NULL );

      E_nickname = newstr( fname );

      ff = FOPEN(fname, "r", TEXT_MODE);

      if (ff == NULL)
      {
         printerr(fname);
         return elements;
      } /* if */

      while (! feof(ff))
      {
         char node[MAXADDR];
         char user[MAXADDR];
         char addr[MAXADDR];
         char path[MAXADDR];
         char *eos;              /* End of string pointer         */

         size_t   quotes = 0;    /* Number of quotes in address   */
         char *left, *right;     /* Bracket pointers              */

         if (fgets(buf, BUFSIZ, ff) == NULL)   /* Try to read a line     */
            break;                  /* Exit if end of file             */

         token = strtok(buf, WHITESPACE );

         if (token == NULL)         /* Any data?                       */
            continue;               /* No --> read another line        */

         if (token[0] == '#')
            continue;                  /* Line is a comment; loop again */

         target.anick = token;

/*--------------------------------------------------------------------*/
/*            Expand the nickname table if we're out of room          */
/*--------------------------------------------------------------------*/

         if (elements == max_elements)
         {
             max_elements = max_elements * 2;
             nickname = realloc( nickname,
                                 max_elements * sizeof(*nickname));
             checkref(nickname);
         }

/*--------------------------------------------------------------------*/
/*       Get the information for the nickname: strip leading/         */
/*       trailing whitespace, and verify there was information        */
/*       provided.                                                    */
/*--------------------------------------------------------------------*/

         token = strtok(NULL, "");    /* Get rest of string         */

         while ( token && isspace(*token))
            token++;

         if ( !token || !strlen(token) )
         {
            printmsg(0, "No information provided for nickname %s in "
                        "%s, line ignored",
                        target.anick,
                        fname );
            continue;            /* Ignore rest of this line      */
         }

         eos = token + strlen(token) - 1;
         while ( isspace(*eos))
         {
            *eos = '\0';
            eos--;
         }

         target.afull = token;   /* Save location for later */

/*--------------------------------------------------------------------*/
/*         Verify the quotes are balanced if they exist at all.       */
/*--------------------------------------------------------------------*/

         token = target.afull;
         quotes = 0;

         while( (token = strchr( token, '"' )) != NULL )
         {
            quotes ++;
            token ++;            /* Step past quote               */
         }

         if ( quotes % 2 )
         {

            printmsg(0, "Unbalanced quotes in %s for nickname %s, "
                       "entry ignored: %s",
                       fname,
                       target.anick,
                       target.afull );
            continue;

         }

/*--------------------------------------------------------------------*/
/*       Also verify the general layout of angle brackets, another    */
/*       common format error.                                         */
/*--------------------------------------------------------------------*/

         left = strchr( target.afull, '<' );

         if ( (quotes > 0) && (left == NULL ) )
         {
            printmsg(0,
                     "No address for nickname %s in %s, "
                     "line ignored: %s",
                      target.anick,
                      fname,
                      target.afull );
            continue;
         }

         if ( left != NULL )
         {
            right = strchr( target.afull, '>' );
            if ( (right == NULL) || (right < left ) )
            {
               printmsg(0, "Invalid address for nickname %s in %s, "
                        "line ignored: %s",
                         target.anick,
                         fname,
                         target.afull );
               continue;
            }
         }

/*--------------------------------------------------------------------*/
/*     Extract the address components for lookups by host and userid  */
/*--------------------------------------------------------------------*/

         ExtractAddress(addr, target.afull, ADDRESSONLY );

         if ( !tokenizeAddress(addr, path, node, user) )
         {
            printmsg(0,"%s: %s (nickname %s ignored)",
                     addr,
                     path,
                     target.anick );
            continue;
         }

         nickname[elements].anick = newstr(target.anick);
         nickname[elements].afull = newstr(target.afull);
         nickname[elements].anode = newstr(node);
         nickname[elements].auser = newstr(user);

         elements ++;

      } /* while (! feof(ff)) */

      fclose(ff);

   } /* if (E_nickname != NULL ) */

/*--------------------------------------------------------------------*/
/*           Add the local users as final nicknames in table          */
/*--------------------------------------------------------------------*/

   nickname = realloc(nickname, (elements + userElements) * sizeof(*nickname));
                              /* Resize table to final known size     */
   checkref(nickname);

   for ( subscript = 0; subscript < userElements;  subscript++)
   {
      if ( equal(users[subscript].realname, EMPTY_GCOS) )
         continue;

      nickname[elements].anick = "";   /* No nickname, only good for addr */

      if (bflag[F_BANG])
         sprintf(buf, "(%s) %s!%s",
               users[subscript].realname, E_fdomain,
               users[subscript].uid);
      else
         sprintf(buf, "\"%s\" <%s@%s>", users[subscript].realname,
               users[subscript].uid, E_fdomain );
      nickname[elements].afull = newstr(buf);
      nickname[elements].anode = E_nodename;
      nickname[elements].auser = users[subscript].uid;

      elements++;

   } /* for */

/*--------------------------------------------------------------------*/
/*                         Now sort the table                         */
/*--------------------------------------------------------------------*/

   qsort(nickname, elements, sizeof(nickname[0]), nickCompare);

/*--------------------------------------------------------------------*/
/*                   Check for duplicate nicknames                    */
/*--------------------------------------------------------------------*/

   for ( subscript = elements - 1;
         (nickname[subscript].anick[0] != '\0') && (subscript > 0);
         subscript--)
   {
      if ( equal( nickname[subscript].anick,
                  nickname[subscript - 1].anick ))
         printmsg(0,"loadAliases: Duplicate nickname %s in %s, "
                    "use of nickname will be unpredictable!",
                    nickname[subscript].anick,
                    E_nickname );
   }

/*--------------------------------------------------------------------*/
/*                 Return number of entries to caller                 */
/*--------------------------------------------------------------------*/

   return elements;

} /* loadAliases */
