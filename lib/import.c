/*--------------------------------------------------------------------*/
/*    i m p o r t . c                                                 */
/*                                                                    */
/*    File name mapping routines for UUPC/extended                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1997 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*
 *    $Id: import.c 1.36 1997/12/14 21:13:45 ahd v1-12u $
 *
 *    $Log: import.c $
 *    Revision 1.36  1997/12/14 21:13:45  ahd
 *    Correct corrupted buffer when short name with invalid characters
 *    is mapped.
 *    Check NT advanced file system based on name length, not FS name
 *    Cache NT advanced file system status for letter drives
 *    Never cache OS/2 UNC Names
 *    Use symbolic names for cache flags
 *    ,
 *
 *    Revision 1.35  1997/03/31 07:00:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.34  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.33  1996/01/01 20:55:48  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.32  1995/12/12 13:48:54  ahd
 *    restrict issuing importPath messages to higher verbosity levels
 *
 *    Revision 1.31  1995/09/24 19:07:05  ahd
 *    Trap invalid input names with spaces
 *
 *    Revision 1.30  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.29  1995/01/07 16:12:48  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.28  1995/01/03 05:32:26  ahd
 *    Reduce, clarify logging in OS/2 Advanced FS function
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>
#include <limits.h>

#if defined(FAMILYAPI) || defined(__OS2__)
#define INCL_NOPM             /* No need to include OS/2 PM info */
#define INCL_BASE

#include <os2.h>
#include "pos2err.h"

#elif defined(WIN32)
#include <windows.h>
#endif

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

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

#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif

#define CACHE_LONG_NAME_SUPPORT  'L'
#define CACHE_SHORT_NAME_ONLY    'S'

RCSID("$Id: import.c 1.36 1997/12/14 21:13:45 ahd v1-12u $");
currentfile();

/*--------------------------------------------------------------------*/
/*                     Local function prototypes                      */
/*--------------------------------------------------------------------*/

static void ImportName(char *local,
                        const char *canon,
                        size_t charsetsize,
                        const KWBoolean longname);

static KWBoolean advancedFS(const char *path);

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

   if (local == NULL)
      panic();

   if (canon == NULL)
      panic();

/*--------------------------------------------------------------------*/
/*                      Define our character set                      */
/*--------------------------------------------------------------------*/

    if (E_charset == NULL)
       E_charset = DOSCHARS;

    charsetsize = strlen(E_charset);

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

      KWBoolean longname;

      if (bflag[F_LONGNAME] && advancedFS(E_spooldir))
         longname = KWTrue;
      else
         longname = KWFalse;

/*--------------------------------------------------------------------*/
/*                    Verify we have a remote name                    */
/*--------------------------------------------------------------------*/

      if (remote == NULL)
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

      while (remlen > 0)
      {
         if (equaln(remote,s,remlen))
            break;
         remlen--;
      }

      while (nodelen > 0)
      {
         if (equaln(E_nodename,s,nodelen))
            break;
         nodelen--;
      }

      if (nodelen > remlen)
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

      for (subscript = 0; subscript < MAX_DIGITS; subscript++)
         number[subscript] = 0;  /* Initialize number to zero         */

      add(number, nodelen + remlen * HOSTLEN, MAX_DIGITS);
                                 /* Append host name info to the
                                    front of the converted string     */

      while ((*s != '\0') && (*number == '\0'))
      {
         if (isspace(*s))
         {
            printmsg(0,"importPath: Invalid file \"%s\" for %s, "
                       "name contains whitespace",
                       canon,
                       remote);
            panic();
         }

         mult(number, range, MAX_DIGITS); /* Shift the number over    */
         add(number, (unsigned) (*s++  - UNIX_START_C), MAX_DIGITS);
                                          /* Add in new low order     */
      } /* while */

/*--------------------------------------------------------------------*/
/*   We now have stripped off the leading x. and host name, if any;   */
/*   now, convert the remaining characters in the name by doing a     */
/*   range to charset base conversion.                                */
/*--------------------------------------------------------------------*/

      out = tempname + FILENAME_MAX;
      *--out = '\0';          /* Terminate the string we will build   */

