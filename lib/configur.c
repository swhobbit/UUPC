/*--------------------------------------------------------------------*/
/*    c o n f i g u r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1994 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: configur.c 1.41 1994/02/19 04:40:04 ahd Exp $
 *
 *    Revision history:
 *    $Log: configur.c $
 *     Revision 1.41  1994/02/19  04:40:04  ahd
 *     Use standard first header
 *
 *     Revision 1.40  1994/02/19  04:04:54  ahd
 *     Use standard first header
 *
 *     Revision 1.39  1994/02/19  03:48:35  ahd
 *     Use standard first header
 *
 *     Revision 1.38  1994/02/18  23:08:10  ahd
 *     Use standard first header
 *
 *     Revision 1.37  1994/01/01  19:00:55  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.36  1993/12/29  02:46:47  ahd
 *     Add Vmail queuing support
 *
 *     Revision 1.35  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.34  1993/12/02  02:25:12  ahd
 *     Add max generated UUXQT command line length
 *
 *     Revision 1.33  1993/11/30  04:18:14  ahd
 *     Correct spelling error in message
 *
 *     Revision 1.32  1993/11/21  02:47:07  ahd
 *     Allow UUPCUSRRC to be set from environment variables
 *
 *     Revision 1.31  1993/11/14  20:51:37  ahd
 *     Add showspool
 *
 *     Revision 1.30  1993/11/13  17:37:02  ahd
 *     Add new options supporting mail formats and new windows under OS/2
 *
 *     Revision 1.29  1993/10/31  15:51:11  ahd
 *     Allow configuring permissions file name
 *     Restore period in names before printing error messages
 *
 *     Revision 1.28  1993/10/30  22:27:57  rommel
 *     News history support
 *
 *     Revision 1.27  1993/10/26  12:46:10  ahd
 *     Add include for setstdin.h
 *
 *     Revision 1.26  1993/10/25  02:39:44  ahd
 *     Delete OBSOLETE keyword from aliases, it breaks it
 *
 *     Revision 1.25  1993/10/25  01:21:22  ahd
 *     Rename Aliases to Nickname to make more unique for end users; allow
 *     Aliases as obsolete alias for now.
 *
 *     Revision 1.24  1993/10/12  00:47:04  ahd
 *     Normalize comments
 *
 *     Revision 1.23  1993/10/09  15:47:16  ahd
 *     Add run time IsDOS query function
 *
 *     Revision 1.22  1993/09/29  23:29:56  ahd
 *     Add xqtrootdir for UUXQT
 *
 *     Revision 1.21  1993/09/29  04:49:20  ahd
 *     Move priority variables to modem file
 *
 *     Revision 1.20  1993/09/24  03:43:27  ahd
 *     Use positive defaults for unsigned priority values
 *
 *     Revision 1.19  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.18  1993/07/31  16:22:16  ahd
 *     Changes in support of Robert Denny's Windows 3.x support
 *
 *     Revision 1.17  1993/07/22  23:19:50  ahd
 *     First pass for Robert Denny's Windows 3.x support changes
 *
 *     Revision 1.16  1993/07/05  14:45:29  ahd
 *     Correct message-of-the-day variable name
 *     Correct WIN32 prefix support
 *
 *     Revision 1.15  1993/06/16  04:03:25  ahd
 *     Special case root directories for UUPC/extended variable default
 *
 *     Revision 1.14  1993/06/15  12:18:06  ahd
 *     Saved changed directory name for debugging
 *
 *     Revision 1.13  1993/05/30  15:25:50  ahd
 *     Multiple driver support
 *
 *     Revision 1.12  1993/05/29  15:19:59  ahd
 *     Add systems file, passwd files
 *
 *     Revision 1.11  1993/05/09  03:49:21  ahd
 *     Support banner, motd strings
 *     Support longname, honordebug, senddebug options
 *
 *     Revision 1.10  1993/04/15  03:17:21  ahd
 *     Add bounce system option
 *
 *     Revision 1.9  1993/04/11  00:31:31  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.8  1993/04/05  04:32:19  ahd
 *     Set timezone, windows input mode in common routine
 *
 *     Revision 1.7  1993/04/04  04:57:01  ahd
 *     Default configuration directory from UUPCSYSRC
 *     Default system directories from Configuration directory
 *
 *     Revision 1.6  1993/03/06  22:48:23  ahd
 *     Don't fall off end of shorter tables
 *
 *     Revision 1.5  1993/01/23  19:08:09  ahd
 *     Add Windows/NT to allowed environments
 *
 * Revision 1.4  1992/12/01  04:37:03  ahd
 * Add SpeedOverMemory
 *
 * Revision 1.3  1992/11/22  20:58:55  ahd
 * Normalize directories as read
 * Use strpool to allocate const strings
 *
 * Revision 1.2  1992/11/19  02:56:47  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

#include "uupcmoah.h"

#include <ctype.h>

#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "pushpop.h"

#ifdef WIN32
#include "setstdin.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

boolean bflag[F_LAST];        /* Initialized to zero by compiler      */

