/*--------------------------------------------------------------------*/
/*    i m p o r t . c                                                 */
/*                                                                    */
/*    File name mapping routines for UUPC/extended                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*
 *    $Id: import.c 1.11 1993/10/12 00:48:44 ahd Exp $
 *
 *    $Log: import.c $
 *     Revision 1.11  1993/10/12  00:48:44  ahd
 *     Normalize comments
 *
 *     Revision 1.10  1993/10/09  15:46:15  rhg
 *     ANSIify the source
 *
 *     Revision 1.9  1993/09/27  04:04:06  ahd
 *     Correct creation of pointer to file system name
 *
 *     Revision 1.8  1993/09/26  03:32:27  dmwatt
 *     Use Standard Windows NT error message module
 *
 *     Revision 1.7  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.6  1993/09/03  12:54:55  ahd
 *     Add missing endif
 *
 *     Revision 1.5  1993/09/03  12:18:55  dmwatt
 *     Windows NT support for long names on file systems
 *
 *     Revision 1.4  1993/09/02  12:08:17  ahd
 *     HPFS Support
 *
 *     Revision 1.3  1993/04/11  00:31:31  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/11/22  21:06:14  ahd
 * Correct mapping of dos paths with trailing slashes
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(FAMILYAPI) || defined(__OS2__)
#define INCL_NOPM             /* No need to include OS/2 PM info */
#define INCL_BASE
#include <os2.h>
#elif defined(WIN32)
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "import.h"
#include "arbmath.h"
#include "hostable.h"
#include "usertabl.h"
#include "security.h"

#ifdef WIN32
#include "pnterr.h"
#endif

#define MAX_DIGITS 20         /* Number of digits for arb math */

/*--------------------------------------------------------------------*/
/*                    Internal function prototypes                    */
/*--------------------------------------------------------------------*/

#define min(x,y) (((x) < (y)) ? (x) : (y))

currentfile();

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static void ImportName( char *local,
                        const char *canon,
                        size_t charsetsize,
                        const boolean longname );

static boolean advancedFS( const char *path );

