/*--------------------------------------------------------------------*/
/*    S y s A l i a s . C                                             */
/*                                                                    */
/*    System wide alias support for UUPC/extended                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2002 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: sysalias.c 1.21 2001/03/12 13:56:08 ahd v1-13k $
 *
 *    $Log: sysalias.c $
 *    Revision 1.21  2001/03/12 13:56:08  ahd
 *    Annual Copyright update
 *
 *    Revision 1.20  2000/05/12 12:35:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.19  1999/01/08 02:21:05  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.18  1999/01/04 03:54:27  ahd
 *    Annual copyright change
 *
 *    Revision 1.17  1998/11/01 20:38:15  ahd
 *    Add additional traps for memory allocation failing
 *
 * Revision 1.16  1998/03/01  01:31:49  ahd
 * Annual Copyright Update
 *
 *    Revision 1.15  1997/04/24 01:10:29  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1996/01/20 13:00:29  ahd
 *    Summarize number of aliases loaded from system alias table
 *
 *    Revision 1.13  1996/01/01 21:04:20  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/04/02 00:01:39  ahd
 *    Properly handle case of comment immediately before new alias;
 *    previously, starting location of alias was saved wrong
 *
 *    Revision 1.11  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.10  1995/01/08 19:52:44  ahd
 *    Add in memory files to RMAIL, including additional support and
 *    bug fixes.
 *
 *    Revision 1.9  1995/01/07 16:19:27  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.8  1994/12/22 00:19:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1994/12/09 03:42:09  ahd
 *    Modify alias support to recurse in system aliases file
 *    Put 'U' line first to work with brain dead MKS systems
 *
 * Revision 1.6  1994/02/19  04:18:57  ahd
 * Use standard first header
 *
 * Revision 1.6  1994/02/19  04:18:57  ahd
 * Use standard first header
 *
 * Revision 1.5  1994/01/24  03:17:36  ahd
 * Annual Copyright Update
 *
 * Revision 1.4  1994/01/01  19:13:22  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1993/04/11  00:33:05  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.2  1992/12/05  23:38:43  ahd
 * Skip blanks as well as unprintable characters
 *
 * Revision 1.1  1992/12/04  01:00:27  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>
#include <ctype.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "imfile.h"
#include "deliver.h"
#include "sysalias.h"

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

char *SysAliases = NULL;      /* Name of our system alias file       */
static ALIASTABLE *aliasTable = NULL;
static size_t aliases = 0;

RCSID("$Id: sysalias.c 1.21 2001/03/12 13:56:08 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                             Prototypes                             */
/*--------------------------------------------------------------------*/

static void InitAlias( void );

/*--------------------------------------------------------------------*/
/*    c h e c k a l i a s                                             */
/*                                                                    */
/*    Check the system alias table for a user.  We perform a linear   */
/*    search on the unordered table we we don't expect to perform     */
/*    many searchs in one execution of the program, and this saves    */
/*    sorting it.                                                     */
/*--------------------------------------------------------------------*/

ALIASTABLE *checkalias( const char *user )
{
   size_t subscript = 0;

   if ( SysAliases == NULL )
      InitAlias();

   for ( subscript = 0; subscript < aliases; subscript ++ )
   {
      if ( equali(aliasTable[subscript].alias , user ))
         return &aliasTable[subscript];

   } /* for */

/*--------------------------------------------------------------------*/
/*                        No hit, return NULL                         */
/*--------------------------------------------------------------------*/

   return NULL;

} /* checkalias */

/*--------------------------------------------------------------------*/
/*       I n i t A l i a s                                            */
/*                                                                    */
/*       Initialize our system alias table                            */
/*--------------------------------------------------------------------*/

static void InitAlias( void )
{
   char buf[BUFSIZ];
   int subscript  = -1;
   size_t maxmimumAliases = 64;
   KWBoolean inAlias = KWFalse;
   FILE *stream;
   long here;

/*--------------------------------------------------------------------*/
/*            Build the file name and try to open the file            */
/*--------------------------------------------------------------------*/

   mkfilename( buf, E_confdir, "aliases" );
   SysAliases = newstr( buf );

   stream = FOPEN( SysAliases , "r", TEXT_MODE );

   if ( stream == NULL )
   {
      if (debuglevel > 1)
         printerr( SysAliases );

      return;

   } /* if */

/*--------------------------------------------------------------------*/
/*                 The file is open, allocate a table                 */
/*--------------------------------------------------------------------*/

   aliasTable = malloc( sizeof *aliasTable * maxmimumAliases );
   checkref(aliasTable);

   here = ftell( stream );       /* Remember location in file        */

/*--------------------------------------------------------------------*/
/*                    Begin loop to process names                     */
/*--------------------------------------------------------------------*/

   while (fgets( buf , BUFSIZ , stream ) != NULL )
   {
      char *s = buf;

/*--------------------------------------------------------------------*/
/*    Ignore comments, lines that begin with whitespace, and empty    */
/*    lines                                                           */
/*--------------------------------------------------------------------*/

      while( *s && ! isgraph( *s ))
         s++;

      if (*s == '#')
      {
            /* Comment, no operation  */
      }
      else if ( ! *s )           /* Empty line?                      */
      {                          /* Yes --> 'tis end of alias        */
         if ( inAlias )
         {
            inAlias = KWFalse;
            if (aliasTable[subscript].start == -1 )
               printmsg(0,"%s: Invalid alias %s, no data defined!",
                  SysAliases, aliasTable[subscript].alias );
            else
               aliasTable[subscript].end = here;
         }
      } /* if ( ! *s ) */
      else if (inAlias)
      {
         if (aliasTable[subscript].start == -1 )
            aliasTable[subscript].start = here + (s - buf);
      }
      else {                           /* Start of a new alias */

         char *colon = strchr( s, ':' );

         if (s != buf )
            printmsg(0,"%s: Error aliases must begin in column 1",
                  SysAliases );

         if (colon == NULL )
         {
            printmsg(0,"%s: No colon after alias %s",
                  SysAliases, s);
            continue;
         }

         if ( subscript+2 == (int) maxmimumAliases )
         {
            maxmimumAliases *= 2;
            aliasTable = realloc( aliasTable,
                                  maxmimumAliases * sizeof *aliasTable );
            checkref( aliasTable );
         }

         *colon = '\0';          /* Terminate the name               */
         aliasTable[++subscript].alias = newstr(strtok(s,WHITESPACE));
                                 /* Save name of alias               */
         s = strtok( colon+1, WHITESPACE);
                                 /* Find next token                  */

         if ((s == NULL) || (*s == '#'))  /* Any alias on same?      */
             aliasTable[subscript].start = -1;
                                 /* No--> Look for it later          */
         else
             aliasTable[subscript].start = here + ( s - buf );

          aliasTable[subscript].end  = -1;
          aliasTable[subscript].recurse = KWFalse;
          inAlias = KWTrue;

      } /* if */

      here = ftell( stream );       /* Remember start of next line   */

   } /* while */

/*--------------------------------------------------------------------*/
/*                   Clean up and return to caller                    */
/*--------------------------------------------------------------------*/

   if ( inAlias )
      aliasTable[subscript].end = LONG_MAX;

   aliases = (size_t) subscript + 1;

   if ( aliases == 0 )
      free( aliasTable );
   else {
      aliasTable = realloc( aliasTable, aliases * sizeof *aliasTable );
      checkref(aliasTable);
   }

   printmsg(2,"InitAlias: Loaded %d system aliases from %s",
               aliases,
               SysAliases);

   fclose( stream );

} /* InitAlias */