/*--------------------------------------------------------------------*/
/*         Here's the loop to actually do the base conversion         */
/*--------------------------------------------------------------------*/

      while (adiv(number, charsetsize, &subscript, MAX_DIGITS))
            *--out = E_charset[ subscript ];

/*--------------------------------------------------------------------*/
/*    The conversion is done; now squeeze it into an 11 character     */
/*    MS-DOS name with period.                                        */
/*--------------------------------------------------------------------*/

      ImportName(next, out, charsetsize, longname);

   }
   else {         /* Not file for spooling directory, convert it  */

      char *in = (char *) canon;
      KWBoolean longname ;

      longname = advancedFS(canon);

      if (ValidDOSName(canon, longname))
      {
         strcpy(local, canon);
         return;
      }

/*--------------------------------------------------------------------*/
/*      Handle leading drive letter (ignore it, assuming valid)       */
/*--------------------------------------------------------------------*/

      if (isalpha(*in) && (in[1] == ':'))
      {
         *out++ = *in++;      /* The drive letter                     */
         *out++ = *in++;      /* The colon making it a driver letter  */
      } /* if */

      if (*in == '/')         /* Absolute path name?                  */
         *out++ = *in++;      /* Yes, step past it                    */

      while (*in == '/')       /* Additional slashes?                  */
         in++;                /* Skip them,  they mean nothing        */

      s = strchr(in, '/');    /* Get end of next path segment         */

/*--------------------------------------------------------------------*/
/*              Now convert each simple name in the path              */
/*--------------------------------------------------------------------*/

      while (*in)
      {
         if (s != NULL)
            *s = '\0';        /* Truncate input string to simple name */

         ImportName(out, in, charsetsize, longname);

         if (s == NULL)
            break;
         out = out + strlen(out);
         *out++ = *s++ = '/'; /* Restore path to input and output     */
         in = s;              /* Remember start of this simple name   */
         while (*in == '/')    /* Additional slashes?                  */
            in++;             /* Skip them,  they mean nothing        */
         s = strchr(in, '/');
      }

   } /* else */

   printmsg(equali(canon,local) ? 8 : 4,
            "ImportPath: Mapped %s to %s", canon, local);

} /*importpath*/

/*--------------------------------------------------------------------*/
/*    I m p o r t N a m e                                             */
/*                                                                    */
/*    Translate a simple DOS name without the path                    */
/*--------------------------------------------------------------------*/