/*--------------------------------------------------------------------*/
/*                                                                    */
/*   i m p o r t p a t h                                              */
/*                                                                    */
/*   Convert a canonical name to a format the host can handle         */
/*                                                                    */
/*   These routines convert file name between canonical form, which   */
/*   is defined as a 'unix' style pathname, and the MS-DOS all        */
/*   uppercase "xxxxxxxx.xxx" format.                                 */
/*                                                                    */
/*   If the canonical name does not have a path, that is the file is  */
/*   destined for the local spool directory, we can assume the UNIX   */
/*   name will normally be in a format like this:                     */
/*                                                                    */
/*                                                                    */
/*       X.hostid#######            (Execute files)                   */
/*       C.hostid#######            (Call files)                      */
/*       D.hostid#######            (Data files)                      */
/*                                                                    */
/*   where "hostid" may be most, but not always all, of the local     */
/*   host or remote host (the file came from or is going to) and      */
/*   "######" can be any character valid for the UNIX file system.    */
/*   Note, however, that the routine has to be generic to allow for   */
/*   other file names to be placed in the spool directory without     */
/*   collisions.                                                      */
/*                                                                    */
/*   Avoiding collisions in the spool directory is important; when    */
/*   receiving files with mixed case names longer than 11             */
/*   characters, sooner or later a file name collision will occur.    */
/*                                                                    */
/*   We can also assume that only UUPC will see these names, which    */
/*   means we can transform the name using any method we choose, so   */
/*   long as the UUPC functions opening the file always call          */
/*   importpath, and that importpath is reducible (that is, two       */
/*   calls to importpath with the same argument always yield the      */
/*   same result).  Note that if end user really wanted the file in   */
/*   the spool directory, all he has to do is rename the file-- far   */
/*   better than losing the data because duplicate file names.        */
/*                                                                    */
/*   For these files, we map the name as follows:                     */
/*                                                                    */
/*   0 - If the name is a valid MS-DOS name, use it without changing  */
/*                                                                    */
/*   1 - Begin the output name by inserting up to the first eight     */
/*       characters of the remote host name (followed by a slash) as  */
/*       a subdirectory name.                                         */
/*                                                                    */
/*   2 - If the input name begins with an uppercase alphabetic        */
/*       character followed by a period, also insert the alphabetic   */
/*       (followed by a slash) to make this a second subdirectory.    */
/*       Then, move the logical start of the input name past the two  */
/*       characters.                                                  */
/*                                                                    */
/*   3 - Determine the number of characters the local host and        */
/*       remote hosts have equal to the next characters of the input  */
/*       name, up to a maximum of 8, and zero the lower of the two    */
/*       counts.  Then, step past the number of characters of the     */
/*       larger count.                                                */
/*                                                                    */
/*       For example, if the file name is X.keane22222 and the local  */
/*       host name is kendra (2 characters match) and the remote      */
/*       host is keane1 (5 characters match), zero the number of      */
/*       characters matched by kendra, and make the new start of the  */
/*       file name five characters further (at the first "2").        */
/*                                                                    */
/*   4 - Convert the remaining string using a base conversion, with   */
/*       the input character size being from ascii "#" to ascii "z"   */
/*       (88 characters) to the allowed set of characters in MS-DOS   */
/*       file names (charset, below, 52 characters).                  */
/*                                                                    */
/*   5 - Prepend to the string to be converted the length of the      */
/*       remote host added to the length of the local host            */
/*       multiplied by 8 (both lengths were computed in step 3,       */
/*       above).  The base conversion is also applied to this         */
/*       "character", we which know will be in the range 1-64.        */
/*                                                                    */
/*   6 - If the string created by steps 4 and 5 exceeds 8             */
/*       characters, insert a period after the eighth character to    */
/*       make it a valid MS-DOS file name.  If the string created by  */
/*       steps 4 and 5 exceeds 11 characters, truncate the string by  */
/*       using the first eight and last three characters.             */
/*                                                                    */
/*   7 - Append the string created in steps 4 through 6 to the path   */
/*       name created in steps 1 and 2.                               */
/*                                                                    */
/*   If the canonical name has a path, it is destined for an end      */
/*   user, so we should not radically transform it like we do for     */
/*   files in the spool directory.  Thus, if the canonical name has   */
/*   a path, mung the canonical file name as follows:                 */
/*                                                                    */
/*   1 - skip any path from the canonical name                        */
/*                                                                    */
/*   2 - copy up to 8 character from the canonical name converting .  */
/*       to _ and uppercase to lowercase.                             */
/*                                                                    */
/*   3 - if the name was longer than 8 character copy a . to the      */
/*       host name and then copy the up to three characters from      */
/*       the tail of the canonical name to the host name.             */
/*                                                                    */
/*   Note that this set of rules will cause a collision with names    */
/*   that only differ in case, but leaves the name in a recongizable  */
/*   format for the user.                                             */
/*--------------------------------------------------------------------*/