char **E_internal = NULL;
char *E_nickname = NULL;
char *E_altsignature = NULL;
char *E_anonymous = NULL;
char *E_archivedir = NULL;
char *E_backup = NULL;
char *E_banner = NULL;
char *E_charset = NULL;
char *E_passwd  = NULL;
char *E_systems = NULL;
char *E_confdir = NULL;
char *E_domain = NULL;
char *E_editor = NULL;
char *E_fdomain = NULL;
char *E_filesent = NULL;
char *E_homedir = NULL;
char *E_inmodem = NULL;
char *E_localdomain = NULL;
char *E_mailbox = NULL;
char *E_maildir = NULL;
char *E_mailext = NULL;
char *E_mailserv = NULL;
char *E_motd = NULL;
char *E_name = NULL;
char *E_newsdir = NULL;
char *E_newsserv = NULL;
char *E_nodename = NULL;
char *E_organization = NULL;
char *E_pager = NULL;
char *E_permissions = NULL;
char *E_postmaster = NULL;
char *E_pubdir = NULL;
char *E_replyto = NULL;
char *E_signature = NULL;
char *E_spooldir = NULL;
char *E_tempdir = NULL;
char *E_uncompress = NULL;
char *E_uuxqtpath = NULL;
char *E_version = NULL;
char *E_cwd = NULL;
char *E_xqtRootDir = NULL;
char *E_vmsQueueDir = NULL;
char *E_vmail = NULL;
KEWSHORT E_maxhops = 20;                                    /* ahd */
KEWSHORT E_maxuuxqt = 0;      /* Max length of command line for remote */
static char *dummy = NULL;
static char *E_tz = NULL;

/*--------------------------------------------------------------------*/
/*                       Local emumerated types                       */
/*--------------------------------------------------------------------*/

typedef enum {
      ENV_UNKNOWN    = 0x0001,
      ENV_DOS        = 0x0002,
      ENV_BIT32      = 0x0004,
      ENV_BIT16      = 0x0008,
      ENV_OS2        = 0x0010,
      ENV_OS2_32BIT  = 0x0020,
      ENV_OS2_16BIT  = 0x0040,
      ENV_WIN        = 0x0080,
      ENV_WIN_32BIT  = 0x0100,
      ENV_WIN_16BIT  = 0x0200
      } ENV_TYPE;

#ifdef WIN32
static ENV_TYPE active_env = ENV_WIN_32BIT | ENV_WIN | ENV_BIT32;
#elif defined(_Windows)
static ENV_TYPE active_env = ENV_WIN_16BIT | ENV_WIN | ENV_BIT16;
#elif defined(__OS2__)
static ENV_TYPE active_env = ENV_OS2_32BIT | ENV_OS2 | ENV_BIT32;
#else
static ENV_TYPE active_env = ENV_DOS | ENV_BIT16;
#endif

static boolean getrcnames(char **sysp,char **usrp);

/*--------------------------------------------------------------------*/
/*  The following table controls the configuration files processing   */
/*--------------------------------------------------------------------*/

