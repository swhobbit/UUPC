/*--------------------------------------------------------------------*/
/*       h o s t a b l e . c                                          */
/*                                                                    */
/*       Remote host table routines for UUPC/extended                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

 /*
  *      $Id: hostable.c 1.31 1997/11/28 23:11:38 ahd v1-12u $
  *
  *      $Log: hostable.c $
  *      Revision 1.31  1997/11/28 23:11:38  ahd
  *      Initial host table is initialized before domain name paranoia check
  *
  *      Revision 1.30  1997/11/24 02:58:14  ahd
  *      Don't allow non-mail programs to call checkname(), which could use
  *      uninitialized local domain name.
  *
  *      Revision 1.29  1997/05/11 04:27:40  ahd
  *      SMTP client support for RMAIL/UUXQT
  *
  *      Revision 1.28  1997/03/31 06:59:39  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.27  1996/11/18 04:46:49  ahd
  *      Normalize arguments to bugout
  *      Reset title after exec of sub-modules
  *      Normalize host status names to use HS_ prefix
  *
  *      Revision 1.26  1996/01/01 20:52:21  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.25  1995/07/21 13:23:19  ahd
  *      Clean up OS/2 compiler warnings
  *
  *      Revision 1.24  1995/03/24 04:17:22  ahd
  *      Compiler warning message cleanup, optimize for low memory processing
  *
  *      Revision 1.23  1995/02/25 18:21:44  ahd
  *      Correct selected flags for config variables
  *
  *      Revision 1.22  1995/02/21 13:02:33  ahd
  *      Supress compiler warning
  *
  *      Revision 1.21  1995/02/21 03:30:52  ahd
  *      More compiler warning cleanup, drop selected messages at compile
  *      time if not debugging.
  *
  *      Revision 1.20  1995/01/29 16:43:03  ahd
  *      IBM C/Set compiler warnings
  *
  *      Revision 1.19  1995/01/07 16:12:32  ahd
  *      Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
  *
  *      Revision 1.18  1994/12/27 20:49:47  ahd
  *      Revise confusing local domain message
  *
  *      Revision 1.17  1994/12/22 00:08:50  ahd
  *      Annual Copyright Update
  *
  *      Revision 1.16  1994/05/08 21:43:33  ahd
  *      Report caller of checkname(), checkreal() on panics
  *
 *     Revision 1.15  1994/02/20  19:07:38  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.11  1994/02/18  23:09:55  ahd
 *     Use standard first header
 *
 *     Revision 1.9  1994/01/01  19:02:25  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.8  1993/10/12  00:45:27  ahd
 *     Normalize comments
 *
 *     Revision 1.7  1993/06/21  02:17:31  ahd
 *     Correct errors in mail routing via HOSTPATH
 *
 *     Revision 1.6  1993/05/29  15:19:59  ahd
 *     Allow configured systems, passwd files
 *
 *     Revision 1.5  1993/04/11  00:32:29  ahd
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.4  1993/04/04  04:57:01  ahd
 *     Trap existence of local host name in SYSTEMS file
 *
 *     Revision 1.3  1992/12/18  12:05:57  ahd
 *     Suppress duplicate machine state messages to improving OS/2 scrolling
 *
 *     Revision 1.3  1992/12/18  12:05:57  ahd
 *     Suppress duplicate machine state messages to improving OS/2 scrolling
 *
 *    Revision 1.2  1992/11/22  20:58:55  ahd
 *    Use strpool to allocate const strings
 *
 *    18 Mar 1990 Create hostable.c from router.c                    ahd
 *                Move code to generate localdomain to here          ahd
 *    22 Apr 90   Perform check for full host name before examining
 *                name without domain.                               ahd
 *    29 Jul 90   Only load host table based on first six characters
 *                of host name.                                      ahd
 *
  */

#include "uupcmoah.h"

#include <ctype.h>

#include <limits.h>

#include "hostable.h"
#include "security.h"

currentfile();

static struct HostTable *hosts = NULL;

static size_t  HostElements = 0;

static size_t loadhost( void );

static int hostcmp( const void *a , const void *b );

static size_t localdomainl;   /* Length of localdomain                */

