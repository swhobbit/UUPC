/*--------------------------------------------------------------------*/
/*       u s e r t a b l . c                                          */
/*                                                                    */
/*       User table routines for UUPC/extended                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: usertabl.c 1.17 1998/01/03 05:27:58 ahd Exp $
 *
 *    $Log: usertabl.c $
 *    Revision 1.17  1998/01/03 05:27:58  ahd
 *    Password file overrides environment variable
 *
 *    Revision 1.16  1995/12/26 16:55:43  ahd
 *    Don't scan table for every user
 *    Correct failure user table reallocation
 *
 *    Revision 1.15  1995/02/21 03:30:52  ahd
 *    More compiler warning cleanup, drop selected messages at compile
 *    time if not debugging.
 *
 *    Revision 1.14  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 */

#include "uupcmoah.h"

#include <assert.h>

#include <ctype.h>

#include "expath.h"
#include "usertabl.h"
#include "hostable.h"
#include "security.h"
#include "pushpop.h"

struct UserTable *users = NULL;  /* Public to allow alias.c to use it */

size_t  userElements = 0;        /* Public to allow alias.c to use it */

static size_t loaduser( void );

char *NextField( char *input );

static char uucpsh[] = UUCPSHELL;

currentfile();

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

#ifdef UDEBUG
   printmsg(14,"checkuser: Searching for user id %s", name);
#endif

 /*-------------------------------------------------------------------*/
 /*             Initialize the host name table if needed              */
 /*-------------------------------------------------------------------*/

   if (userElements == 0)           /* host table initialized yet?    */
      userElements = loaduser();        /* No --> load it             */

   lower = 0;
   upper = (int) userElements - 1;

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

   static size_t allocUsers = (BUFSIZ / sizeof *users);
                                    /* Number of users allocated */

   if (users == NULL)
   {
      users = calloc(allocUsers, sizeof(*users));
      checkref(users);
   }

/*--------------------------------------------------------------------*/
/*    Return active user point if adding password entry for current   */
/*    user and it has already been added.                             */
/*--------------------------------------------------------------------*/

   if ( userElements && equali( users[0].uid, name ))
      return &users[0];

/*--------------------------------------------------------------------*/
/*                Make the table bigger if we need it                 */
/*--------------------------------------------------------------------*/

   if ( userElements == allocUsers )
   {
      allocUsers *= 3;
      users = realloc(users, allocUsers * sizeof(*users));
      checkref( users );

#ifdef UDEBUG
      printmsg(4,"initUser: Reallocated user table to %d entries",
                  allocUsers );
#endif
   }

/*--------------------------------------------------------------------*/
/*                        Add the current user                        */
/*--------------------------------------------------------------------*/

   users[userElements].uid      = newstr(name);
   users[userElements].realname = EMPTY_GCOS;
   users[userElements].beep     = NULL;
   users[userElements].homedir  = E_pubdir;
   users[userElements].hsecure  = NULL;
   users[userElements].password = NULL;
   users[userElements].group    = UUCP_GROUP;
   users[userElements].sh       = uucpsh;

   return &users[userElements++];

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
   size_t subscript;
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
      if ( debuglevel > 2 )
         printerr( E_passwd );

      users = realloc(users, userElements *  sizeof(*users));

      checkref(users);
      return userElements;
   } /* if */

   PushDir( E_confdir );      /* Use standard reference point for     */
                              /* for directories                      */

/*--------------------------------------------------------------------*/
/*                 The password file is open; read it                 */
/*--------------------------------------------------------------------*/

   while (! feof(stream))
   {
      char buf[BUFSIZ];

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

      if ( userp->uid == NULL )
      {
         panic();
      }

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

      token = NextField(NULL);   /* UNIX group number                 */

      if (token != NULL)         /* Did they provide a group?         */
         userp->group = newstr(token); /* Yes --> Copy                */

      token = NextField(NULL);   /* Get the formal user name          */

      if (token != NULL)         /* Did they provide user name?       */
         userp->realname = newstr(token); /* Yes --> Copy             */

      token = NextField(NULL);   /* Get home directory (optional)     */

      if ( token != NULL)
      {
         userp->homedir = newstr(normalize( token ));
         if ( equal( userp->uid, E_mailbox ))
            E_homedir = userp->homedir;
      }

      token = NextField(NULL);   /* Get user shell (optional)         */

      if ( token != NULL )       /* Did we get it?                    */
         userp->sh = newstr(token); /* Yes --> Copy it in             */

   }  /* while */

   PopDir();

   fclose(stream);
   users = realloc(users, userElements *  sizeof(*users));
   checkref(users);

   qsort(users, userElements ,sizeof(users[0]) , usercmp);

   for (subscript = 0 ; subscript < userElements; subscript ++)
   {
      KWBoolean duplicate = KWFalse;

      if ( subscript && equali( users[subscript].uid,
                                users[subscript- 1 ].uid))
      {
         printmsg(0,"*error* The user id \"%s\" occurs more than once in %s!"
                    "  Delete extra entries!",
                     users[subscript-1].uid,
                     E_passwd );

         duplicate = KWTrue;
      }

      printmsg(duplicate ? 0: 8,
                 "loaduser: user[%d]\tlogin(%s)\tno(%s)\tgroup(%s)"
                 "\tname(%s)\thome(%s)\tshell(%s)",
         subscript,
         users[subscript].uid,
         users[subscript].beep == NULL ? "NONE" : users[subscript].beep,
         users[subscript].group,
         users[subscript].realname,
         users[subscript].homedir,
         users[subscript].sh);
   } /* for */

   return userElements;

} /* loaduser */

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
