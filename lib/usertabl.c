/*--------------------------------------------------------------------*/
/*       u s e r t a b l . c                                          */
/*                                                                    */
/*       User table routines for UUPC/extended                        */
/*                                                                    */
/*       Copyright (C) 1989, 1990 by Andrew H. Derbyshire             */
/*                                                                    */
/*       See file README.SCR for restrictions on re-distribution.     */
/*                                                                    */
/*       History:                                                     */
/*          Create from hostable.c                                    */
/*--------------------------------------------------------------------*/

/*
 *    $Id: usertabl.c 1.12 1994/04/24 20:35:08 ahd Exp $
 *
 *    $Log: usertabl.c $
 *     Revision 1.12  1994/04/24  20:35:08  ahd
 *     Change case of userElement
 *
 *     Revision 1.11  1994/02/20  19:05:02  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.10  1993/11/06  16:56:13  ahd
 *     Correct reference to userid in password warning message
 *
 *     Revision 1.9  1993/11/06  13:04:13  ahd
 *     Trap NULL user passwords
 *
 *     Revision 1.8  1993/10/12  00:49:39  ahd
 *     Normalize comments
 *
 *     Revision 1.7  1993/10/02  19:07:49  ahd
 *     Drop unneeded checkref()
 *
 *     Revision 1.6  1993/05/29  15:19:59  ahd
 *     Allow configured systems, passwd files
 *
 *     Revision 1.5  1993/05/06  03:41:48  ahd
 *     Use expand_path to get reasonable correct drive for aliases file
 *
 *     Revision 1.4  1993/04/15  03:17:21  ahd
 *     Use standard define for undefined user names
 *
 *     Revision 1.3  1993/04/11  00:31:04  ahd
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/22  20:58:55  ahd
 * Use strpool to allocate const strings
 * Normalize directories as read
 *
 */

#include "uupcmoah.h"

#include <assert.h>

#include <ctype.h>

#include "expath.h"
#include "usertabl.h"
#include "hostable.h"
#include "security.h"
#include "pushpop.h"

struct UserTable *users = NULL;  /* Public to allow router.c to use it */

size_t  userElements = 0;        /* Public to allow router.c to use it */

static size_t loaduser( void );

static int usercmp( const void *a , const void *b );

char *NextField( char *input );

static char uucpsh[] = UUCPSHELL;

currentfile();

/*--------------------------------------------------------------------*/
/*    c h e c k u s e r                                               */
/*                                                                    */
/*    Look up a user name in the PASSWD file                          */
/*--------------------------------------------------------------------*/

struct UserTable *checkuser(const char *name)
{
   int   lower;
   int   upper;

   if ( (name == NULL) || (strlen(name) == 0) )
   {
      printmsg(0,"checkuser: Invalid argument!");
      panic();
   }

   printmsg(14,"checkuser: Searching for user id %s",name);

 /*-------------------------------------------------------------------*/
 /*             Initialize the host name table if needed              */
 /*-------------------------------------------------------------------*/

   if (userElements == 0)           /* host table initialized yet?    */
      userElements = loaduser();        /* No --> load it             */

   lower = 0;
   upper = userElements - 1;

/*--------------------------------------------------------------------*/
/*              Peform a binary search on the user table              */
/*--------------------------------------------------------------------*/

   while ( lower <= upper )
   {
      int midpoint;
      int hit;
      midpoint = (lower + upper) / 2;

      hit = stricmp(name,users[midpoint].uid);

      if (hit > 0)
         lower = midpoint + 1;
      else if (hit < 0)
         upper = midpoint - 1;
      else
         return &users[midpoint];
   }

/*--------------------------------------------------------------------*/
/*         We didn't find the user.  Return failure to caller         */
/*--------------------------------------------------------------------*/

   return BADUSER;

}  /* checkuser */

/*--------------------------------------------------------------------*/
/*    i n i t u s e r                                                 */
/*                                                                    */
/*    Intializes a user table entry for for loaduser                  */
/*--------------------------------------------------------------------*/