/*--------------------------------------------------------------------*/
/*    c h e c k n a m e                                               */
/*                                                                    */
/*    Perform a search for a single host name                         */
/*                                                                    */
/*    Rewritten for release 1.10a.  The old release had               */
/*    most of the same logic, but nested it inside the search         */
/*    loop; adding new cases (specifically, the wildcard domain       */
/*    search) was difficult.  This version is slower because it       */
/*    makes multiple passes through the host table, but this          */
/*    isn't really performance code for a small (under 100 hosts)     */
/*    table.                                 ahd 26 April 1991        */
/*                                                                    */
/*    Note because we save the arguments and use a static variable    */
/*    to save the result of searches, this function is not            */
/*    recursive!                                                      */
/*--------------------------------------------------------------------*/

struct HostTable *checkName(const char *name,
                            const size_t line,
                            const char *function)
{
   char  hostname[MAXADDR];   /* Local copy of name to process        */
   char *period;              /* Pointer "." in hostname              */
   size_t namel;              /* Length of the name input             */
   size_t column;             /* Length of the name input             */

   static char savename[MAXADDR] = "";
                              /* Saved copy of name to make function
                                 reducible                            */
   static struct HostTable *hostz;

/*--------------------------------------------------------------------*/
/*                       Validate the argument                        */
/*--------------------------------------------------------------------*/

   if ((name == NULL) || (*name == '\0'))
   {
      printmsg(0,"checkName: Invalid (missing) hostname passed");
      bugout(function, line);
   }

   namel = strlen(name);

 /*-------------------------------------------------------------------*/
 /*             Initialize the host name table if needed              */
 /*-------------------------------------------------------------------*/

   if (HostElements == 0)           /* host table initialized yet?    */
      HostElements = loadhost();        /* No --> load it             */

/*--------------------------------------------------------------------*/
/*       Validate our environment; if the domain information is       */
/*       not initialized, program should only be calling              */
/*       checkreal() directly.                                        */
/*--------------------------------------------------------------------*/

   if ( E_localdomain == NULL )
   {
      printmsg(0,"Internal error: Attempt to use local domain "
                  "without initialization" );
      bugout(function, line);
   }

/*--------------------------------------------------------------------*/
/*    If same argument as last time, return same result; otherwise    */
/*    save input for next pass                                        */
/*--------------------------------------------------------------------*/

   if (equali(name, savename))
      return hostz;

   strcpy( savename, name);   /* Save for next pass                   */

/*--------------------------------------------------------------------*/
/*                      Search for the full name                      */
/*--------------------------------------------------------------------*/

   if ((hostz = searchname(name, MAXADDR)) != BADHOST)
      return hostz;

/*--------------------------------------------------------------------*/
/*    If the name already has the local domain attached, search for   */
/*    the host name without the domain.                               */
/*--------------------------------------------------------------------*/

   column = namel - localdomainl;

   if ((namel > localdomainl) && equali(E_localdomain, &name[column]) &&
       (name[ column - 1] == '.'))
   {
      if ((hostz = searchname(name,column-1 )) != BADHOST)
         return hostz;
   } /* if */

/*--------------------------------------------------------------------*/
/*    If the name already has the UUCP  domain attached, search for   */
/*    the host name without the domain.                               */
/*--------------------------------------------------------------------*/

   column = namel - 5;

   if ((column > 0) && equali(".UUCP", &name[column]))
   {
      if ((hostz = searchname(name, column )) != BADHOST)
         return hostz;
   } /* if */

/*--------------------------------------------------------------------*/
/*              Search for the name in the local domain               */
/*--------------------------------------------------------------------*/

   if ((namel + localdomainl + 2) < MAXADDR)
   {
      strcpy( hostname, name );
      strcat( hostname, "." );
      strcat( hostname, E_localdomain );

      if ((hostz = searchname(hostname, MAXADDR)) != BADHOST)
         return hostz;

   } /* if */

/*--------------------------------------------------------------------*/
/*    If a simple name and not found, return search for truncated     */
/*    UNIX name.                                                      */
/*--------------------------------------------------------------------*/

   if ( strchr(name,'.') == NULL )
      return checkreal( name );

/*--------------------------------------------------------------------*/
/*               Perform a wildcard domain name search                */
/*--------------------------------------------------------------------*/

   period = (char *) name;          /* Begin at front of name        */

   while( (period = strchr( ++period, '.' )) != NULL )
   {

      hostname[0] = '*';               /* Do wildcard search time    */
      strcpy( hostname + 1, period );

      if ((hostz = searchname(hostname, MAXADDR)) != BADHOST)
         return hostz;

   }

/*--------------------------------------------------------------------*/
/*         We didn't find the host.  Return failure to caller         */
/*--------------------------------------------------------------------*/

   return BADHOST;

}  /* checkname */

