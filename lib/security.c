/*--------------------------------------------------------------------*/
/*    s e c u r i t y . c                                             */
/*                                                                    */
/*    Security routines for UUPC/extended                             */
/*                                                                    */
/*    Copyright (c) 1991, Andrew H. Derbyshire                        */
/*    See README.PRN for additional copyrights and restrictions       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: security.c 1.8 1993/09/20 04:38:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: security.c $
 *     Revision 1.8  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.7  1993/08/03  03:11:49  ahd
 *     Make missing directories non-fatal
 *
 *     Revision 1.6  1993/05/06  03:41:48  ahd
 *     Use NULL to denote current directory, not "."
 *
 *     Revision 1.5  1993/04/11  00:31:04  ahd
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.4  1993/03/06  22:48:23  ahd
 *     Re-do compare of sort to void bug in some qsort() functions
 *
 * Revision 1.3  1992/11/22  20:58:55  ahd
 * Normalize directories as read
 * Use strpool to allocate const strings
 *
 * Revision 1.2  1992/11/19  02:57:31  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>        /* Only really needed for MS C         */
#include <sys/stat.h>
#include <time.h>
#include <direct.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "hostable.h"
#include "security.h"
#include "usertabl.h"
#include "expath.h"
#include "hlib.h"

/*--------------------------------------------------------------------*/
/*                           Local defines                            */
/*--------------------------------------------------------------------*/

static boolean InitEntry( char *buf, const char *fname);

static size_t InitDir( char *directories,
         const REMOTE_ACCESS access,
         const boolean grant,
         struct HostSecurity *anchor,
         size_t max_elements );

int dircmp( const void *a , const void *b );

/*--------------------------------------------------------------------*/
/*                          Global varables                           */
/*--------------------------------------------------------------------*/

struct HostSecurity *securep = NULL;
static struct HostSecurity *default_security = NULL;
static char drive[] = "C:";

currentfile();

/*--------------------------------------------------------------------*/
/*    L o a d S e c u r i t y                                         */
/*                                                                    */
/*    Initialize security processing; returns TRUE if security        */
/*    initialized, otherewise FALSE                                   */
/*--------------------------------------------------------------------*/

boolean LoadSecurity( void )
{
   char fname[FILENAME_MAX];
   char buffer[BUFSIZ*4];     /* Allows around 2K for the data       */
   struct HostTable *hostp;
   FILE *stream;

/*--------------------------------------------------------------------*/
/*      Generate a filename for the permissions file and open it      */
/*--------------------------------------------------------------------*/

   mkfilename(fname, E_confdir, PERMISSIONS);
   stream  = FOPEN( fname, "r",TEXT_MODE);

   if ( stream == NULL )      /* Did the file open?                  */
   {                          /* No --> Report failure to caller     */
      printerr( fname );
      return FALSE;
   } /* ( stream == NULL ) */

/*--------------------------------------------------------------------*/
/*              Get current drive for normalizing names               */
/*--------------------------------------------------------------------*/

   getcwd( buffer, sizeof buffer );
   *drive = *buffer;

/*--------------------------------------------------------------------*/
/*               Begin processing the PERMISSIONS file                */
/*--------------------------------------------------------------------*/

   while ( !feof( stream ) )
   {
      char *next = buffer;

/*--------------------------------------------------------------------*/
/*                Build up the buffer to be processed                 */
/*--------------------------------------------------------------------*/

      *next = '\0';
      while( fgets( next, sizeof buffer - strlen(next), stream ) != NULL)
      {
         if ((*next == '#') || (*next == '\n'))
         {
            *next = '\0';
            continue;
         }

         next = next + strlen( next ) - 1;
         if (*next == '\n')
            *next-- = '\0';
         else if (!feof( stream ))  /* Did we hit EOF?               */
         {                    /* No --> Presume the buffer overflowed*/
            printmsg(0,"LoadSecurity: buffer overflow while reading %s",
             fname);
            fclose( stream );
            return FALSE;
         }

         while( isspace( *next ))   /* Dump trailing white space     */
            *next-- = '\0';

         if (*next == '\\')
            *next = '\0';
         else
            break;
      } /* while( fgets( next, sizeof available, stream )) != NULL)) */

/*--------------------------------------------------------------------*/
/*            Done read the data; verify we had no errors             */
/*--------------------------------------------------------------------*/

      if (ferror( stream ))
      {
         printerr( fname );
         clearerr( stream );
         return FALSE;
      } /* if */

/*--------------------------------------------------------------------*/
/*              Build entries for one permissions entry               */
/*--------------------------------------------------------------------*/

      printmsg(10,"Buffer is \"%s\"", buffer );
      if ((*next != '\0') && !InitEntry( buffer , fname))
      {
         fclose( stream );
         return FALSE;
      }

   } /* while ( !feof( stream ) ) */

/*--------------------------------------------------------------------*/
/*                    Initialize local host entry                     */
/*--------------------------------------------------------------------*/

   hostp = checkname( E_nodename );
   if ( hostp == NULL )
      panic();
   hostp->hsecure = malloc( sizeof *hostp->hsecure );
   checkref( hostp->hsecure );
   memset( hostp->hsecure , '\0', sizeof *hostp->hsecure);
                              /* Clear pointers                      */
   hostp->hsecure->local = TRUE;

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   fclose( stream );
   return TRUE;

} /* LoadSecurity */