static CONFIGTABLE envtable[] = {
   {"aliases",      &E_nickname,     B_TOKEN|B_MUA},
   {"altsignature", &E_altsignature, B_TOKEN|B_MUA},
   {"anonymouslogin", &E_anonymous,  B_GLOBAL|B_TOKEN|(B_ALL & ~ B_MAIL)},
   {"archivedir",   &E_archivedir,   B_GLOBAL|B_PATH|B_ALL},
   {"backupext",    &E_backup,       B_TOKEN|B_MUA},
   {"banner",       &E_banner,       B_GLOBAL|B_PATH|B_UUCICO},
   {"charset",      &E_charset,      B_TOKEN|B_GLOBAL|B_SPOOL},
   {"confdir",      &E_confdir,      B_GLOBAL|B_PATH|B_ALL},
   {"domain",       &E_domain,       B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"editor",       &E_editor,       B_STRING|B_MUA|B_NEWS},
   {"filesent",     &E_filesent,     B_TOKEN|B_MUA|B_NEWS},
   {"folders",      &dummy,          B_PATH|B_MUSH },
   {"fromdomain",   &E_fdomain,      B_GLOBAL|B_ALL|B_TOKEN},
   {"home",         &E_homedir,      B_PATH|B_REQUIRED|B_ALL},
   {"inmodem",      &E_inmodem,      B_GLOBAL|B_TOKEN|B_UUCICO},
   {"internalcommands", (char **)   &E_internal, B_GLOBAL|B_LIST|B_ALL},
   {"localdomain",  &E_localdomain,  B_GLOBAL|B_TOKEN|B_MAIL},
   {"mailbox",      &E_mailbox,      B_REQUIRED|B_TOKEN|B_ALL},
   {"maildir",      &E_maildir,      B_GLOBAL|B_PATH|B_ALL},
   {"mailext",      &E_mailext,      B_TOKEN|B_MAIL},
   {"mailserv",     &E_mailserv,     B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"maximumhops",  (char **) &E_maxhops, B_MTA | B_SHORT | B_GLOBAL},
   {"maximumuuxqt", (char **) &E_maxuuxqt, B_MTA | B_SHORT | B_GLOBAL},
   {"motd",         &E_motd,         B_GLOBAL|B_PATH|B_UUCICO},
   {"mushdir",      &dummy,          B_GLOBAL|B_PATH|B_MUSH},
   {"name",         &E_name,         B_REQUIRED|B_MAIL|B_NEWS|B_STRING},
   {"nickname",     &E_nickname,     B_TOKEN|B_MUA},
   {"newsdir",      &E_newsdir,      B_GLOBAL|B_PATH|B_ALL},
   {"newsserv",     &E_newsserv,     B_GLOBAL|B_TOKEN|B_NEWS},
   {"nodename",     &E_nodename,     B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"options",      (char **) bflag, B_ALL|B_BOOLEAN},
   {"organization", &E_organization, B_STRING|B_MAIL|B_NEWS},
   {"pager",        &E_pager,        B_STRING|B_MUA|B_NEWS},
   {"path",         &E_uuxqtpath,    B_STRING|B_UUXQT|B_GLOBAL},
   {"permissions",  &E_permissions,  B_GLOBAL|B_PATH|B_ALL},
   {"postmaster",   &E_postmaster,   B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"priority",     &dummy,          B_OBSOLETE },
   {"passwd",       &E_passwd,       B_GLOBAL|B_PATH|B_ALL},
   {"prioritydelta",&dummy,          B_OBSOLETE },
   {"pubdir",       &E_pubdir,       B_GLOBAL|B_PATH|B_ALL},
   {"replyto",      &E_replyto,      B_TOKEN|B_MAIL|B_NEWS},
   {"rmail",        &dummy,          B_OBSOLETE },
   {"rnews",        &dummy,          B_OBSOLETE },
   {"signature",    &E_signature,    B_TOKEN|B_MUA|B_NEWS},
   {"spooldir",     &E_spooldir,     B_GLOBAL|B_PATH|B_ALL},
   {"systems",      &E_systems,      B_GLOBAL|B_PATH|B_ALL},
   {"tempdir",      &E_tempdir,      B_GLOBAL|B_PATH|B_ALL},
   {"tz",           &E_tz,           B_TOKEN|B_ALL},
   {"uncompress",   &E_uncompress,   B_GLOBAL|B_STRING|B_NEWS },
   {"version",      &E_version,      B_TOKEN|B_INSTALL},
   {"vmail",        &E_vmail,        B_MTA|B_PATH|B_ALL},
   {"vmsqueuedir",  &E_vmsQueueDir,  B_MTA|B_PATH|B_ALL},
   {"xqtrootdir",   &E_xqtRootDir,   B_UUXQT|B_PATH|B_ALL},
   { nil(char) }
}; /* table */

/*--------------------------------------------------------------------*/
/*               Boolean options shared by all programs               */
/*--------------------------------------------------------------------*/