/*--------------------------------------------------------------------*/
/*    c h e c k r e a l                                               */
/*                                                                    */
/*    Perform a search for a real (connected) simple host name        */
/*--------------------------------------------------------------------*/

struct HostTable *checkReal(const char *name,
                            const size_t line,
                            const char *function)
{
   size_t  namel;
   struct HostTable *hostp;

/*--------------------------------------------------------------------*/
/*                       Validate the argument                        */
/*--------------------------------------------------------------------*/

   if ((name == NULL) || ((namel = strlen(name)) == 0))
   {
      printmsg(0,"checkReal: Invalid (missing) hostname passed");
      bugout(function, line);
   }

   namel = max( strlen(name), HOSTLEN);
   hostp = searchname( name, namel );

/*--------------------------------------------------------------------*/
/*             If we didn't find the host, return failure             */
/*--------------------------------------------------------------------*/

   if ((hostp == BADHOST) || (hostp->status.hstatus >= HS_NOCALL))
      return hostp;           /* Return raw information               */
   else
      return BADHOST;         /* Not a real host, invalid for our
                                 purposes                             */

} /* searchreal */

/*--------------------------------------------------------------------*/
/*    s e a r c h n a m e                                             */
/*                                                                    */
/*    Look up a system name in our systems (L.sys) file.              */
/*    Only the first 7 characters of a system name is significant.    */
/*--------------------------------------------------------------------*/

struct HostTable *searchname(const char *name, const size_t namel)
{
   int   lower;
   int   upper;

 /*-------------------------------------------------------------------*/
 /*             Initialize the host name table if needed              */
 /*-------------------------------------------------------------------*/

   if (HostElements == 0)           /* host table initialized yet?    */
      HostElements = loadhost();        /* No --> load it             */

   lower = 0;
   upper = (int) (HostElements - 1);

   while ( lower <= upper )
   {
      int midpoint = (lower + upper) / 2;
      int hit;

/*--------------------------------------------------------------------*/
/*    Compare for up to the specified length of the host name, but    */
/*    never less than the length of the item we are comparing it      */
/*    to.  In other words, the search key can be shorter than the     */
/*    table entry for a simple host name.                             */
/*                                                                    */
/*    This mostly affects simple host names, as domain names          */
/*    have a Very Large Number (VLN) passed into to insure we         */
/*    compare the entire length of the name.                          */
/*--------------------------------------------------------------------*/

      hit = strnicmp(name,hosts[midpoint].hostname,namel);

      if (hit > 0)
         lower = midpoint + 1;
      else if ((hit < 0) || (strlen(hosts[midpoint].hostname) > namel))
         upper = midpoint - 1;
      else {

#ifdef UDEBUG
            printmsg(8,"searchname: Looking for \"%s\" of length %d,\
 found \"%s\"",
            name,
            namel,
            hosts[midpoint].hostname);
#endif

         return &hosts[midpoint];
      }
   }

/*--------------------------------------------------------------------*/
/*         We didn't find the host.  Return failure to caller         */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   printmsg(8,"searchname: Looking for \"%s\", did not find it",
            name);
#endif

   return BADHOST;

}  /* searchname */

/*--------------------------------------------------------------------*/
/*    n e x t h o s t                                                 */
/*                                                                    */
/*    Returns next host in table with requested attribute             */
/*--------------------------------------------------------------------*/

struct HostTable *nexthost( const KWBoolean start )
{
   static size_t current = 0;

   if (HostElements == 0)     /* host table initialized yet?          */
      HostElements = loadhost(); /* No --> load it                    */