/*--------------------------------------------------------------------*/
/*    I n i t i a l i z e E n t r y                                   */
/*                                                                    */
/*    Initialize a single permissions file entry                      */
/*--------------------------------------------------------------------*/

static boolean InitEntry( char *buf, const char *fname)
{

/*--------------------------------------------------------------------*/
/*                      Configuration variables                       */
/*--------------------------------------------------------------------*/

  static char *myname, *validate, *commands;

  static char *callback, *xpubdir, *machine, *noread, *nowrite;
  static char *request, *read, *sendfiles,  *write, *logname;

  static CONFIGTABLE securetable[] = {
     { "callback",      &callback,     B_TOKEN  | B_UUXQT } ,
     { "commands",      &commands,     B_CLIST  | B_UUXQT } ,
     { "logname",       &logname,      B_TOKEN  | B_UUXQT } ,
     { "machine",       &machine,      B_TOKEN  | B_UUXQT | B_MALLOC } ,
     { "myname",        &myname,       B_TOKEN  | B_UUXQT } ,
     { "pubdir",        &xpubdir,      B_PATH   | B_UUXQT } ,
     { "noread",        &noread,       B_TOKEN  | B_UUXQT | B_MALLOC } ,
     { "nowrite",       &nowrite,      B_TOKEN  | B_UUXQT | B_MALLOC } ,
     { "read",          &read,         B_TOKEN  | B_UUXQT | B_MALLOC} ,
     { "request",       &request,      B_TOKEN  | B_UUXQT } ,
     { "sendfiles",     &sendfiles,    B_TOKEN  | B_UUXQT } ,
     { "validate",      &validate,     B_CLIST  | B_UUXQT } ,
     { "write",         &write,        B_TOKEN  | B_UUXQT | B_MALLOC } ,
     { nil(char) }
}; /* securetable */

   struct HostSecurity *anchor = malloc( sizeof *anchor );

/*--------------------------------------------------------------------*/
/*                  Default list of allowed commands                  */
/*--------------------------------------------------------------------*/

   static char *command_list[] = { "rmail", "rnews" , NULL } ;

/*--------------------------------------------------------------------*/
/*                          Other variables                           */
/*--------------------------------------------------------------------*/

   boolean success = TRUE;
   CONFIGTABLE *tptr;
   char *token = buf;
   char *parameter;
   struct UserTable *userp;
   struct HostTable *hostp;
   size_t max_elements = 16;

/*--------------------------------------------------------------------*/
/*                 Initialize the security structure                  */
/*--------------------------------------------------------------------*/

   checkref( anchor );
   memset( anchor , '\0', sizeof *anchor); /* Clear pointers         */

/*--------------------------------------------------------------------*/
/*                        Initialize the table                        */
/*--------------------------------------------------------------------*/

   for (tptr = securetable; tptr->sym != nil(char); tptr++)
      if (tptr->bits & (B_TOKEN | B_STRING | B_LIST| B_CLIST))
         *(tptr->loc) = nil(char);

/*--------------------------------------------------------------------*/
/*                 Parse the information in the table                 */
/*--------------------------------------------------------------------*/

   while ( (parameter = strtok( token, WHITESPACE )) != NULL)
   {
      token = strtok( NULL, ""); /* Save for next pass               */
#ifdef _DEBUG
      printmsg(8,"InitEntry: Parameter is \"%s\"", parameter);
      if ( token != NULL )
         printmsg(10,"InitEntry: Buffer remaining is \"%s\"", token);
#endif
      if (!processconfig(parameter, SYSTEM_CONFIG,B_UUXQT,securetable,NULL))
      {
         printmsg(0,
          "Unknown keyword \"%s\" in %s ignored",parameter, fname);
         success = FALSE;
      } /* if */
   } /* while ( (parameter = strtok( token, WHITESPACE )) != NULL) */

   anchor->commands = (char **) commands;
   anchor->validate = (char **) validate;

/*--------------------------------------------------------------------*/
/*    Now we have the data procesed by keyword, break it down more    */
/*--------------------------------------------------------------------*/

   if ((logname == NULL) && (machine == NULL))
   {
      printmsg(0,"InitEntry: No machine or logname given in %s",
                  fname );
      success = FALSE;
   } /* if ((logname == NULL) && (machine == NULL)) */

/*--------------------------------------------------------------------*/
/*                        Handle a login name                         */
/*--------------------------------------------------------------------*/

   if (logname != NULL)
   {
      printmsg(10,"InitEntry: Processing logname=%s",logname );
      userp = checkuser( logname );
      if ( userp == BADUSER )
      {
         printmsg(0,"InitEntry: Invalid user id in %s, LOGNAME=%s",
                     fname, logname );
         success = FALSE;
      } /* if ( userp == BADUSER ) */
      else if (userp->hsecure == NULL)
         userp->hsecure = anchor;
      else {
         printmsg(0,"InitEntry: Duplicate user id in %s, LOGNAME=%s",
                     fname, logname );
         success = FALSE;
      } /* else */
   } /* if (logname != NULL) */

/*--------------------------------------------------------------------*/
/*                        Handle machine names                        */
/*--------------------------------------------------------------------*/

   token = machine;
   while( token != NULL )
   {
      char *host = strtok( token, ":");
      printmsg(10,"InitEntry: Processing machine=%s", host );

      token = strtok( NULL, "");
      if ( equal( host , ANY_HOST ) )
      {
         if ( default_security == NULL )
            default_security = anchor;
         else {
            printmsg(0,"InitEntry: "
                       "Multiple MACHINE entries in %s which specify OTHER",
                       fname);
            success = FALSE;
         } /* else */
      } /* if ( equal( host , ANY_HOST ) ) */
      else {
         hostp = checkreal( host );
         if ( hostp == BADUSER )
         {
            printmsg(0,"InitEntry: Invalid host id in %s, MACHINE=%s",
                        fname, host );
            success = FALSE;
         } /* if ( hostp == BADUSER ) */
         else if (hostp->hsecure == NULL)
            hostp->hsecure = anchor;
         else {
            printmsg(0,"InitEntry: Duplicate host id in %s, MACHINE=%s",
                        fname, token );
            success = FALSE;
         } /* else */
      } /* else */
   } /* while( token != NULL ) */

   if ( machine != NULL )
      free( machine );

/*--------------------------------------------------------------------*/
/*                       Handle validated names                       */
/*--------------------------------------------------------------------*/

   if ( anchor->validate != NULL )
   {
      char **plist = anchor->validate;

      while ( *plist != NULL )
      {
         hostp = checkreal( *plist );

         if ( hostp == BADUSER )
         {
            printmsg(0,"InitEntry: Invalid host id in %s, VALIDATE=%s",
                        fname, *plist);
            success = FALSE;
         } /* if ( hostp == BADUSER ) */
         else
            hostp->anylogin = FALSE;   /* Flag we must use specific
                                          login                      */

         plist++;             /* Step to next hostname in list       */

      } /* while ( *plist != NULL ) */

   } /* if ( anchor->validate != NULL ) */

/*--------------------------------------------------------------------*/
/*                          Handle CALLBACK                           */
/*--------------------------------------------------------------------*/

   if ( callback != NULL )
   {
      if (equal(strlwr(callback),"no"))
         anchor->callback = FALSE;
      else if (equal(callback,"yes"))
         anchor->callback = TRUE;
      else {
         printmsg(0,"InitEntry: Invalid value in %s, CALLBACK=%s",
                     fname, callback );
         success = FALSE;
      } /* else */
   } /* if ( callback != NULL ) */

/*--------------------------------------------------------------------*/
/*                          Handle REQUEST                            */
/*--------------------------------------------------------------------*/

   if ( request != NULL )
   {
      if (equal(strlwr(request),"no"))
         anchor->request = FALSE;
      else if (equal(request,"yes"))
         anchor->request = TRUE;
      else {
         printmsg(0,"InitEntry: Invalid value in %s, REQUEST=%s",
                     fname, request );
         success = FALSE;
      } /* else */

   } /* if ( request != NULL ) */

/*--------------------------------------------------------------------*/
/*                          Handle SENDFILES                          */
/*--------------------------------------------------------------------*/

   if ( sendfiles != NULL)
   {
      if (equal(strlwr(sendfiles),"call"))
         anchor->sendfiles = FALSE;
      else if (equal(sendfiles,"yes"))
         anchor->sendfiles = TRUE;
      else {
         printmsg(0,"InitEntry: Invalid value in %s, SENDFILES=%s",
                     fname, sendfiles );
         success = FALSE;
      } /* else */
   } /* if */

/*--------------------------------------------------------------------*/
/*                          handle commands                           */
/*--------------------------------------------------------------------*/

   if ( anchor->commands == NULL )
      anchor->commands = command_list;

/*--------------------------------------------------------------------*/
/*                 Handle local system name aliasing                  */
/*--------------------------------------------------------------------*/

   if (myname == NULL)
      anchor->myname = E_nodename;
   else
      anchor->myname = myname;

/*--------------------------------------------------------------------*/
/*                      Directory processing                          */
/*--------------------------------------------------------------------*/

   anchor->dirlist = malloc( sizeof anchor->dirlist[0] * max_elements );
   checkref( anchor->dirlist );

   max_elements = InitDir( read,    ALLOW_READ,  TRUE,  anchor,
            max_elements );
   free( read );
   max_elements = InitDir( noread,  ALLOW_READ,  FALSE, anchor,
            max_elements );
   free( noread );
   max_elements = InitDir( write,   ALLOW_WRITE, TRUE,  anchor,
            max_elements );
   free( write );
   max_elements = InitDir( nowrite, ALLOW_WRITE, FALSE, anchor,
                           max_elements );
   free( nowrite );

/*--------------------------------------------------------------------*/
/*                 Provide a default public directory                 */
/*--------------------------------------------------------------------*/

   if (xpubdir == NULL)
       anchor->pubdir = E_pubdir;
   else
       anchor->pubdir = xpubdir;

/*--------------------------------------------------------------------*/
/*    If no explicit directories given, give them access to pubdir    */
/*--------------------------------------------------------------------*/

   if ( anchor->dirsize == 0)
   {
      max_elements = InitDir( anchor->pubdir, ALLOW_READ, TRUE,
                              anchor, max_elements );
      max_elements = InitDir( anchor->pubdir, ALLOW_WRITE, TRUE,
                              anchor, max_elements );
   }

   if ( max_elements == 0 )
      success = FALSE;
   else {
      size_t subscript;
      anchor->dirlist = realloc( anchor->dirlist,
                                 anchor->dirsize * sizeof anchor->dirlist[0]);
      checkref( anchor->dirlist );
      qsort(anchor->dirlist, anchor->dirsize, sizeof(anchor->dirlist[0]),
               dircmp);
      if ( debuglevel > 4 )
      for ( subscript = 0; subscript < anchor->dirsize; subscript++ )
         printmsg(4, "InitEntry: dirlist[%d] %s\t%s\t%s",
                  subscript,
                  anchor->dirlist[subscript].grant ? "grant" : "deny" ,
                  anchor->dirlist[subscript].priv == ALLOW_WRITE ?
                           "WRITE" : "READ" ,
                  anchor->dirlist[subscript].path );
   } /* else */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return success;

} /* InitEntry */

