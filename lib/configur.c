/*--------------------------------------------------------------------*/
/*    c o n f i g u r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*                                                                    */
/*    Changes Copyright 1990, 1991 (c) Andrew H. Derbyshire           */
/*                                                                    */
/*    History:                                                        */
/*       21Nov1991 Break out of lib.c                          ahd    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Header: E:\src\uupc\LIB\RCS\CONFIGUR.C 1.4 1992/12/01 04:37:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: CONFIGUR.C $
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifndef __GNUC__
#include <io.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hlib.h"
#include "timestmp.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

boolean bflag[F_LAST];        /* Initialized to zero by compiler     */

char *E_aliases = NULL;
char *E_anonymous = NULL;
char *E_archivedir = NULL;
char *E_confdir = NULL;
char *E_domain = NULL;
char *E_altsignature = NULL;
char *E_backup = NULL;
char *E_charset = NULL;
char *E_editor = NULL;
char *E_filesent = NULL;
char *E_inmodem = NULL;
char *E_mailext = NULL;
char *E_pager = NULL;
char *E_signature = NULL;
char *E_uuxqtpath = NULL;
char *E_fdomain = NULL;
char *E_homedir = NULL;
char *E_localdomain = NULL;
char *E_mailbox = NULL;
char *E_maildir = NULL;
char *E_mailserv = NULL;
char *E_name = NULL;
char **E_internal = NULL;
char *E_newsdir = NULL;
char *E_newsserv = NULL;
char *E_nodename = NULL;
char *E_organization = NULL;
char *E_postmaster = NULL;
char *E_pubdir = NULL;
char *E_replyto = NULL;
char *E_spooldir = NULL;
char *E_tempdir = NULL;
char *E_version = NULL;
char *E_uncompress = NULL;
static char *dummy = NULL;

INTEGER E_maxhops = 20;                                     /* ahd */

/*--------------------------------------------------------------------*/
/*                       Local emumerated types                       */
/*--------------------------------------------------------------------*/

typedef enum {
      OS2_ENV,
      DOS_ENV,
      WIN32_ENV,
      UNKNOWN_ENV
      } ENV_TYPE;

#ifdef WIN32
static ENV_TYPE active_env = WIN32_ENV;
#else
static ENV_TYPE active_env = DOS_ENV;
#endif

static boolean getrcnames(char **sysp,char **usrp);

/*--------------------------------------------------------------------*/
/*  The following table controls the configuration files processing   */
/*--------------------------------------------------------------------*/