FLAGTABLE configFlags[] = {
 { "askcc",       F_ASKCC,       B_LOCAL},
 { "autoedit",    F_AUTOEDIT,    B_LOCAL},
 { "autoinclude", F_AUTOINCLUDE, B_LOCAL},
 { "autoprint",   F_AUTOPRINT,   B_LOCAL},
 { "autosign",    F_AUTOSIGN,    B_LOCAL},
 { "backup",      F_BACKUP,      B_LOCAL},
 { "doskey",      F_DOSKEY,      B_LOCAL},
 { "dot",         F_DOT,         B_LOCAL},
 { "expert",      F_EXPERT,      B_LOCAL},
 { "forwardsave", F_SAVERESENT,  B_LOCAL},
 { "fromsep",     F_FROMSEP,     B_LOCAL},
 { "neweditorwindow",
                  F_NEWEDITORSESSION,
                                 B_LOCAL},
 { "newpagerwindow",
                  F_NEWPAGERSESSION,
                                 B_LOCAL},
 { "pager",       F_PAGER,       B_LOCAL},
 { "purge",       F_PURGE,       B_LOCAL},
 { "save",        F_SAVE,        B_LOCAL},
 { "suppresscopyright",
                  F_SUPPRESSCOPYRIGHT,
                                 B_LOCAL},
 { "speedovermemory",
                  F_SPEEDOVERMEMORY,
                                 B_LOCAL},
 { "undelete",    F_UNDELETE,    B_LOCAL},
 { "verbose",     F_VERBOSE,     B_LOCAL},
 { "windows",     F_WINDOWS,     B_LOCAL},

 { "bang",        F_BANG,        B_GLOBAL},
 { "bounce",      F_BOUNCE,      B_GLOBAL},
 { "collect",     F_COLLECTSTATS,B_GLOBAL},
 { "directory",   F_DIRECT,      B_GLOBAL},
 { "escape",      F_ESCAPE,      B_GLOBAL},
 { "history",     F_HISTORY,     B_GLOBAL},
 { "honordebug",  F_HONORDEBUG,  B_GLOBAL},
 { "kanji",       F_KANJI,       B_GLOBAL},
 { "longname",    B_LONGNAME,    B_GLOBAL},
 { "monocase",    F_ONECASE,     B_GLOBAL},
 { "multiqueue",  F_MULTI,       B_GLOBAL},
 { "multitask",   F_MULTITASK,   B_GLOBAL},
 { "senddebug",   F_SENDDEBUG,   B_GLOBAL},
 { "shortfrom",   F_SHORTFROM,   B_GLOBAL},
 { "showspool",   F_SHOWSPOOL,   B_GLOBAL},
 { "snews",       F_SNEWS,       B_GLOBAL},
 { "suppressfrom",F_SUPPRESSFROM,B_GLOBAL},
 { "syslog",      F_SYSLOG,      B_GLOBAL},
 { "symmetricgrades", F_SYMMETRICGRADES, B_GLOBAL},
 { "uupcnewsserv", F_UUPCNEWSSERV, B_GLOBAL},
 { nil(char) }
}           ;

/*--------------------------------------------------------------------*/
/*    p r o c e s s c o n f i g                                       */
/*                                                                    */
/*    Handle a single line of a configuration file                    */
/*--------------------------------------------------------------------*/