   if (start)
      current = 0;
   else
      current ++;

   while ( current < HostElements )
   {
      if (hosts[current].status.hstatus >= HS_NOCALL)
         return &hosts[current];
      else
         current++;
   }

   return BADHOST;

}  /* nexthost */

/*--------------------------------------------------------------------*/
/*    i n i t h o s t                                                 */
/*                                                                    */
/*    Intializes a host table entry for for loadhost                  */
/*--------------------------------------------------------------------*/

struct HostTable *inithost(char *name)
{

   size_t hit = HostElements;
   size_t element = 0;
   static size_t max_elements = 32; /* This is automatically
                                       raised if we run out of room   */

   if (hosts == NULL)
   {
      hosts = calloc(max_elements, sizeof(*hosts));

#ifdef UDEBUG
      printmsg(5,"inithost: Allocated room for %d host entries",
               max_elements);
#endif

   }
   else if ( max_elements == HostElements )
   {
      max_elements = max_elements * 2;
      hosts = realloc(hosts , max_elements * sizeof(*hosts));

#ifdef UDEBUG
      printmsg(5,"inithost: reallocated room for %d host entries",
               max_elements);
#endif

   }

   checkref(hosts);

/*--------------------------------------------------------------------*/
/*    Add the host to the table.  Note that we must add the host      */
/*    to the table ourselves (rather than use lsearch) because we     */
/*    must make a copy of the string; the *token we use for the       */
/*    search is in the middle of our I/O buffer!                      */
/*--------------------------------------------------------------------*/

   while ( element < hit )
   {
      if (equali( hosts[element].hostname , name ))
         hit = element;
      else
         element++;
   }

/*--------------------------------------------------------------------*/
/*               If a new element, initialize the block               */
/*--------------------------------------------------------------------*/

   if (hit == HostElements)
   {
      memset( &hosts[hit] , 0, sizeof hosts[hit] );
      hosts[hit].hostname = newstr(name);
      checkref( hosts[hit].hostname );
      hosts[hit].anylogin = KWTrue;  /* Allow generic login by default */
      HostElements ++ ;
   } /* if */

   return &hosts[hit];

} /* inithost */

/*--------------------------------------------------------------------*/
/*    l o a d h o s t                                                 */
/*                                                                    */
/*    Initializes table of known host names for checkname             */
/*--------------------------------------------------------------------*/