/*--------------------------------------------------------------------*/
/*    I n i t D i r                                                   */
/*                                                                    */
/*    Initialize security table directory entries                     */
/*--------------------------------------------------------------------*/

static size_t InitDir( char *directories,
         const REMOTE_ACCESS access,
         const boolean grant,
         struct HostSecurity *anchor,
         size_t max_elements )
{
   char *field = directories;
   char *token = directories;
   struct  stat    statbuf;
   size_t subscript;

/*--------------------------------------------------------------------*/
/*    Don't process data if no input or we previously had an error    */
/*--------------------------------------------------------------------*/

   if ( (directories == NULL ) || ( max_elements == 0) )
      return max_elements;

/*--------------------------------------------------------------------*/
/*              Begin loop to process names in the path               */
/*--------------------------------------------------------------------*/

   while ( (token = NextField( field )) != NULL)
   {
      char path[FILENAME_MAX];
      if ( anchor->dirsize == max_elements )
      {
         max_elements = max_elements * 2;
         anchor->dirlist = realloc( anchor->dirlist,
                sizeof anchor->dirlist[0] * max_elements );
         checkref( anchor->dirlist );
      }

/*--------------------------------------------------------------------*/
/*                      Normalize directory name                      */
/*--------------------------------------------------------------------*/

      strcpy( path, token);
      if (isalpha(path[0]) && (path[1] != ':') && (strlen(path) == 2))
         ;                 /* Yup, do nothing for root drive names  */
      else if ( expand_path( path, NULL, E_pubdir , NULL) == NULL )
      {
         printmsg(0, "Unable to expand path \"%s\"",path );
         return 0;
      } /* else */

      printmsg(5,"InitDir: Normalizing path %s", path );
      field = newstr( normalize( path ));

/*--------------------------------------------------------------------*/
/*               Verify it really is a valid directory                */
/*--------------------------------------------------------------------*/

      if ( strlen( field ) > 2 ) /* More than just drive/colon? (x:) */
      {                       /* Yes --> Go check disk for path      */
         if (stat(field , &statbuf) != 0)
         {
            printmsg(0,"Warning ... invalid PERMISSIONS file entry %s:",
                       token );
            printerr(field);
         }
         else if ((statbuf.st_mode & S_IFDIR) == 0)
            printmsg(0,"InitDir: \"%s\" is a file, not a directory",
                        field);
      } /* if ( strlen( field ) > 2 ) */

      strlwr( field);            // Lower case for compares

/*--------------------------------------------------------------------*/
/*           Verify this directory not already in the list            */
/*--------------------------------------------------------------------*/

      for (subscript = 0; subscript < anchor->dirsize ; subscript++)
      {
         if ( (access == anchor->dirlist[subscript].priv) &&
              equal( field, anchor->dirlist[subscript].path))
         {
            printmsg(0,"InitDir: Duplicate directory %s/", field);
            return 0;
         } /* if */
      } /* for */

/*--------------------------------------------------------------------*/
/*            No conflict, add this directory to the list             */
/*--------------------------------------------------------------------*/

      printmsg(10,"InitDir: Adding \"%s\" as \"%s\"", token , field);
      anchor->dirlist[subscript].path  = field;
      anchor->dirlist[subscript].priv  = access;
      anchor->dirlist[subscript].grant = grant;
      anchor->dirsize++;

      field = NULL;           /* Look at next field next pass        */

   } /* while ( (field = NextField( field )) != NULL) */

/*--------------------------------------------------------------------*/
/*                          Return to caller                          */
/*--------------------------------------------------------------------*/

   return max_elements;
} /* InitDir */