boolean processconfig(char *buff,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  FLAGTABLE *btable)
{
   CONFIGTABLE *tptr;
   char *cp;
   char *period;
   char *keyword;
   ENV_TYPE target_env;

/*--------------------------------------------------------------------*/
/*                break out the keyword from its value                */
/*--------------------------------------------------------------------*/

   if ((cp = strchr(buff, '=')) == nil(char))
   {
      printmsg(0,"Missing equals sign after keyword \"%s\", ignored",
                  buff);
      return TRUE;
   }
   *cp++ = '\0';
   strlwr(buff);

/*--------------------------------------------------------------------*/
/*    Determine if the keyword should processed in this environment   */
/*--------------------------------------------------------------------*/

   period = keyword = strchr( buff, '.' );
                              /* Look for environment prefix          */

   if ( keyword == NULL )     /* No environment?                      */
   {
      keyword = buff;         /* Then buffer starts with keyword      */
      target_env = active_env;
   }
   else {

      typedef struct _ENVLIST {
            char *name;
            int value;
      } ENVLIST;

      static ENVLIST envtable[] = {
         { "dos",      ENV_DOS      },
         { "16bit",    ENV_BIT16    },
         { "32bit",    ENV_BIT32    },
         { "32bitos2", ENV_OS2_16BIT},
         { "16bitos2", ENV_OS2_32BIT},
         { "os2",      ENV_OS2      },
         { "win32",    ENV_WIN_32BIT},
         { "win16",    ENV_WIN_16BIT},
         { "32bitwin", ENV_WIN_32BIT},
         { "16bitwin", ENV_WIN_16BIT},
         { "win",      ENV_WIN      },
         { NULL,       ENV_UNKNOWN  }
       };

      short subscript = 0;

      *keyword++ = '\0';      /* Terminate environment string         */
      target_env = ENV_UNKNOWN;

      while( envtable[subscript].name != NULL)
      {
         if (equal( envtable[subscript].name, buff ))
         {
            target_env = envtable[subscript].value;
            break;
         }
         else
            subscript ++;
      } /* while */

      if ( target_env == ENV_UNKNOWN )
      {
         printmsg(0,"Unknown environment \"%s\", keyword \"%s\" ignored",
               buff, keyword );
         return FALSE;
      }

   } /* else */

/*--------------------------------------------------------------------*/
/*                    Scan the table for its value                    */
/*--------------------------------------------------------------------*/

   for (tptr = table; tptr->sym != nil(char); tptr++)
   {
      boolean error = FALSE;
      if (equal(keyword, tptr->sym)) {
/*--------------------------------------------------------------------*/
/*            Skip the keyword because of the environment?            */
/*--------------------------------------------------------------------*/
        if (!(active_env & target_env) )
            printmsg(2,"%s-environment keyword \"%s\" skipped.",
                        strupr(buff), keyword);
/*--------------------------------------------------------------------*/
/*                      Handle obsolete options                       */
/*--------------------------------------------------------------------*/
        else if (tptr->bits & B_OBSOLETE)
            printmsg(2,"Obsolete keyword \"%s\" ignored.", keyword);
/*--------------------------------------------------------------------*/
/*                  Handle mis-placed system options                  */
/*--------------------------------------------------------------------*/
        else if ((tptr->bits & B_GLOBAL) && (sysmode != SYSTEM_CONFIG))
            printmsg(0,
               "User specified system keyword \"%s\" ignored.",
               keyword);
/*--------------------------------------------------------------------*/
/*                       Handle Boolean options                       */
/*--------------------------------------------------------------------*/
         else {
            if (tptr->bits & B_BOOLEAN)
               options(cp, sysmode, btable, (boolean *) tptr->loc);
/*--------------------------------------------------------------------*/
/*                       Handle integer values                        */
/*--------------------------------------------------------------------*/
            else if (tptr->bits & (B_SHORT|B_LONG))
            {
               long foo;
               cp = strtok(cp,WHITESPACE);
               if ( equal(cp,"0"))
                  foo = 0;
               else {
                  foo = atol(cp);

                  if ( foo == 0)
                  {
                     printmsg(0,
                        "Unable to convert \"%s\" value \"%s\" to integer",
                        keyword, cp);
                     error = TRUE;
                  } /* if */
               } /* else */

               if (tptr->bits & B_LONG)
                  *((long *) tptr->loc) = foo;
               else
                  *((KEWSHORT *) tptr->loc) = (KEWSHORT) foo;
            } /* else */
/*--------------------------------------------------------------------*/
/*                       Handle lists of tokens                       */
/*--------------------------------------------------------------------*/
            else if ((tptr->bits & program) && (tptr->bits & (B_LIST | B_CLIST)))
            {
               char **list = malloc( (MAXLIST+1) * sizeof (*list));
               char *colon;
               int words;

               checkref( list );

               if (tptr->bits & B_CLIST)  /* Use colon as delimiter?  */
                  while ( (colon = strchr( cp , ':')) != NULL)
                     *colon = ' ';     /* Make colons spaces ...      */

               words = getargs(cp, list);
               if( words > MAXLIST)
                  panic();

               if (words > 0)
               {
                  if ( *(tptr->loc) )
                     free( *(tptr->loc) );
                  list = realloc( list, (words+1) * sizeof(*list));
                  checkref( list );
                  *(tptr->loc) = (char *) list;
                  list[words] = NULL;

                  while( *list != NULL)
                  {
                     if (strlen(*list))
                     {
                        *list = newstr(*list);
                        checkref( *list++ );
                     }
                     else
                        *list++ = "";
                  } /* while */
               } /* if (words > 0) */
               else {
                  printmsg(0,"No parameters given for keyword \"%s\"",
                           keyword);
                  error = TRUE;
               } /* else */
            } /* else if */
/*--------------------------------------------------------------------*/
/*                  Handle single tokens and strings                  */
/*--------------------------------------------------------------------*/
            else if (tptr->bits & program)
            {
               while( *cp == ' ' )     /* Trim leading whitespace     */
                  cp++;

               if (*cp == '\0')
               {
                  error = TRUE;
                  printmsg(0,"No parameter given for keyword \"%s\""
                           ", ignored.",
                           keyword);
               } /* if */

               if (tptr->bits & B_TOKEN)  /* One word value?      */
                  cp = strtok(cp,WHITESPACE); /* Yes --> Tokenize */

               if (tptr->bits & B_NORMAL)  /* Normalize path?     */
                  cp = normalize( cp );

               if (tptr->bits & B_MALLOC)  /* Allocate normally?  */
               {
                  *(tptr->loc) = strdup(cp); /* Save string           */
                  checkref( *(tptr->loc) );  /* Verify malloc()       */
               }
               else
                  *(tptr->loc) = newstr(cp); /* Save string           */

            } /* else */
         } /* else */

         if (!error)
            tptr->bits |= B_FOUND;
         return TRUE;         /* Report we found the keyword      */
      } /* if (equal(keyword, tptr->sym)) */
   } /* for */

/*--------------------------------------------------------------------*/
/*      We didn't find the keyword; report failure to the caller      */
/*--------------------------------------------------------------------*/

   if ( period )
      *period = '.';          /* Restore period in keyword           */

   return FALSE;

} /* processconfig */