static size_t loadhost()
{
   FILE *ff;
   char *token;
   char s_hostable[FILENAME_MAX]; /* full-name of hostable file       */
   size_t hit;

   struct HostTable *hostp;

/*--------------------------------------------------------------------*/
/*                  Load the local host information                   */
/*--------------------------------------------------------------------*/

   hostp = inithost(E_nodename);
   hostp->status.hstatus  = HS_LOCALHOST;
   hostp->realname = E_nodename; /* Don't let user alias our system
                                    name                              */

/*--------------------------------------------------------------------*/
/*                Now do the local domain information                 */
/*--------------------------------------------------------------------*/

   if ( E_domain != NULL )
   {

      if (strchr(E_domain,'.') == NULL)
      {
         printmsg(0,"Domain name \"%s\" is invalid, missing period",
                     E_domain);
         panic();
      }

      token = strchr( E_domain, '%' );

      if ( token == NULL )
         token = strchr( E_domain, '!' );

      if (token != NULL)
      {
         printmsg(0,"Domain name \"%s\" contains invalid character \"%c\"",
                     E_domain,
                     *token );
         panic();
      }

      hostp = inithost(E_domain);
      hostp->via = hostp->realname = E_nodename;

   } /* if ( E_domain != NULL ) */

/*--------------------------------------------------------------------*/
/*    If we allow anonymous UUCP, load the dummy host we use for      */
/*    connections to such hosts                                       */
/*--------------------------------------------------------------------*/

   if ( E_anonymous != NULL )
   {
      hostp = inithost( ANONYMOUS_HOST );
      hostp->status.hstatus = HS_NOCALL;
      hostp->via     = E_nodename;
   } /* if */

/*--------------------------------------------------------------------*/
/*                  Load names from the systems file                  */
/*--------------------------------------------------------------------*/

   ff = FOPEN(E_systems, "r",TEXT_MODE);

   if (ff == NULL)
   {
      printerr(E_systems);
      panic();
   }

   while (! feof(ff))
   {
      char buf[BUFSIZ];

      if (fgets(buf,BUFSIZ,ff) == NULL)   /* Try to read a line       */
         break;                  /* Exit if end of file               */

      token = strtok(buf,WHITESPACE);

      if (token == NULL)         /* Any data?                         */
         continue;               /* No --> read another line          */

      if (token[0] == '#')
         continue;                  /* Line is a comment; loop again  */

      if ( equali( token, E_nodename ))
      {
         printmsg(0,"Error: Local host %s must not be in SYSTEMS file",
                    E_nodename );
         panic();
      }

      hostp = inithost(token);

      if (hostp->status.hstatus == HS_PHANTOM)
      {
         hostp->status.hstatus = HS_NOCALL;
      }

   } /* while */

   fclose(ff);

/*--------------------------------------------------------------------*/
/*               Now the load the routing file, if any.               */
/*--------------------------------------------------------------------*/

   mkfilename(s_hostable, E_confdir, PATHS);

   if ((E_mailserv != NULL ) &&
       (ff = FOPEN(s_hostable, "r",TEXT_MODE)) != NULL)
   {

      while (! feof(ff))
      {
         char buf[BUFSIZ];

         KWBoolean freeit = KWFalse;

         if (fgets(buf,BUFSIZ,ff) == NULL)   /* Try to read a line    */
            break;                  /* Exit if end of file            */

         token = strtok(buf,WHITESPACE);

         if (token == NULL)         /* Any data?                      */
            continue;               /* No --> read another line       */

         if (*token == '#')
            continue;               /* Line is a comment; loop again  */

         hostp = inithost(token);
         token = strtok(NULL,WHITESPACE);

         if ( token == NULL )
         {
            printmsg(0,"loadhost: Missing path name for host \"%s\"",
                        hostp->hostname);
            freeit = KWTrue;
         }

/*--------------------------------------------------------------------*/
/*                              Gate way                              */
/*--------------------------------------------------------------------*/

         else if ((equal(token,"|") || equal(token,"@")))
         {
            char operator = *token;

            token = strtok(NULL,"\n");

            if (( hostp->via != NULL ) || ( token == NULL ))
               freeit = KWTrue;
            else {
               if ( operator == '@' )
                  hostp->status.hstatus = HS_SMTP;
               else
                  hostp->status.hstatus = HS_GATEWAYED;

               while(isspace( *token ))   /* Drop leading white space only */
                  token++;

               if (*token == '\0')        /* Empty string?            */
                  freeit = KWTrue;         /* Yes --> Flag for error   */
               else
                  hostp->via = token = newstr(token);

            } /* else if */

            if ( freeit )
               printmsg(0,"loadhost: Invalid/duplicate gateway for \"%s\"",
                     hostp->hostname );

         } /* else if */

/*--------------------------------------------------------------------*/
/*                               Alias                                */
/*--------------------------------------------------------------------*/

         else if (equal(token,"="))
         {
            token = strtok(NULL,WHITESPACE);

            if (( hostp->realname == NULL ) && (token != NULL))
               hostp->realname = token = newstr( token );
            else {

               printmsg(0,"loadhost: Invalid/duplicate alias of \"%s\"",
                     hostp->hostname );
               freeit = KWTrue;

            } /* else */

         } /* else if (equal(token,"=")) */

/*--------------------------------------------------------------------*/
/*                           Routing entry                            */
/*--------------------------------------------------------------------*/

         else {

            if ( hostp->via == NULL )
               hostp->via = token = newstr( token );
            else {
               printmsg(0,"loadhost: Invalid/duplicate route for \"%s\"",
                     hostp->hostname );
               freeit = KWTrue;
            } /* else */

         } /* else */

         if ( ! freeit )
         {
            checkref( token );

            if (*token == '*')       /* Wildcard on right side?    */
            {
               printmsg(0, "loadhost: Wildcard \"%s\" not "
                           "allowed for real name of host \"%s\"",
                           token,
                           hostp->hostname);
               freeit = KWTrue;

            } /* if (*token == '*') */

         } /* if ( ! freeit ) */

         if ( freeit )
         {
            if ( hostp->status.hstatus == HS_PHANTOM )
               HostElements--;            /* Ignore the routing entry */
         }

      }  /* end while */

      fclose(ff);

   } /* if ((E_mailserv != NULL ) && ... */
   else {

      if (( debuglevel > 2 ) && (E_mailserv != NULL ))
         perror( s_hostable );

   } /* else */

/*--------------------------------------------------------------------*/
/*                   Provide default for fromdomain                   */
/*--------------------------------------------------------------------*/

   if ( E_mailserv != NULL )
   {
      if (E_fdomain != NULL)     /* If fromdomain provided ...        */
      {
         hostp = inithost(E_fdomain);

         if (hostp->via == NULL)    /* Uninitialized?                 */
            hostp->via = E_mailserv;   /* Yes --> Use default route   */
      }
      else
         E_fdomain = E_domain;   /* Use domain as fromdomain          */
   }

/*--------------------------------------------------------------------*/
/*    Shrink the table to whatever we actually need, then sort it     */
/*--------------------------------------------------------------------*/

   hosts = realloc(hosts, HostElements *  sizeof(*hosts));
   checkref(hosts);

   qsort(hosts, HostElements ,sizeof(hosts[0]) , hostcmp);

/*--------------------------------------------------------------------*/
/*                      Display the final table                       */
/*--------------------------------------------------------------------*/

   for (hit = 0; hit < HostElements; hit++)
   {
      printmsg(8,"loadhost: entry[%02d:%02d] %-20s\tvia %s\talias %s",
                  hit,
                  hosts[hit].status.hstatus,
                  hosts[hit].hostname,
                  (hosts[hit].via == NULL) ? "(self)" : hosts[hit].via,
                  (hosts[hit].realname == NULL)
                                    ? "(self)" : hosts[hit].realname);
   } /* for */

/*--------------------------------------------------------------------*/
/*      If not processing domain/routing information, return now      */
/*--------------------------------------------------------------------*/

   if ( E_mailserv == NULL )
      return (HostElements) ;

/*--------------------------------------------------------------------*/
/*    If the user did not define a local domain, then generate one    */
/*    based on our own domain name; the generated name will either    */
/*    be of the format ".a.b.c" (incuding the trailing period) or     */
/*    a null string.                                                  */
/*--------------------------------------------------------------------*/

   if ( E_localdomain == NULL )
   {
      E_localdomain = strchr(E_domain,'.');

      if (E_localdomain == NULL)
         E_localdomain = "UUCP";
      else {

         E_localdomain ++;    /* Step past the period                 */

         if ( !equali(E_localdomain, "UUCP" ) &&
              (strchr( E_localdomain, '.' ) == NULL ))
                              /* Implied single level domain name?    */
            E_localdomain = E_domain;
                              /* Impossible, use both parts of name   */

      } /* else */

      printmsg(3,"loadhost: domain name is %s, "
                 "domain in RFC-822 mail is %s, "
                 "other systems assumed in %s domain",
                 E_domain,
                 E_fdomain,
                 E_localdomain);

   } /* if */

   localdomainl = strlen(E_localdomain);

/*--------------------------------------------------------------------*/
/*    Amend for the sin of typing the domain wrong in the sample      */
/*    files for the old releases of UUPC/extended.                    */
/*--------------------------------------------------------------------*/

   if (equali(E_localdomain,"UUPC"))
   {
      printmsg(0,"inithost: UUPC is an invalid domain name! "
                 "Change it to UUCP");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return (HostElements) ;

} /*loadhost*/

/*--------------------------------------------------------------------*/
/*    h o s t c m p                                                   */
/*                                                                    */
/*    Accepts indirect pointers to two strings and compares           */
/*    them using stricmp (case insensitive string compare)            */
/*--------------------------------------------------------------------*/

int hostcmp( const void *a , const void *b )
{

   return stricmp(((struct HostTable*) a)->hostname,
        ((struct HostTable*) b)->hostname);

}  /*hostcmp*/