/*--------------------------------------------------------------------*/
/*    d i r c m p                                                     */
/*                                                                    */
/*                                                                    */
/*    Compares two directory structures for sorting                   */
/*--------------------------------------------------------------------*/

int dircmp( const void *a , const void *b )
{
   struct DIRLIST *x = (struct DIRLIST*) a;
   struct DIRLIST *y = (struct DIRLIST*) b;

   int result = strcmp(x->path, y->path);

   if (result == 0 && (x->priv != y->priv))
      result = ( x->priv < y->priv ) ? -1 : 1;

   return result;
}  /*dircmp*/

/*--------------------------------------------------------------------*/
/*    V a l i d a t e H o s t                                         */
/*                                                                    */
/*    Determine that a host is allowed for a specific login           */
/*--------------------------------------------------------------------*/

boolean ValidateHost( const char *host )
{
   char **target;

/*--------------------------------------------------------------------*/
/*      If this host has no security profile, reject the access       */
/*--------------------------------------------------------------------*/

   if ( securep == NULL )
      return FALSE;

/*--------------------------------------------------------------------*/
/*    If we allow any host on this user id, use it if the calling     */
/*    host is not supported any other profile                         */
/*--------------------------------------------------------------------*/

   target = securep->validate;
   if ( target == NULL )      /* No validate list for this user?     */
   {                          /* Correct --> Use if none for host    */
      struct HostTable *hostp = checkreal( host );
      if ( hostp == BADHOST ) /* Host exist?                         */
         panic();             /* No --> Internal error, abort        */

      return hostp->anylogin; /* Allow action if generic access
                                 allowed for host                    */
   }  /* if ( target == NULL ) */

/*--------------------------------------------------------------------*/
/*          Determine if this host is allowed for this login          */
/*--------------------------------------------------------------------*/

   while (*target != NULL)
   {
      if ( equal(*target++, host ))
         return TRUE;
   } /* (*target != NULL) */

/*--------------------------------------------------------------------*/
/*                 We didn't find the host; reject it                 */
/*--------------------------------------------------------------------*/

   return FALSE;

} /* ValidateHost */