struct UserTable *inituser(char *name)
{

   static int allocUsers = 100;    /* Number of users allocated */
   size_t hit = userElements;
   size_t element = 0;

   if (users == NULL)
   {
      users = calloc(allocUsers, sizeof(*users));
      checkref(users);
   }

/*--------------------------------------------------------------------*/
/*    Add the user to the table.  Note that we must add the user      */
/*    to the table ourselves (rather than use lsearch) because we     */
/*    must make a copy of the string; the *token we use for the       */
/*    search is in the middle of our I/O buffer!                      */
/*--------------------------------------------------------------------*/

   while ( element < hit )
   {
      if (equali( users[element].uid , name ))
         hit = element;
      else
         element++;
   }

   if (hit == userElements)
   {
      if ( (hit-1) == allocUsers)
      {
         allocUsers *= 4;
         users = realloc(users, allocUsers * sizeof(*users));
         checkref( users );
      }

      users[hit].uid      = newstr(name);
      users[hit].realname = EMPTY_GCOS;
      users[hit].beep     = NULL;
      users[hit].homedir  = E_pubdir;
      users[hit].hsecure  = NULL;
      users[hit].password = NULL;
      users[hit].sh       = uucpsh;
      userElements++;

   } /* if */

   return &users[hit];

} /* inituser */

/*--------------------------------------------------------------------*/
/*    l o a d u s e r                                                 */
/*                                                                    */
/*    Load the user password file                                     */
/*--------------------------------------------------------------------*/

static size_t loaduser( void )
{
   FILE *stream;
   struct UserTable *userp;
   size_t hit;
   char buf[BUFSIZ];
   char *token;

/*--------------------------------------------------------------------*/
/*     First, load in the active user as first user in the table      */
/*--------------------------------------------------------------------*/

   userp = inituser( E_mailbox );
   userp->realname = E_name;
   userp->homedir  = E_homedir;

/*--------------------------------------------------------------------*/
/*       Password file format:                                        */
/*          user id:password:::user/system name:homedir:shell         */
/*--------------------------------------------------------------------*/

   if ((stream = FOPEN(E_passwd, "r",TEXT_MODE)) == NULL)
   {
      printmsg(2,"loaduser: Cannot open password file %s",E_passwd);
      users = realloc(users, userElements *  sizeof(*users));
      checkref(users);
      return userElements;
   } /* if */

   PushDir( E_confdir );      /* Use standard reference point for     */
                              /* for directories                      */

/*--------------------------------------------------------------------*/
/*                 The password file is open; read it                 */
/*--------------------------------------------------------------------*/

   while (! feof(stream)) {
      if (fgets(buf,BUFSIZ,stream) == NULL)   /* Try to read a line   */
         break;               /* Exit if end of file                  */
      if ((*buf == '#') || (*buf == '\0'))
         continue;            /* Line is a comment; loop again        */
      if ( buf[ strlen(buf) - 1 ] == '\n')
         buf[ strlen(buf) - 1 ] = '\0';
      token = NextField(buf);
      if (token    == NULL)   /* Any data?                            */
         continue;            /* No --> read another line             */
      userp = inituser(token);/* Initialize record for user           */

      if (userp->password != NULL)  /* Does the user already exist?   */
      {                       /* Yes --> Report and ignore            */
         printmsg(0,"loaduser: Duplicate entry for '%s' in '%s' ignored",
               token,E_passwd);
         continue;            /* System already in /etc/passwd,
                                 ignore it.                           */
      }

/*--------------------------------------------------------------------*/
/*       Password fields are funny; if the tokenize field function    */
/*       is returns NULL, we set the password to the comparable       */
/*       empty string ("").  But if the password is if asterisk       */
/*       (*), we leave the password NULL, and the user can never      */
/*       login remotely.                                              */
/*--------------------------------------------------------------------*/

      token = NextField(NULL);   /* Get the user password             */

      if ( token == NULL )       /* No password needed for login?     */
      {
         printmsg(2,"loaduser: WARNING: No password assigned for user %s",
                     userp->uid );
         userp->password = "";   /* Assign requested password        */
      }
      else if (!equal(token,"*")) /* User can login with passwd?      */
         userp->password = newstr(token); /* Yes --> Set password     */

      token = NextField(NULL);   /* Use  UNIX user number as tone     */
                                 /* to beep at                        */
      if (token != NULL)
         userp->beep = newstr( token );

      token = NextField(NULL);   /* Skip UNIX group number            */

      token = NextField(NULL);   /* Get the formal user name          */
      if (token != NULL)         /* Did they provide user name?       */
         userp->realname = newstr(token); /* Yes --> Copy             */

      token = NextField(NULL);   /* Get home directory (optional)     */
      if ( token != NULL)
         userp->homedir = newstr(normalize( token ));

      token = NextField(NULL);   /* Get user shell (optional)         */
      if ( token != NULL )       /* Did we get it?                    */
         userp->sh = newstr(token); /* Yes --> Copy it in             */

   }  /* while */

   PopDir();

   fclose(stream);
   users = realloc(users, userElements *  sizeof(*users));
   checkref(users);

   qsort(users, userElements ,sizeof(users[0]) , usercmp);

   for (hit = 0 ; hit < userElements; hit ++)
   {
      printmsg(8,"loaduser: user[%d] user id(%s) no(%s) name(%s) "
                 "home directory(%s) shell(%s)",
         hit,
         users[hit].uid,
         users[hit].beep == NULL ? "NONE" : users[hit].beep,
         users[hit].realname,
         users[hit].homedir,
         users[hit].sh);
   } /* for */

   return userElements;
} /* loaduser */