/*--------------------------------------------------------------------*/
/*    g e t c o n f i g                                               */
/*                                                                    */
/*    Process a single configuration file                             */
/*--------------------------------------------------------------------*/

boolean getconfig(FILE *fp,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  FLAGTABLE *btable)
{

   char buff[BUFSIZ];
   char *cp;

   while(!(fgets(buff, sizeof buff, fp) == nil(char))) {

/*--------------------------------------------------------------------*/
/*                        Ingore comment lines                        */
/*--------------------------------------------------------------------*/

      if (*buff == '#')
         continue;            /* comment line   */

/*--------------------------------------------------------------------*/
/*                       Drop trailing new line                       */
/*--------------------------------------------------------------------*/

      if (*(cp = buff + strlen(buff) - 1) == '\n')
         *cp = '\0';

/*--------------------------------------------------------------------*/
/*                 Drop leading blanks on input line                  */
/*--------------------------------------------------------------------*/

      cp = buff;
      while( isspace( *cp ) )
         cp ++ ;

/*--------------------------------------------------------------------*/
/*                 If line was not blank, process it.                 */
/*--------------------------------------------------------------------*/

      if ( (*cp != '\0') && !processconfig(cp,sysmode,program,table,btable))
         printmsg(0,
               "Unknown keyword \"%s\" in %s configuration file ignored",
               buff, sysmode ? "system" : "user");

   } /*while*/

   return TRUE;

} /*getconfig*/

/*--------------------------------------------------------------------*/
/*    o p t i o n s                                                   */
/*                                                                    */
/*    Process a line of boolean option flags.                         */
/*--------------------------------------------------------------------*/

void options(char *s, SYSMODE sysmode , FLAGTABLE *flags, boolean *barray)
{
   char *token;

   strlwr(s);
   token = strtok(s,WHITESPACE);

   while (token != NULL)
   {
      size_t subscript;
      boolean hit = FALSE;
      boolean negate;
      negate = equaln(token,"no",2) && (strlen(token) > 2);

      for ( subscript=0; (flags[subscript].sym != NULL ) && !hit; subscript++)
      {
         if ((flags[subscript].bits & B_GLOBAL) && (sysmode != SYSTEM_CONFIG))
            continue;
         if (negate)
         {
            if (equal(&token[2],flags[subscript].sym))
            {
               barray[ flags[subscript].position ] = FALSE;
               hit = TRUE;
            }
         } /* if negate */
         else {
            if (equal(token,flags[subscript].sym))
            {
               barray[ flags[subscript].position ] = TRUE;
               hit = TRUE;
            }
         } /* else */
      } /* for */

      if (!hit)
         printf("Invalid or system option '%s' specified\n",token);

      token = strtok(NULL,WHITESPACE);  /* Step to next token on line */

   } /* while */
} /* options */