static CONFIGTABLE envtable[] = {
   {"aliases",      &E_aliases,      B_TOKEN|B_MUA},
   {"altsignature", &E_altsignature, B_TOKEN|B_MUA},
   {"anonymouslogin",
                    &E_anonymous,   B_GLOBAL|B_TOKEN|(B_ALL & ~ B_MAIL)},
   {"archivedir",   &E_archivedir,   B_REQUIRED|B_GLOBAL|B_PATH|B_NEWS},
   {"backupext",    &E_backup,       B_TOKEN|B_MUA},
   {"confdir",      &E_confdir,      B_REQUIRED|B_GLOBAL|B_PATH|B_ALL},
   {"charset",      &E_charset,      B_TOKEN|B_GLOBAL|B_SPOOL},
   {"domain",       &E_domain,       B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"editor",       &E_editor,       B_STRING|B_MUA|B_NEWS},
   {"filesent",     &E_filesent,     B_TOKEN|B_MUA|B_NEWS},
   {"folders",      &dummy,          B_PATH|B_MUSH },
   {"fromdomain",   &E_fdomain,      B_GLOBAL|B_MAIL|B_NEWS|B_TOKEN},
   {"home",         &E_homedir,      B_PATH|B_REQUIRED|B_ALL},
   {"inmodem",      &E_inmodem,      B_GLOBAL|B_TOKEN|B_UUCICO},
   {"internalcommands", (char **)   &E_internal,
                                     B_GLOBAL|B_LIST|B_UUXQT},
   {"localdomain",  &E_localdomain,  B_GLOBAL|B_TOKEN|B_MAIL},
   {"mailbox",      &E_mailbox,      B_REQUIRED|B_TOKEN|B_ALL},
   {"mailext",      &E_mailext,      B_TOKEN|B_MAIL},
   {"maildir",      &E_maildir,      B_REQUIRED|B_GLOBAL|B_PATH|B_MAIL},
   {"mailserv",     &E_mailserv,     B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"maximumhops",  (char **) &E_maxhops,
                                     B_MTA | B_INTEGER | B_GLOBAL},
   {"mushdir",      &dummy,          B_GLOBAL|B_PATH|B_MUSH},
   {"name",         &E_name,         B_REQUIRED|B_MAIL|B_NEWS|B_STRING},
   {"newsdir",      &E_newsdir,      B_REQUIRED|B_GLOBAL|B_PATH|B_NEWS},
   {"newsserv",     &E_newsserv,     B_GLOBAL|B_PATH|B_NEWS},
   {"nodename",     &E_nodename,     B_REQUIRED|B_GLOBAL|B_TOKEN|B_ALL},
   {"options",      (char **) bflag, B_ALL|B_BOOLEAN},
   {"organization", &E_organization, B_STRING|B_MAIL|B_NEWS},
   {"pager",        &E_pager,        B_STRING|B_MUA|B_NEWS},
   {"path",         &E_uuxqtpath,    B_STRING|B_UUXQT|B_GLOBAL},
   {"postmaster",   &E_postmaster,   B_REQUIRED|B_GLOBAL|B_TOKEN|B_MTA},
   {"pubdir",       &E_pubdir,       B_REQUIRED|B_GLOBAL|B_PATH|B_SPOOL},
   {"replyto",      &E_replyto,      B_TOKEN|B_MAIL|B_NEWS},
   {"signature",    &E_signature,    B_TOKEN|B_MUA|B_NEWS},
   {"spooldir",     &E_spooldir,     B_REQUIRED|B_GLOBAL|B_PATH|B_SPOOL|B_NEWS},
   {"tempdir",      &E_tempdir,      B_REQUIRED|B_GLOBAL|B_PATH|B_ALL},
   {"uncompress",   &E_uncompress,   B_GLOBAL|B_STRING|B_NEWS },
   {"version",      &E_version,      B_TOKEN|B_INSTALL},
   {"rmail",        &dummy,          B_OBSOLETE },
   {"rnews",        &dummy,          B_OBSOLETE },
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
 { "pager",       F_PAGER,       B_LOCAL},
 { "purge",       F_PURGE,       B_LOCAL},
 { "save",        F_SAVE,        B_LOCAL},
 { "suppresscopyright",
                  F_SUPPRESSCOPYRIGHT,
                                 B_LOCAL},
 { "speedovermemory",
                  F_SPEEDOVERMEMORY,
                                 B_LOCAL},
 { "verbose",     F_VERBOSE,     B_LOCAL},
 { "undelete",    F_UNDELETE,    B_LOCAL},

 { "bang",        F_BANG,        B_GLOBAL},
 { "collect",     F_COLLECTSTATS,B_GLOBAL},
 { "directory",   F_DIRECT,      B_GLOBAL},
 { "escape",      F_ESCAPE,      B_GLOBAL},
 { "hpfs",        F_HPFS,        B_GLOBAL},
 { "history",     F_HISTORY,     B_GLOBAL},
 { "kanji",       F_KANJI,       B_GLOBAL},
 { "monocase",    F_ONECASE,     B_GLOBAL},
 { "multiqueue",  F_MULTI,       B_GLOBAL},
 { "multitask",   F_MULTITASK,   B_GLOBAL},
 { "snews",       F_SNEWS,       B_GLOBAL},
 { "syslog",      F_SYSLOG,      B_GLOBAL},
 { "symmetricgrades",
                  F_SYMMETRICGRADES,
                                 B_GLOBAL},

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

   keyword = strchr( buff, '.' );   /* Look for environment          */

   if ( keyword == NULL )     /* No environment?                     */
   {
      keyword = buff;         /* Then buffer starts with keyword     */
      target_env = active_env;
   }
   else {
      *keyword++ = '\0';      /* Terminate environment string        */

      if ( equal( buff, "dos" ))
         target_env = DOS_ENV;
      else if ( equal( buff, "os2" ))
         target_env = OS2_ENV;
      else if ( equal( buff, "win32" ))
         target_env = WIN32_ENV;
      else {
         printmsg(0,"Unknown environment \"%s\", keyword \"%s\" ignored",
               buff, keyword );
         return FALSE;
      } /* else */
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
        if (active_env != target_env )
            printmsg(2,"%s-only keyword \"%s\" skipped.",
                        buff, keyword);
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
            else if (tptr->bits & B_INTEGER)
            {
               int *value = (int *) tptr->loc;
               cp = strtok(cp,WHITESPACE);
               if ( equal(cp,"0"))
                  *value = 0;
               else {
                  *value = atoi(cp);
                  if ( *value == 0)
                  {
                     printmsg(0,
                        "Unable to convert \"%s\" value \"%s\" to integer",
                        keyword, cp);
                     error = TRUE;
                  } /* if */
               } /* else */
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

               if (tptr->bits & B_CLIST)  /* Use colon as delimiter? */
                  while ( (colon = strchr( cp , ':')) != NULL)
                     *colon = ' ';     /* Make colons spaces ...           */

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
               while( *cp == ' ' )     /* Trim leading whitespace    */
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
                  *(tptr->loc) = strdup(cp); /* Save string          */
                  checkref( *(tptr->loc) );  /* Verify malloc()      */
               }
               else
                  *(tptr->loc) = newstr(cp); /* Save string          */

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

      for (subscript = 0; (subscript < F_LAST) && !hit; subscript++)
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

   static char *envlist[] = { "EDITOR",   "EDITOR",
                              "HOME",     "HOME",
                              "NAME",     "NAME",
                              "MAILBOX",  "MAILBOX",
                              "TEMP",     "TEMPDIR",
                              "TMP",      "TEMPDIR",
                              NULL } ;

   int subscript = 0;

   CONFIGTABLE *tptr;

/*--------------------------------------------------------------------*/
/*                  Determine the active environment                  */
/*--------------------------------------------------------------------*/

#ifndef __TURBOC__
#ifndef __GNUC__
   if (_osmode != DOS_MODE)
      active_env = OS2_ENV;
#endif
#endif

   if (!getrcnames(&sysrc, &usrrc))
      return FALSE;

/*--------------------------------------------------------------------*/
/*          Extract selected variables from our environment           */
/*--------------------------------------------------------------------*/

   while( envlist[subscript] != NULL )
   {
      char *s = getenv( envlist[subscript++] );

      if (s != NULL )
      {
         char buf[BUFSIZ];
         sprintf(buf,"%s=%s",envlist[subscript], s );
         processconfig( buf, SYSTEM_CONFIG, program, envtable, configFlags);
      } /* if (sysrc != NULL ) */

      subscript++;            /* Step to next environment var in list   */
   }

/*--------------------------------------------------------------------*/
/*               Process the system configuration file                */
/*--------------------------------------------------------------------*/

   if ((fp = FOPEN(sysrc, "r", TEXT)) == nil(FILE))
   {
      printmsg(0, "Cannot open system configuration file \"%s\"", sysrc);
      printerr(sysrc);
      return FALSE;
   }

   success = getconfig(fp, SYSTEM_CONFIG, program, envtable, configFlags);
   fclose(fp);
   if (!success)
      return FALSE;

/*--------------------------------------------------------------------*/
/*                Process the user configuration value                */
/*--------------------------------------------------------------------*/

   if (usrrc != nil(char))
   {
      if ((fp = FOPEN(usrrc, "r", TEXT)) == nil(FILE))
      {
         printmsg(0, "Cannot open user configuration file \"%s\"", usrrc);
         return FALSE;
      }

      success = getconfig(fp, USER_CONFIG, program, envtable, configFlags);
      fclose(fp);
      if (!success)
         return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                       Display our copyright                        */
/*--------------------------------------------------------------------*/

   if (! bflag[F_SUPPRESSCOPYRIGHT] &&
        (program != B_MTA) &&
        isatty(fileno(stdout)))
      fprintf(stdout,
"Changes Copyright (c) 1989 by Andrew H. Derbyshire.  Changes and\n"
"Compilation Copyright (c) 1990-%s by Kendra Electronic Wonderworks.  May\n"
"be freely distributed if original documentation and source is included.\n",
              &compiled[7]);

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

   return success;

} /*configure*/

/*--------------------------------------------------------------------*/
/*    g e t r c n a m e s                                             */
/*                                                                    */
/*    Return the name of the configuration files                      */
/*--------------------------------------------------------------------*/

static boolean getrcnames(char **sysp,char **usrp)
{
   char *debugp = NULL;      /* Pointer to debug environment variable  */

   if ((*sysp = getenv(SYSRCSYM)) == nil(char))
   {
      printf("environment variable %s must be specified\n", SYSRCSYM);
      return FALSE;
   }

   *usrp = getenv(USRRCSYM);

   debugp = getenv(SYSDEBUG);

   if ( debugp != nil(char))        /* Debug specified in environment?     */
      debuglevel = atoi(debugp);    /* Yes --> preset debuglevel for user  */

   return TRUE;

} /*getrcnames*/