void importpath(char *local, const char *canon, const char *remote)
{
   char *s, *out;
   size_t charsetsize;     /* Number of allowed characters in
                              MS-DOS file names                   */

   out = local;

/*--------------------------------------------------------------------*/
/*                       Verify our parameters                        */
/*--------------------------------------------------------------------*/

   if ( local == NULL )
      panic();

   if ( canon == NULL )
      panic();

/*--------------------------------------------------------------------*/
/*                      Define our character set                      */
/*--------------------------------------------------------------------*/

    if ( E_charset == NULL )
       E_charset = DOSCHARS;

    charsetsize = strlen( E_charset );

/*--------------------------------------------------------------------*/
/*                 Determine if spool file directory                  */
/*--------------------------------------------------------------------*/

   if ((s = strrchr(canon, '/')) == NULL)
   {                          /* File for spooling directory, use
                                 internal character set to avoid
                                 collisions                           */
      static size_t range =  UNIX_END_C - UNIX_START_C + 1;
                              /* Determine unique number characters in
                                 the UNIX file names we are mapping   */

      size_t remlen = min(HOSTLEN, strlen(remote));
                              /* Length of the remote name passed
                                 in, shortened below to number of
                                 characters matched in name           */
      size_t nodelen = min(HOSTLEN, strlen(E_nodename));
                              /* Length of the local host name,
                                 shortened below to number of
                                 characters matched in name           */
      size_t subscript = 0;   /* Value of UNIX character to be
                                 converted to MS-DOS character set    */
      char *next        = local + remlen;
      char tempname[FILENAME_MAX];
      unsigned char number[MAX_DIGITS];
                              /* Arbitary length number, for base
                                 conversions                        */

      boolean longname;

      printmsg(4,"importpath: Checking File system for spool directory %s",
                  E_spooldir );
      longname = advancedFS( E_spooldir ) && bflag[B_LONGNAME];

/*--------------------------------------------------------------------*/
/*                    Verify we have a remote name                    */
/*--------------------------------------------------------------------*/

      if ( remote == NULL )
         panic();

/*--------------------------------------------------------------------*/
/*    Put the host name (up to six characters) at the beginning of    */
/*    the MS-DOS file name as a sub-directory name.                   */
/*--------------------------------------------------------------------*/

      strncpy(local, remote, remlen);
      *next++ = '/';          /* Add in the sub-directory seperator   */
      s = (char *) canon;     /* Get the beginnging of the UNIX name  */

/*--------------------------------------------------------------------*/
/*    Files in the spooling directory generally start with "D.",      */
/*    "C.", or "X."; strip off any upper case letter followed by a    */
/*    period into its own directory.                                  */
/*--------------------------------------------------------------------*/

      if ((s[0] >= 'A') && (s[0] <= 'Z') && (s[1] == '.'))
      {
         *next++ = *s;        /* Copy the input character             */
         *next++ = '/';       /* Add the sub-directory indicator too  */
         s += 2;              /* Step input string past the copied
                                 data                                 */
      }

      while( remlen > 0 )
      {
         if (equaln(remote,s,remlen))
            break;
         remlen--;
      }

      while( nodelen > 0 )
      {
         if (equaln(E_nodename,s,nodelen))
            break;
         nodelen--;
      }

      if (nodelen > remlen )
      {
         remlen = 0;
         s += nodelen;
      }
      else {
         nodelen = 0;
         s += remlen;
      }

      *next  = '\0';          /* Terminate first part of host string  */

/*--------------------------------------------------------------------*/
/*       Create a binary number which represents our file name        */
/*--------------------------------------------------------------------*/

      for (subscript = 0; subscript < MAX_DIGITS; subscript++ )
         number[subscript] = 0;  /* Initialize number to zero         */

      add(number, nodelen + remlen * HOSTLEN, MAX_DIGITS);
                                 /* Append host name info to the
                                    front of the converted string     */

      while( (*s != '\0') && (*number == '\0'))
      {
         mult(number, range, MAX_DIGITS); /* Shift the number over    */
         add(number, *s++  - UNIX_START_C , MAX_DIGITS);
                                          /* Add in new low order     */
      } /* while */

/*--------------------------------------------------------------------*/
/*   We now have stripped off the leading x. and host name, if any;   */
/*   now, convert the remaining characters in the name by doing a     */
/*   range to charset base conversion.                                */
/*--------------------------------------------------------------------*/

      out = &tempname[FILENAME_MAX];
      *--out = '\0';          /* Terminate the string we will build   */

/*--------------------------------------------------------------------*/
/*         Here's the loop to actually do the base conversion         */
/*--------------------------------------------------------------------*/

      while(adiv( number, charsetsize, &subscript, MAX_DIGITS))
            *--out = E_charset[ subscript ];

/*--------------------------------------------------------------------*/
/*    The conversion is done; now squeeze it into an 11 character     */
/*    MS-DOS name with period.                                        */
/*--------------------------------------------------------------------*/

      ImportName( next, out, charsetsize, longname );

   }
   else {         /* Not file for spooling directory, convert it  */

      char *in = (char *) canon;
      boolean longname ;

      printmsg(4,"importpath: Checking file system for file %s",
                  canon );
      longname = advancedFS( canon );

      if ( ValidDOSName( canon, longname ))
      {
         strcpy( local, canon );
         return;
      }

/*--------------------------------------------------------------------*/
/*      Handle leading drive letter (ignore it, assuming valid)       */
/*--------------------------------------------------------------------*/

      if ( isalpha( *in ) && (in[1] == ':'))
      {
         *out++ = *in++;      /* The drive letter                     */
         *out++ = *in++;      /* The colon making it a driver letter  */
      } /* if */

      if ( *in == '/' )       /* Absolute path name?                  */
         *out++ = *in++;      /* Yes, step past it                    */

      while( *in == '/')      /* Additional slashes?                  */
         in++;                /* Skip them,  they mean nothing        */

      s = strchr( in, '/' );  /* Get end of next path segment         */

/*--------------------------------------------------------------------*/
/*              Now convert each simple name in the path              */
/*--------------------------------------------------------------------*/

      while ( *in )
      {
         if ( s != NULL )
            *s = '\0';        /* Truncate input string to simple name */

         ImportName( out, in , charsetsize, longname );

         if ( s == NULL )
            break;
         out = out + strlen( out );
         *out++ = *s++ = '/'; /* Restore path to input and output     */
         in = s;              /* Remember start of this simple name   */
         while( *in == '/')   /* Additional slashes?                  */
            in++;             /* Skip them,  they mean nothing        */
         s = strchr( in , '/' );
      }

   } /* else */

   printmsg( 3, "ImportPath: Mapped %s to %s", canon, local );

} /*importpath*/