static void ImportName(char *local,
                        const char *canon,
                        size_t charsetsize,
                        const KWBoolean longname)
{

   char *in = (char *) canon;
   char *out = local;
   size_t len = strlen(canon);
   size_t column;
   char *best_period = NULL;     /* Assume no prince charming         */

   if (strchr(canon,'/') != NULL)
   {
      printmsg(0,"ImportName: Parameter error, not simple name: %s",
            canon);
      panic();
   }

   if (len == 0)
   {
      printmsg(0,"ImportName: Parameter error, zero length input");
      panic();
   }

/*--------------------------------------------------------------------*/
/*                 If a valid DOS name, use it as-is                  */
/*--------------------------------------------------------------------*/

   if (ValidDOSName(canon, longname))
   {
      strcpy(local, canon);
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

   for (column = 1; (column < 9) && (in[column] != '\0') ; column++)
   {
      if (in[column] == '.')
      {
         strncpy(out, in, column + 5);
                                    /* Period, 3 char extension,
                                       and terminating \0             */
         best_period = out + column;/* Remember output location of
                                       period in name                 */

         if (len > (column + 4))    /* Need to trunc extension to 3?  */
            strcpy(out + column + 1, in + len - 3); /* Yes            */

      } /*if */

   }  /* if */

/*--------------------------------------------------------------------*/
/*    No period in the first eight characters, search the rest of     */
/*    the name for the last period (unless period is very last        */
/*    character in the string).                                       */
/*--------------------------------------------------------------------*/

   if (len < 9)
      strcpy( out, in );
   else if (best_period == NULL)
   {
      strncpy(out, in, 9);
      best_period = strrchr(in+1, '.');

      if ((best_period != NULL) && (best_period[1] != '\0'))
      {
         strncpy(out + 8, best_period, 4); /* Plus period and 3
                                                in extension          */
         out[12] = '\0';                   /* Insure terminated string*/

      } /* if */
      else {
            /* No periods at all, generate one needed for long name */
            out[8] = '.';
            strcpy(out + 9, in + max(8, (len - 3)));
      } /* else */

      best_period = out + 8;        /* Remember location of period   */

   } /* if (best_period == NULL) */


/*--------------------------------------------------------------------*/
/*                Now, clean up any invalid characters                */
/*--------------------------------------------------------------------*/

   if (out[ strlen(out) - 1 ] == '.') /* Trailing period?             */
      out[ strlen(out) - 1 ] = '\0';      /* Just truncate string     */

   while (*out != '\0')
   {
      int c ;
      if (isupper(*out))
         c = tolower(*out);
      else
         c = *out;

      if ((out != best_period) && (strchr(E_charset, c) == NULL))
      {
         if (c > 'z')
            c -= 62;
         else if (c > 'Z')
            c -= 36;
         else if (c > '9')
            c -= 10;
         *out = E_charset[ (c - UNIX_START_C) % charsetsize ];
      }

      out++;                    /* Step to next character         */
   } /* while (*out != '\0') */

/*--------------------------------------------------------------------*/
/*                   Report our results and return                    */
/*--------------------------------------------------------------------*/

   printmsg(5,
            "ImportName: Mapped %s to %s", canon, local);

} /* ImportName */

/*--------------------------------------------------------------------*/
/*    V a l i d D O S N a m e                                         */
/*                                                                    */
/*    Validate an MS-DOS file name                                    */
/*--------------------------------------------------------------------*/

KWBoolean ValidDOSName(const char *s,
                      const KWBoolean longname)
{
   char *ptr;
   size_t len = strlen (s);
   char tempName[FILENAME_MAX];

   static char *longCharSet = NULL;

/*--------------------------------------------------------------------*/
/*                      Define our character set                      */
/*--------------------------------------------------------------------*/

   if (E_charset == NULL)
      E_charset = DOSCHARS;

   if (longname)
   {
      char workBuffer[UCHAR_MAX + 1];

#if defined(FAMILYAPI) || defined(__OS2__)

/*--------------------------------------------------------------------*/
/*       Ask OS/2 if the file name is okay.  Because the invoked      */
/*       function accepts wildcards, we pre-test for them and reject  */
/*       them as needed.                                              */
/*--------------------------------------------------------------------*/

      if ((strchr(s, '*') == NULL) && (strchr(s, '?') == NULL))
      {

#ifdef __OS2__
         APIRET result = DosQPathInfo((PSZ) s,
                                       FIL_QUERYFULLNAME,
                                       (PVOID) workBuffer,
                                       sizeof workBuffer);
#else
         USHORT result = DosQPathInfo((PSZ) s,
                                       FIL_NAMEISVALID,
                                       (PBYTE) workBuffer,
                                       sizeof workBuffer,
                                       0);

#endif
         if (result == 0)
            return KWTrue;

         printmsg(2,
                  "ValidDOSName: Invalid name %s, syntax error code %d",
                   s,
                   (int) result);

      } /* if */

#endif

      if (longCharSet == NULL)
      {
         strcpy(workBuffer, E_charset);
         strcat(workBuffer, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
                                          /* Uppercase okay           */
         strcat(workBuffer, "." );        /* multiple periods allowed */
         longCharSet = newstr(workBuffer);
      }


      if (strspn(s, longCharSet) == len)
      {
         printmsg(9,"ValidDOSName: \"%s\" is valid long name", s);
         return KWTrue;
      }

      return KWFalse;

   } /* if (longname) */

/*--------------------------------------------------------------------*/
/*                 Name must be 12 characters or less                 */
/*--------------------------------------------------------------------*/

   if (len > 12)
      return KWFalse;

   strcpy(tempName, s);       /* Make a temp copy we can alter        */

/*--------------------------------------------------------------------*/
/*    Simple file name without extension must be eight characters     */
/*    or less                                                         */
/*--------------------------------------------------------------------*/

   ptr = strrchr(tempName, '.');
   if (ptr == NULL)
   {
      if (len > 8)
         return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*          Period must be in second through ninth character          */
/*--------------------------------------------------------------------*/

   else {
      if ((ptr == tempName) || (ptr > tempName + 8))
         return KWFalse;

/*--------------------------------------------------------------------*/
/*             Extension must be three characters or less             */
/*--------------------------------------------------------------------*/

      if (strlen(ptr) > 4) /* Three characters plus the period?       */
         return KWFalse;       /* No --> Too much                      */

/*--------------------------------------------------------------------*/
/*                          Only one period                           */
/*--------------------------------------------------------------------*/

      if (ptr != strchr(tempName, '.'))
         return KWFalse;
   } /* else */

/*--------------------------------------------------------------------*/
/*                Must only be valid MS-DOS characters                */
/*--------------------------------------------------------------------*/

   strlwr(tempName);          /* Map into our desired character set   */
   if (ptr != NULL)
      *ptr = 'x';             /* We've already accounted for the
                                 period, don't let it ruin our day    */

   if (strspn(tempName, E_charset) == len)
   {
#ifdef UDEBUG2
      printmsg(9,"ValidDOSName: \"%s\" is valid", s);
#endif
      return KWTrue;
   }
   else
      return KWFalse;

} /* ValidateDOSName */

#if defined(FAMILYAPI) || defined(__OS2__)

/*--------------------------------------------------------------------*/
/*    I s F i l e N a m e V a l i d                                   */
/*                                                                    */
/*    Determine if file system supports non-8.3 format names          */
/*--------------------------------------------------------------------*/

static int IsFileNameValid(char *name)
{
  HFILE hf;
#ifdef __OS2__
  ULONG uAction;
  APIRET result;
#else
  USHORT uAction;
  USHORT result;
#endif

  result = DosOpen(name, &hf, &uAction, 0, 0, FILE_OPEN,
                  OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, 0);

  switch(result)
  {
     case ERROR_INVALID_NAME:
     case ERROR_FILENAME_EXCED_RANGE:
#ifdef UDEBUG
        if (debuglevel > 1)
            printOS2error(name, result);
#endif
        return KWFalse;

     case NO_ERROR:                 /* Hmmm, why does the file exist? */
        DosClose(hf);
        return KWTrue;               /* But worked, so we have answer */

     default:
        if (debuglevel > 1)
            printOS2error(name, result);
        return KWTrue;

  } /* switch */

} /* IsFileNameValid */

/*--------------------------------------------------------------------*/
/*       a d v a n c e d F S                       (OS/2 version)     */
/*                                                                    */
/*       Determine if a file system is advanced (supports better than */
/*       8.3 file names)                                              */
/*--------------------------------------------------------------------*/

static KWBoolean advancedFS(const char *input)
{
   static char UUFAR cache[256] = "";  /* Initialize cache to zeroes  */

   char fdrive[_MAX_DRIVE],
        fpath[_MAX_DIR],
        fname[_MAX_FNAME],
        fdummy[_MAX_PATH];

  _splitpath((char *) input, fdrive, fpath, fdummy, fdummy);

/*--------------------------------------------------------------------*/
/*       Determine if our cache holds the answer for the drive        */
/*       if the path begins with an alphabetic character              */
/*--------------------------------------------------------------------*/

   if (isalpha( *fdrive ))
   switch(cache[ (unsigned char) *fdrive ])
   {
      case CACHE_LONG_NAME_SUPPORT:
      case CACHE_SHORT_NAME_ONLY:
         break;                        /* Cached answer, report it   */

      default:                         /* No cache, determine answer */
         strcpy(fname, fdrive);
         strcat(fname, fpath);
         strcat(fname, ".DUMB.TEST.NAME");

         cache[ (unsigned char) *fdrive ] =
                     (char) (IsFileNameValid(fname) ? CACHE_LONG_NAME_SUPPORT : CACHE_SHORT_NAME_ONLY);

         printmsg(4, "advancedFS: %s resides on file system supporting %s",
                     input,
                     (char *) (cache[ (unsigned char) *fdrive ] == CACHE_SHORT_NAME_ONLY ?
                               "only 8.3 names" :
                               "long file names"));
         break;
   }

/*--------------------------------------------------------------------*/
/*          Report our now cached results back to the caller          */
/*--------------------------------------------------------------------*/

   if (cache[ (unsigned char) *fdrive ] == CACHE_LONG_NAME_SUPPORT)
      return KWTrue;
   else
      return KWFalse;

} /* advancedFS */

#elif WIN32

/*--------------------------------------------------------------------*/
/*       a d v a n c e d F S                    (Window NT version)   */
/*                                                                    */
/*       Determine if a file system is advanced (supports better than */
/*       8.3 file names)                                              */
/*--------------------------------------------------------------------*/

static KWBoolean
advancedFS(const char *path)
{
   static char UUFAR cache[256] = "";  /* Initialize cache to zeroes  */
   char driveInfo[FILENAME_MAX];
   char fsType[128];
   BOOL result;
   KWBoolean cacheable;
   DWORD maxNameLength;
   KWBoolean longNamesSupported;

   if (!path || *path == '\0')         /* Name not supplied          */
   {                                   /* Yes --> Abort              */
      printmsg(0,"advancedFS: Missing path name");
   }
   else if (isalpha(*path) && (path[1] == ':'))
   {                                   /* It's a local drive          */

      printmsg(5, "advancedFS: it's a drive letter");
      strncpy(driveInfo, path, 3);
      cacheable = KWTrue;
      toupper( driveInfo[0] );      /* Normalize upper case for cache */
      driveInfo[3] = '\0';          /* Terminate drive string data    */

      switch(cache[ (unsigned char) driveInfo[0] ])
      {
         case CACHE_LONG_NAME_SUPPORT:
            return KWTrue;

         case CACHE_SHORT_NAME_ONLY:
            return KWFalse;

         default:
            break;
      }

   }
   else /* It's a shared drive... parse out the share name and ask */
   {
      int len;
      char *shareNameEnd;

      if (strncmp(path, "//", 2) != 0) /* Just double-checking */
         return KWFalse;  /* Don't know what it is, don't want to know */

      shareNameEnd = strchr(path + 2, '/');
      if (!shareNameEnd)  /* Probably bad:  server name only */
         return KWFalse;

      shareNameEnd = strchr(shareNameEnd + 1, '/');
      if (shareNameEnd)
      {
         /* Copy the server and share name, including the trailing slash */
         len = shareNameEnd - path + 1;
         memcpy(driveInfo, path, len);
         driveInfo[len] = '\0';
         shareNameEnd = driveInfo;

/*--------------------------------------------------------------------*/
/* On network drives, the GetVolumeInformation call fails unless      */
/* we use back slashes.                                               */
/*--------------------------------------------------------------------*/

         denormalize( shareNameEnd );
      } else
         return KWFalse;

      cacheable = KWFalse;

   } /* else */

/*--------------------------------------------------------------------*/
/*            We've got the drive letter, query its status            */
/*--------------------------------------------------------------------*/

   result = GetVolumeInformation(driveInfo,
                                 NULL,
                                 0,
                                 NULL,
                                 &maxNameLength,
                                 NULL,
                                 fsType,
                                 sizeof fsType);

   if (!result)
   {
      DWORD dwError = GetLastError();
      printmsg(0, "advancedFS: Unable to query file system for %s", driveInfo);
      printNTerror("GetVolumeInformation", dwError);
      panic();
   }

   if (maxNameLength > 12)
      longNamesSupported = KWTrue;
   else
      longNamesSupported = KWFalse;

   if ( cacheable )
      cache[ (unsigned char) driveInfo[0] ] =
               longNamesSupported ? CACHE_LONG_NAME_SUPPORT :
                                    CACHE_SHORT_NAME_ONLY;

   printmsg(4,"advancedFS: File system \"%s\" is type \"%s\""
              " with maximum name length %d",
               driveInfo,
               fsType,
               maxNameLength);

   return longNamesSupported;

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

static KWBoolean advancedFS(const char *path)
{
   return KWFalse;                /* DOS is always dumb on file systems! */
} /* advancedFS for MS-DOS */

#if _MSC_VER >= 700
#pragma warning(default:4100)   /* restore unref'ed formal param. warnings */
#endif

#endif