/*--------------------------------------------------------------------*/
/*    c o n f i g u r e                                               */
/*                                                                    */
/*    Define the global parameters of UUPC/extended                   */
/*--------------------------------------------------------------------*/

boolean configure( CONFIGBITS program)
{
   char *sysrc, *usrrc;
   FILE *fp;
   boolean success;
   char buf[BUFSIZ];
   int subscript = 0;
   char *s;

   CONFIGTABLE *tptr;

   static char *envlist[] = { "EDITOR",   "EDITOR",
                              "HOME",     "HOME",
                              "NAME",     "NAME",
                              "MAILBOX",  "MAILBOX",
                              "LOGNAME",  "MAILBOX",  /* Same as rcs   */
                              "USERNAME", "MAILBOX",  /* Useful for NT */
                              "TEMP",     "TEMPDIR",
                              "TMP",      "TEMPDIR",
                              NULL } ;

   typedef struct _DEFAULTS {
      char **value;
      char *literal;
      boolean path;
   } DEFAULTS;

   static DEFAULTS deflist[] = {
        {&E_archivedir,   "archive" , TRUE },
        {&E_maildir,      "mail"    , TRUE },
        {&E_newsdir,      "news"    , TRUE },
        {&E_pubdir,       "public"  , TRUE },
        {&E_spooldir,     "spool"   , TRUE },
        {&E_tempdir,      "tmp"     , TRUE },
        {&E_systems,      "systems" , TRUE },
        {&E_passwd,       "passwd"  , TRUE },
        {&E_permissions,  "permissn", TRUE },
        {&E_tz,           "tz"      , FALSE},
        { NULL  }
        } ;

/*--------------------------------------------------------------------*/
/*     In Windows/NT, set the console input mode to non-linebased     */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   setstdinmode();
#endif

#ifdef __IBMC__
   logfile = stdout;
#endif

/*--------------------------------------------------------------------*/
/*                  Determine the active environment                  */
/*--------------------------------------------------------------------*/

#if !defined(__TURBOC__) && !defined(BIT32ENV)
   if (_osmode != DOS_MODE)
      active_env = ENV_OS2 | ENV_BIT16;
#endif

   if (!getrcnames(&sysrc, &usrrc))
      return FALSE;

/*--------------------------------------------------------------------*/
/*          Extract selected variables from our environment           */
/*--------------------------------------------------------------------*/

   while( envlist[subscript] != NULL )
   {
      s = getenv( envlist[subscript++] );

      if (s != NULL )
      {
         sprintf(buf,"%s=%s",envlist[subscript], s );
         if ( !processconfig( buf,
                              SYSTEM_CONFIG,
                              program,
                              envtable,
                              configFlags))
         {
            printmsg(0,"Internal error: Invalid keyword %s",
                       envlist[subscript]  );
            panic();
         }
      } /* if (sysrc != NULL ) */

      subscript++;            /* Step to next environment var in list */
   }

/*--------------------------------------------------------------------*/
/*       If the user specified a user name in the environment but     */
/*       not a UUPCUSRRC variable, build a UUPCUSRRC variable         */
/*--------------------------------------------------------------------*/

   if ((usrrc == nil(char)) && ( E_mailbox != NULL ))
   {                             /* Can build from the environment?  */
      strcpy( buf, E_mailbox );
      strcat( buf, ".rc" );
      usrrc = newstr( buf );
      printmsg(2,"Using UUPCUSRRC=%s", usrrc );
   }

/*--------------------------------------------------------------------*/
/*          Determine configuration directory from UUPCSYSRC          */
/*--------------------------------------------------------------------*/

   E_confdir = normalize( sysrc );     /* Make 'em all slashes        */

   s = strrchr( E_confdir, '/' );      /* Get end of path component   */
   if ( s == NULL )                    /* There WAS one, right?       */
   {                                   /* Er, no, sorry.              */
      printmsg(0,"No path name in UUPCSYSRC: %s", sysrc);
      panic();
   }

   *(s+1) = '\0';                      /* Terminate for Config Directory */
   E_confdir = newstr(normalize( E_confdir ));
                                       /* Drop trailing slash unless  */
                                       /* root directory and save     */

/*--------------------------------------------------------------------*/
/*               Process the system configuration file                */
/*--------------------------------------------------------------------*/

   if ((fp = FOPEN(sysrc, "r",TEXT_MODE)) == nil(FILE))
   {
      printmsg(0, "Cannot open system configuration file \"%s\"", sysrc);
      printerr(sysrc);
      return FALSE;
   }

   PushDir( E_confdir );

   success = getconfig(fp, SYSTEM_CONFIG, program, envtable, configFlags);

   fclose(fp);
   if (!success)
   {
      PopDir();
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                Process the user configuration value                */
/*--------------------------------------------------------------------*/

   if (usrrc != nil(char))
   {
      usrrc = normalize( usrrc );
      if ((fp = FOPEN(usrrc, "r",TEXT_MODE)) == nil(FILE))
      {
         printmsg(0, "Cannot open user configuration file \"%s\"", usrrc);
         PopDir();
         return FALSE;
      }

      success = getconfig(fp, USER_CONFIG, program, envtable, configFlags);
      fclose(fp);

      if (!success)
      {
         PopDir();
         return FALSE;
      }

   }

/*--------------------------------------------------------------------*/
/*                       Display our copyright                        */
/*--------------------------------------------------------------------*/

   if (! bflag[F_SUPPRESSCOPYRIGHT] &&
        (program != B_MTA) &&
        isatty(fileno(stdout)))
      fprintf(stdout,
"Changes and Compilation Copyright (c) 1989-1994 by Kendra Electronic\n"
"Wonderworks.  May be freely distributed for reasonable copying fee\n"
"if original documentation and source is included.  See license for\n"
"details and restrictions.\n");

/*--------------------------------------------------------------------*/
/*          Validate that all required parameters were given          */
/*--------------------------------------------------------------------*/

   for (tptr = envtable; tptr->sym != nil(char); tptr++)
   {

      if ((tptr->bits & (B_REQUIRED | B_FOUND)) == B_REQUIRED)
      {
         printmsg(0, "%s configuration parameter \"%s\" must be set.",
            (tptr->bits & B_GLOBAL) ? "System" : "User",
            tptr->sym);
         success = FALSE;
      } /* if */

   } /* for */

/*--------------------------------------------------------------------*/
/*                     Fill in derived parameters                     */
/*--------------------------------------------------------------------*/

   subscript = 0;
   while( deflist[subscript].value != NULL )
   {
      if ( *(deflist[subscript].value) == NULL )
         *(deflist[subscript].value) = deflist[subscript].path ?
                     newstr( normalize(deflist[subscript].literal) ) :
                     deflist[subscript].literal;
      subscript++;
   }

/*--------------------------------------------------------------------*/
/*                         Set our time zone                          */
/*--------------------------------------------------------------------*/

   if ((getenv("TZ") == NULL) && (E_tz != NULL))
   {
      sprintf( buf, "TZ=%s", E_tz );
      E_tz = newstr( E_tz );
      putenv( E_tz );
   }

   tzset();                      /* Set up time zone information  */

   PopDir();

   return success;

} /*configure*/

/*--------------------------------------------------------------------*/
/*    g e t r c n a m e s                                             */
/*                                                                    */
/*    Return the name of the configuration files                      */
/*--------------------------------------------------------------------*/

static boolean getrcnames(char **sysp,char **usrp)
{
   char *debugp = NULL;      /* Pointer to debug environment variable */

   if ((*sysp = getenv(SYSRCSYM)) == nil(char))
   {
      printf("environment variable %s must be specified\n", SYSRCSYM);
      return FALSE;
   }

   *usrp = getenv(USRRCSYM);

   debugp = getenv(SYSDEBUG);

   if ( debugp != nil(char))        /* Debug specified in environment? */
      debuglevel = atoi(debugp);    /* Yes --> preset debuglevel for user */

   return TRUE;

} /*getrcnames*/

/*--------------------------------------------------------------------*/
/*       I s D O S                                                    */
/*                                                                    */
/*       Reports if current enviroment is DOS                         */
/*--------------------------------------------------------------------*/

boolean IsDOS( void )
{
   if ( active_env & ENV_DOS )
      return TRUE;
   else
      return FALSE;

} /* IsDOS */