/*--------------------------------------------------------------------*/
/*    I m p o r t N a m e                                             */
/*                                                                    */
/*    Translate a simple DOS name without the path                    */
/*--------------------------------------------------------------------*/

static void ImportName( char *local,
                        const char *canon,
                        size_t charsetsize,
                        const boolean longname )
{

   char *in = (char *) canon;
   char *out = local;
   size_t len = strlen( canon );
   size_t column;
   char *best_period = NULL;     /* Assume no prince charming         */

   if ( strchr(canon,'/') != NULL )
   {
      printmsg(0,"ImportName: Parameter error, not simple name: %s",
            canon);
      panic();
   }

   if ( len == 0 )
   {
      printmsg(0,"ImportName: Parameter error, zero length input");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 If a valid DOS name, use it as-is                  */
/*--------------------------------------------------------------------*/

   if (ValidDOSName( canon, longname ))
   {
      strcpy( local, canon );
      return;
   }

/*--------------------------------------------------------------------*/
/*    If the dataset name has a period, use it.  The rule we          */
/*    follow is use the last period in the second through ninth       */
/*    characters, otherwise use the last period in the dataset        */
/*    name with the exception of leading period.                      */
/*                                                                    */
/*    In any case, we only copy up to eight characters for the        */
/*    dataset name and up to three characters for the extension.      */
/*--------------------------------------------------------------------*/

   for ( column = 1; (column < 9) && (in[column] != '\0') ; column++)
   {
      if ( in[column] == '.')
      {
         strncpy( out, in, column + 5 );
                                    /* Period, 3 char extension,
                                       and terminating \0             */
         best_period = &out[column];/* Remember output location of
                                       period in name                 */

         if ( len > (column + 4) )  /* Need to trunc extension to 3?  */
            strcpy( out + column + 1, in + len - 3 ); /* Yes          */

         break;
      } /*if */
   }  /* if */

/*--------------------------------------------------------------------*/
/*    No period in the first eight characters, search the rest of     */
/*    the name for the last period (unless period is very last        */
/*    character in the string).                                       */
/*--------------------------------------------------------------------*/

   if ( best_period == NULL )
   {

      strncpy( out , in , 8);
      best_period = strrchr( in+1 , '.');

      if ( (best_period != NULL) && (best_period[1] != '\0') )
      {
         strncpy( &out[8], best_period, 4 ); /* Plus period and 3
                                                in extension          */

         if ( strlen( best_period) > 4 )     /* Long Extension?       */
            out[12] = '\0';                  /* Yes --> Truncate      */

      } /* if */
      else {                  /* No periods at all, generate one
                                 if needed for long name          */

         if ( len > 8 )
         {
            out[8] = '.';
            strcpy(&out[9], in + max(8,(len - 3))  );
         } /* if ( len > 9 ) */

      } /* else */

      best_period = &out[8];              /* Remember location of
                                             period, okay if past
                                             end of string            */

   } /* if ( best_period == NULL ) */

/*--------------------------------------------------------------------*/
/*                Now, clean up any invalid characters                */
/*--------------------------------------------------------------------*/

   if ( out[ strlen( out ) - 1 ] == '.' ) /* Trailing period?         */
      out[ strlen( out ) - 1 ] = '\0';    /* Just truncate string     */

   while( *out != '\0')
   {
      int c ;
      if ( isupper( *out ))
         c = tolower( *out );
      else
         c = *out;

      if ((out != best_period) && (strchr( E_charset, c ) == NULL ))
      {
         if ( c > 'z' )
            c -= 62;
         else if ( c > 'Z' )
            c -= 36;
         else if ( c > '9' )
            c -= 10;
         *out = E_charset[ (c - UNIX_START_C) % charsetsize ];
      }

      out++;                    /* Step to next character         */
   } /* while( *out != '\0') */

/*--------------------------------------------------------------------*/
/*                   Report our results and return                    */
/*--------------------------------------------------------------------*/

   printmsg( 5,
            "ImportName: Mapped %s to %s", canon, local );

} /* ImportName */

/*--------------------------------------------------------------------*/
/*    V a l i d D O S N a m e                                         */
/*                                                                    */
/*    Validate an MS-DOS file name                                    */
/*--------------------------------------------------------------------*/

boolean ValidDOSName( const char *s,
                      const boolean longname )
{
   char *ptr;
   size_t len = strlen ( s );
   char tempname[FILENAME_MAX];

   static char *longCharSet = NULL;

/*--------------------------------------------------------------------*/
/*                      Define our character set                      */
/*--------------------------------------------------------------------*/

   if ( E_charset == NULL )
      E_charset = DOSCHARS;

   if ( longname )
   {

#if defined(FAMILYAPI) || defined(__OS2__)

/*--------------------------------------------------------------------*/
/*       Ask OS/2 if the file name is okay.  Because the invoked      */
/*       function accepts wildcards, we pre-test for them and reject  */
/*       them as needed.                                              */
/*--------------------------------------------------------------------*/

      if ((strchr( s, '*') == NULL ) && (strchr( s, '?') == NULL))
      {

#ifdef __OS2__
         APIRET result = DosQPathInfo( (PSZ) s,
                                       FIL_QUERYFULLNAME,
                                       (PVOID) tempname,
                                       sizeof tempname );
#else
         USHORT result = DosQPathInfo( (PSZ) s,
                                       FIL_NAMEISVALID,
                                       (PBYTE) tempname,
                                       sizeof tempname,
                                       0 );

#endif
         if ( result == 0 )
            return TRUE;

         printmsg(2,
                  "ValidDOSName: Invalid name %s, syntax error code %d",
                   s,
                   (int) result);

      } /* if */

#endif

      if ( longCharSet == NULL )
      {
         *tempname = '.';
         longCharSet = newstr(strcpy( tempname + 1, E_charset ));
      }

      if (strspn(s, longCharSet) == len)
      {
         printmsg(9,"ValidDOSName: \"%s\" is valid long name", s);
         return TRUE;
      }

   } /* if ( longname ) */

/*--------------------------------------------------------------------*/
/*                 Name must be 12 characters or less                 */
/*--------------------------------------------------------------------*/

   if (len > 12)
      return FALSE;

   strcpy( tempname, s);      /* Make a temp copy we can alter        */

/*--------------------------------------------------------------------*/
/*    Simple file name without extension must be eight characters     */
/*    or less                                                         */
/*--------------------------------------------------------------------*/

   ptr = strrchr(tempname, '.');
   if (ptr == NULL)
   {
      if (len > 8)
         return FALSE;
   }

/*--------------------------------------------------------------------*/
/*          Period must be in second through ninth character          */
/*--------------------------------------------------------------------*/

   else {
      if ((ptr == tempname) || (ptr > &tempname[8]))
         return FALSE;

/*--------------------------------------------------------------------*/
/*             Extension must be three characters or less             */
/*--------------------------------------------------------------------*/

      if ( strlen( ptr ) > 4) /* Three characters plus the period?    */
         return FALSE;        /* No --> Too much                      */

/*--------------------------------------------------------------------*/
/*                          Only one period                           */
/*--------------------------------------------------------------------*/

      if (ptr != strchr(tempname, '.'))
         return FALSE;
   } /* else */

/*--------------------------------------------------------------------*/
/*                Must only be valid MS-DOS characters                */
/*--------------------------------------------------------------------*/

   strlwr( tempname );        /* Map into our desired character set   */
   if ( ptr != NULL )
      *ptr = 'x';             /* We've already accounted for the
                                 period, don't let it ruin our day    */

   if (strspn(tempname, E_charset ) == len)
   {
      printmsg(9,"ValidDOSName: \"%s\" is valid", s);
      return TRUE;
   }
   else
      return FALSE;

} /* ValidateDOSName */

#if defined(FAMILYAPI) || defined( __OS2__ )

/*--------------------------------------------------------------------*/
/*       a d v a n c e d F S                       (OS/2 version)     */
/*                                                                    */
/*       Determine if a file system is advanced (supports better than */
/*       8.3 file names)                                              */
/*--------------------------------------------------------------------*/

static boolean advancedFS( const char *path )
{
   char buf[BUFSIZ];             /* One generic large buffer          */

#ifdef __OS2__
   ULONG bufSize = sizeof buf;
   FSQBUFFER2 *dataBuffer = (FSQBUFFER2 *) buf;
   ULONG  result;
#else
   SHORT bufSize = sizeof buf;
   FSQBUFFER *dataBuffer = (FSQBUFFER *) buf;
   USHORT result;
#endif

/*--------------------------------------------------------------------*/
/*                  Get the drive letter to process                   */
/*--------------------------------------------------------------------*/

   char driveInfo[3];
   char *fileSystem;

   if ( isalpha( *path ) && (path[1] == ':') )
      strncpy( driveInfo, path, 2 );
   else
      strncpy( driveInfo, E_cwd, 2 );

   driveInfo[ sizeof(driveInfo) - 1 ] = '\0';   /* Terminate string data */

/*--------------------------------------------------------------------*/
/*      Query the drive (both 1.x and 2.x calls are supported).       */
/*--------------------------------------------------------------------*/

#ifdef __OS2__
   result = DosQueryFSAttach( (PSZ) driveInfo,
                          1,
                          FSAIL_QUERYNAME,
                          dataBuffer,
                          &bufSize );
   fileSystem = (char *) (dataBuffer->szFSDName + dataBuffer->cbName);
#else
   result = DosQFSAttach( driveInfo,
                          0,
                          FSAIL_QUERYNAME,
                          (PBYTE) buf,
                          &bufSize,
                          0L );
   fileSystem = (char *) (dataBuffer->szFSDName + dataBuffer->cbName - 1);
#endif

   if ( result != 0 )
   {
      printmsg(0, "advancedFS: Unable to query file system for %s, error = %d",
                  driveInfo,
                  (int)  result );
      return FALSE;
   }


   printmsg(4,"advancedFS: File system %d, name \"%s\", FS name \"%s\"",
               (int) dataBuffer->iType,
               dataBuffer->szName,
               fileSystem );

   if (equal( fileSystem, "FAT"))
      return FALSE;
   else
      return TRUE;

} /* advancedFS */

#elif WIN32

/*--------------------------------------------------------------------*/
/*       a d v a n c e d F S                    (Window NT version)   */
/*                                                                    */
/*       Determine if a file system is advanced (supports better than */
/*       8.3 file names)                                              */
/*--------------------------------------------------------------------*/

static boolean advancedFS( const char *path )
{
   char driveInfo[4];
   char fsType[5];
   BOOL result;
   char *shareNameEnd;

   if ( !path || *path == '\0' ) {       /* use CWD                   */
      strncpy( driveInfo, E_cwd, 3);
      driveInfo[3] = '\0';
   }
   else if ( isalpha( *path ) && (path[1] == ':') )
   {                                   /* It's a local drive          */

      printmsg(5, "advancedFS: it's a drive letter");
      strncpy( driveInfo, path, 3 );
      driveInfo[3] = '\0';          /* Terminate drive string data    */

   }
   else
      return FALSE;

/*--------------------------------------------------------------------*/
/*            We've got the drive letter, query its status            */
/*--------------------------------------------------------------------*/

   result = GetVolumeInformation(driveInfo, NULL, 0, NULL, NULL,
         NULL, fsType, 5);

   if ( !result )
   {
      DWORD dwError = GetLastError();
      printmsg(0, "advancedFS: Unable to query file system for %s", driveInfo);
      printNTerror("GetVolumeInformation", dwError);
      panic();
   }

   printmsg(4,"advancedFS: File system for \"%s\" has name \"%s\"",
               driveInfo,
               fsType );

   return strcmp( fsType, "FAT");

} /* advancedFS for WIN32 */

#else

/*--------------------------------------------------------------------*/
/*       a d v a n c e d F S                          (DOS version)   */
/*                                                                    */
/*       Determine if a file system is advanced (supports better than */
/*       8.3 file names)                                              */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#pragma argsused
#elif _MSC_VER >= 700
#pragma warning(disable:4100)   /* suppress unref'ed formal param. warnings */
#endif

static boolean advancedFS( const char *path )
{
   return FALSE;                 /* DOS is always dumb on file systems! */
} /* advancedFS for MS-DOS */

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

#endif