/*--------------------------------------------------------------------*/
/*    u s e r c m p                                                   */
/*                                                                    */
/*    Accepts indirect pointers to two strings and compares           */
/*    them using stricmp (case insensitive string compare)            */
/*--------------------------------------------------------------------*/

int usercmp( const void *a , const void *b )
{
   return stricmp(((struct UserTable*) a)->uid,
        ((struct UserTable*) b)->uid);
}  /*usercmp*/

/*--------------------------------------------------------------------*/
/*    n e x t f i e l d                                               */
/*                                                                    */
/*    Find the next field in the user password file                   */
/*--------------------------------------------------------------------*/

char *NextField( char *input )
{
   static char *start = NULL;
   char *next = NULL;

   if (input == NULL)         /* Starting a new field?                */
   {
      if ( start  == NULL )   /* Anything left to parse?              */
         return NULL;         /* No --> Return empty string           */
      else
         input = start;       /* Yes --> Continue parse of old one    */
   } /* if */

/*--------------------------------------------------------------------*/
/*    Look for the next field; because MS-DOS directories may have    */
/*    a sequence of x:/ or x:\ where 'x' is a drive letter, we take   */
/*    special care to allow DOS directories to appear unmolested      */
/*    in the password file                                            */
/*--------------------------------------------------------------------*/

   if ((strlen(input) > 2) && isalpha(input[0]) && (input[1] == ':') &&
       ((input[2] == '/') || (input[2] == '\\')))
      next = strchr( &input[2], ':');   /* Find start of next field   */
   else
      next = strchr( input, ':');   /* Find start of next field       */

   if (next == NULL )         /* Last field?                          */
      start = NULL;           /* Yes --> Make next call return NULL   */
   else {                     /* No                                   */
      *next = '\0';           /* Terminate the string                 */
      start = ++next;         /* Have next call look at next field    */
   } /* else */

   if (strlen(input))         /* Did we get anything in the field?    */
      return input;           /* Yes --> Return the string            */
   else
      return NULL;            /* Field is empty, return NULL          */

} /* NextField */