/*--------------------------------------------------------------------*/
/*    V a l i d a t e F i l e                                         */
/*                                                                    */
/*    Allow or reject access to a file by name                        */
/*--------------------------------------------------------------------*/

boolean ValidateFile( const char *input,  /* Full path name          */
                      const REMOTE_ACCESS needed )
{
   char path[FILENAME_MAX];
   char *column;

/*--------------------------------------------------------------------*/
/*                  Validate the length of the name                   */
/*--------------------------------------------------------------------*/

   printmsg(5,"ValidateFile: Checking %s access for file \"%s\"",
            (needed == ALLOW_WRITE) ? "WRITE" : "READ" , input);

   if ( strlen( input ) >= sizeof path)   /* Reject all invalid names*/
   {
      printmsg(0,"ValidateFile: Access rejected, name too long: %s",
                 input);
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*     Validate format of name; we don't allow parent directories     */
/*--------------------------------------------------------------------*/

   if ( strstr( input, "..") )            /* Games with parent dir?  */
   {
      printmsg(0,"ValidateFile: Access rejected, name not normalized: %s",
                 input);
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                Validate the security table is okay                 */
/*--------------------------------------------------------------------*/

   if ( securep == NULL )
      panic();

/*--------------------------------------------------------------------*/
/*                        Handle local system                         */
/*--------------------------------------------------------------------*/

   if ( securep->local )      /* Local system?                       */
      return TRUE;            /* Yes --> Bless the request           */

/*--------------------------------------------------------------------*/
/*       Determine if the user is allowed to request files            */
/*--------------------------------------------------------------------*/

   if ((needed == ALLOW_READ) && !securep->request)
   {
      printmsg(0,"ValidateFile: access rejected, "
                 "REQUEST not enabled in permissions file");
      return FALSE;
   }

/*--------------------------------------------------------------------*/
/*                           Copy path name                           */
/*--------------------------------------------------------------------*/

   if ( input[1] == ':' )
      strcpy( path, input );
   else
      strcat( strcpy( path , drive ), input );
   strlwr( path );

/*--------------------------------------------------------------------*/
/*              Locate the best file match for the path               */
/*--------------------------------------------------------------------*/

   while( (column = strrchr( path, '/')) != NULL )
   {
      int lower = 0;
      int upper = securep->dirsize - 1;

      *column = '\0';
      printmsg(10,"ValidateFile: Searching for %s", path);

      while( lower <= upper )
      {
         int midpoint = (lower + upper) / 2;
         int hit = strcmp(path, securep->dirlist[midpoint].path);

         printmsg(10,"ValidateFile: Comparing %s and %s",
                        path, securep->dirlist[midpoint].path);

         if ( hit == 0 )
            hit = (int) needed - (int) securep->dirlist[midpoint].priv;

         if (hit > 0)
            lower = midpoint + 1;
         else if (hit < 0)
            upper = midpoint - 1;
         else {
            printmsg( securep->dirlist[midpoint].grant ? 5 : 0 ,
                     "ValidateFile: Found path \"%s\", access %s to \"%s\"",
                     securep->dirlist[midpoint].path,
                     securep->dirlist[midpoint].grant ?
                                    "granted" : "denied", input);
            return securep->dirlist[midpoint].grant;
         }
      } /* while( lower <= upper ) */
   } /* while( (column = strrchr( path, '/')) != NULL ) */

/*--------------------------------------------------------------------*/
/*          We didn't find the file; reject all access to it          */
/*--------------------------------------------------------------------*/

   printmsg(0,"ValidateFile: No access definition found for \
\"%s\", access denied",
            input);
   return FALSE;

} /* ValidateFile */


/*--------------------------------------------------------------------*/
/*    G e t S e c u r i t y                                           */
/*                                                                    */
/*    Return security structure for to use when calling out to        */
/*    another system                                                  */
/*--------------------------------------------------------------------*/

struct HostSecurity *GetSecurity( struct HostTable *hostp)
{
   if ((hostp->hsecure == NULL) && (default_security != NULL ))
   {
      printmsg(2,"GetSecurity: Using security for MACHINE=OTHER for \
system \"%s\"", hostp->hostname );
      hostp->hsecure = default_security;
   } /* if  */

   return hostp->hsecure;

} /* GetSecurity */
