/*--------------------------------------------------------------------*/
/*    a l i a s . c                                                   */
/*                                                                    */
/*    Smart routing and alias routines for UUPC/extend mail           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
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
 *    $Id: alias.c 1.11 1994/01/01 19:12:06 ahd Exp $
 *
 *    Revision history:
 *    $Log: alias.c $
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
#include "alias.h"
#include "address.h"
#include "expath.h"

static size_t AliasCount = 0;

static struct AliasTable *alias = NULL;

int nickcmp( const void *a, const void *b );

static size_t LoadAliases( void ) ;

currentfile();

/*--------------------------------------------------------------------*/
/*    I n i t R o u t e r                                             */
/*                                                                    */
/*    Verify, initialize the global routing data                      */
/*--------------------------------------------------------------------*/

boolean InitRouter()
{
   boolean success = TRUE;       /* Assume the input data is good      */
   struct HostTable *Hptr;

/*--------------------------------------------------------------------*/
/*          Verify that the user gave us a good name server           */
/*--------------------------------------------------------------------*/

   Hptr = checkreal(E_mailserv);
   if (Hptr == BADHOST)
   {
      printmsg(0,"mail server '%s' must be listed in SYSTEMS file",
         E_mailserv);
      success = FALSE;
   }
   else if (Hptr->hstatus == localhost)  /* local system?     */
   {
      printmsg(0,"'%s' is name of this host and cannot be mail server",
            E_mailserv);
      success = FALSE;
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

void ExtractName(char *result, char *column)
{
      static int recursion = 0;

      recursion++;

      printmsg((recursion > 2) ? 1:8,
            "ExtractName: Getting name from '%s'",column);

      ExtractAddress(result, column, TRUE);  /* Get the full name     */
      if (!strlen(result))       /* Did we get the name?              */
      {                          /* No --> Get the e-mail address     */
         char addr[MAXADDR];
         char path[MAXADDR];
         char node[MAXADDR];
         char *fullname;

         ExtractAddress(addr,column, FALSE);
         user_at_node(addr,path,node,result);
                                 /* Reduce address to basics */
         fullname = AliasByAddr(node,result);
         if (fullname == NULL)
         {
            strcat(result,"@");
            strcat(result,node);
         }
         else
            strcpy(result,fullname);
      }

      printmsg((recursion > 2) ? 1: 8,"ExtractName: name is '%s'",result);

      recursion--;

      return;
}  /*ExtractName*/

/*--------------------------------------------------------------------*/
/*    B u i l d A d d r e s s                                         */
/*                                                                    */
/*    Builds a standard address format, with aliasing as              */
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

      ExtractAddress(addr,input,FALSE);   /* Get user e-mail addr     */
      user_at_node(addr,path,node,user);  /* Break address down       */

      fulladdr = AliasByAddr(node,user);  /* Alias for the address?   */
      if (fulladdr != NULL)            /* Yes --> Use it              */
      {
         strcpy(result,fulladdr);
         return;
      } /* if */

/*--------------------------------------------------------------------*/
/*   We don't know the address yet; get the name the user provided,   */
/*   and then normalize the address                                   */
/*--------------------------------------------------------------------*/

      ExtractAddress(name,input,TRUE);    /* Also get their name      */

      if (strlen(name))             /* Did we find a name for user?   */
      {                             /* Yes --> Return it              */
         char *s = strchr(node, '.');
         if ((s == NULL) || equalni( s, ".UUCP", 5))
                                    /* Simple name or UUCP domain?    */
         {                          /* Yes--> Use original address    */
            size_t pathlen = strlen(path);/* Save len of orig path    */
            if ((pathlen > strlen(addr)) &&
                (!equal(node,path)) && /* Target not a known host?    */
                equaln(addr,path, strlen(path)) && /* & host starts   */
                (addr[pathlen] == '!'))   /* ...the address?          */
               fulladdr = &addr[pathlen + 1];   /* Yes --> Drop it    */
            else
               fulladdr = addr;  /* No --> Use full address           */
            sprintf(result,"(%s) %s", name, addr);
         } /* (strchr(node, '.') == NULL) */
         else                    /* No --> Use RFC-822 format         */
            sprintf(result,"\"%s\" <%s@%s>", name, user, node);
      } /* if strlen(name) */
      else
         strcpy(result,addr);    /* No name, just use the original    */
} /* BuildAddress */

/*--------------------------------------------------------------------*/
/*    A l i a s B y N i c k                                           */
/*                                                                    */
/*    Locate a mail address by search the alias table.  Returns TRUE  */
/*    if alias found and has address, otherwise FALSE.                */
/*--------------------------------------------------------------------*/

char *AliasByNick(const char *nick)
{
   int   upper;
   int   lower;

   if (!AliasCount)
      AliasCount = LoadAliases();

   upper = AliasCount - 1;
   lower = 0;

   while (upper >= lower)
   {
      int midpoint;
      int hit;

      midpoint = ( upper + lower ) / 2;
      hit = stricmp(nick,alias[midpoint].anick);
      if (!hit)
         return alias[midpoint].afull;
      if ( hit > 0 )
         lower = midpoint + 1;
      else
         upper = midpoint - 1;
   }
   return NULL;
}

/*--------------------------------------------------------------------*/
/*    A l i a s B y A d d r                                           */
/*                                                                    */
/*    Locate a mail address by search the alias table.  Returns TRUE  */
/*    if alias found and has address, otherwise FALSE                 */
/*--------------------------------------------------------------------*/

char *AliasByAddr(const char *node, const char *user)
{
   size_t current = 0;

   if (!AliasCount)
      AliasCount = LoadAliases();

   while (current < AliasCount)
   {
      int hit;

      hit = stricmp(node,alias[current].anode);
      if (!hit)
      {
         hit = stricmp(user,alias[current].auser);
         if (!hit)
            return alias[current].afull;
      }
      current++;
   }
   return NULL;

}

/*--------------------------------------------------------------------*/
/*    L o a d A l i a s e s                                           */
/*                                                                    */
/*    Initializes the address alias table; returns number of aliases  */
/*    loaded                                                          */
/*--------------------------------------------------------------------*/

size_t LoadAliases(void)
{
   FILE *ff;
   char buf[BUFSIZ];
   char *token;
   size_t   elements = 0;
   size_t   max_elements = UserElements + 20;
   size_t   subscript;
   struct AliasTable *hit;           /* temporary pointer for searching */
   struct AliasTable target;

   checkuser( E_mailbox ); /* Force the table to be loaded            */
   alias = calloc(max_elements, sizeof(*alias));
   checkref(alias);

/*--------------------------------------------------------------------*/
/*                   Actually load the alias table                    */
/*--------------------------------------------------------------------*/

   if (E_nickname != NULL )   /* Did the user specify aliases file?   */
   {
      char fname[FILENAME_MAX];

      strcpy( fname, E_nickname);
      expand_path( fname, E_homedir, E_homedir , NULL );
      ff = FOPEN(fname , "r",TEXT_MODE);

      if (ff == NULL)
      {
         printerr(fname);
         return elements;
      } /* if */

      while (! feof(ff))
      {
         if (fgets(buf,BUFSIZ,ff) == NULL)   /* Try to read a line     */
            break;                  /* Exit if end of file             */
         token = strtok(buf," \t\n");
         if (token == NULL)         /* Any data?                       */
            continue;               /* No --> read another line        */
         if (token[0] == '#')
            continue;                  /* Line is a comment; loop again */

         /* Add the alias to the table.  Note that we must add the nick */
         /* to the table ourselves (rather than use lsearch) because   */
         /* we must make a copy of the string; the *token we use for   */
         /* the search is in the middle of our I/O buffer!             */
         /*
         /* I was burned, _you_ have been warned.                      */

         target.anick = token;

         hit = (void *) lfind((void *) &target, (void *) alias,
			      &elements , sizeof(alias[0]), nickcmp);
         if (hit == NULL)
         {
            char node[MAXADDR];
            char user[MAXADDR];
            char path[MAXADDR];
            char addr[MAXADDR];
            char *eos;

            if (elements == max_elements)
            {
                max_elements = max_elements * 2;
                alias = realloc(alias, max_elements * sizeof(*alias));
                checkref(alias);
            }

            alias[elements].anick = newstr(token);
            token = strtok(NULL,"");    /* Get rest of string         */

            while ( strlen(token) && isspace(*token))
               token++;
            eos = token + strlen(token) - 1;
            while ( strlen(token) && isspace(*eos))
            {
               *eos = '\0';
               eos--;
            }

            alias[elements].afull = newstr(token);
            ExtractAddress(addr,alias[elements].afull,FALSE);
            user_at_node(addr,path,node,user);
            alias[elements].anode = newstr(node);
            alias[elements].auser = newstr(user);
            elements += 1;
         }
         else
            printmsg(0,"LoadAliases: Duplicate alias '%s' in table",token);
      }
      fclose(ff);
   } /* if (E_nickname != NULL ) */

/*--------------------------------------------------------------------*/
/*           Add the local users as final aliases in table            */
/*--------------------------------------------------------------------*/

   alias = realloc(alias, (elements + UserElements) * sizeof(*alias));
                              /* Resize table to final known size     */
   checkref(alias);

   for ( subscript = 0; subscript < UserElements;  subscript++)
   {
      if ( equal(users[subscript].realname,EMPTY_GCOS) )
         continue;

      alias[elements].anick = "";   /* No nickname, only good for addr */
      if (bflag[F_BANG])
         sprintf(buf, "(%s) %s!%s",
               users[subscript].realname, E_fdomain,
               users[subscript].uid);
      else
         sprintf(buf, "\"%s\" <%s@%s>", users[subscript].realname,
               users[subscript].uid, E_fdomain );
      alias[elements].afull = newstr(buf);
      alias[elements].anode = E_nodename;
      alias[elements].auser = users[subscript].uid;

      elements++;
   } /* for */

/*--------------------------------------------------------------------*/
/*                         Now sort the table                         */
/*--------------------------------------------------------------------*/

   qsort(alias, elements ,sizeof(alias[0]) , nickcmp);

   return (elements) ;
} /*LoadAliases*/

/*--------------------------------------------------------------------*/
/*   n i c k c m p                                                    */
/*                                                                    */
/*   Accepts indirect pointers to two strings and compares them using */
/*   stricmp (case insensitive string compare)                        */
/*--------------------------------------------------------------------*/

int nickcmp( const void *a, const void *b )
{
   return stricmp(((struct AliasTable *)a)->anick,
         ((struct AliasTable *)b)->anick);
}  /*nickcmp*/
