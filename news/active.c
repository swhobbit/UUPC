/*--------------------------------------------------------------------*/
/*       a c t i v e . c                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.32 1995/07/21 13:28:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

/*--------------------------------------------------------------------*/
/*       Revised active file processing support for UUPC/extended     */
/*       news.                                                        */
/*                                                                    */
/*       This uses a tree structure mirroring the usenet name         */
/*       hierarchy to avoid length linear searches.                   */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <ctype.h>

#include "active.h"

/*--------------------------------------------------------------------*/
/*                       Local data structures                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       The basic structure used to make up the active list in       */
/*       memory.  A level is defined by a simple name between         */
/*       periods, for example rec.humor.funny is a three level        */
/*       name.  Names at the same level are linked in a sorted        */
/*       simple linked list under the previous level.  This yields    */
/*       a structure like:                                            */
/*                                                                    */
/*          alt -->  comp --> rec                                     */
/*           |         |       +--> games +--> humor                  */
/*           |         |              |          |                    */
/*           |         |              |          +--> funny           */
/*           |         |              |                               */
/*           |         |              +--> chess --> go               */
/*           |         |                                              */
/*           |         +--> lang --> mail --> os                      */
/*           .                .       .       .                       */
/*           .                .       .       .                       */
/*           .                .       .       .                       */
/*                                                                    */
/*       Note we allocate the fixed length level names within the     */
/*       structure.  This saves a made romp through the newstr        */
/*       data space, which wasn't designed to handle (literally)      */
/*       10000 strings crammed into it.                               */
/*--------------------------------------------------------------------*/

typedef struct _GROUP
{
   long   high;                     /* Next article number to store  */
   long   low;                      /* Lowest unexpired article num  */
   struct _GROUP  *parent;          /* Our parent group              */
   struct _GROUP  *sibling;         /* Next group at this level      */
   struct _GROUP  *child;           /* First group at next level     */
   char   name[15];                 /* simple name -- "humor" example*/
   char   moderation;               /* Y, N, or M                    */
   char   lap;                      /* Iteration of walking tree     */

} GROUP;

#ifdef BIT32ENV
#define GROUPS_PER_BLOCK      100
#else
#define GROUPS_PER_BLOCK      20
#endif

static GROUP *top = NULL;           /* Top of group tree             */
static GROUP *cachedGroup = NULL;   /* Last group searched for       */

static GROUP *nextNode;             /* Saves state while walking tree*/

currentfile();

#ifdef UDEBUG

static long groups;                 /* Total groups loaded           */
static long nodes;                  /* Nodes in tree created         */
static long siblings;               /* Nodes visited during load     */
static long parents;                /* Parents recursed during load  */

static long searches;               /* Number of searches performed  */
static long cacheHits;              /* Number of cache hits in search*/
static long searchNodes;            /* Nodes walked during searched  */

#endif

/*--------------------------------------------------------------------*/
/*       g e t A c t i v e                                            */
/*                                                                    */
/*       Load the new active file for processing                      */
/*--------------------------------------------------------------------*/

KWBoolean
loadActive( const KWBoolean mustExist )
{
   char fname[FILENAME_MAX];
   FILE *stream;

   mkfilename(fname, E_confdir, ACTIVE);
   stream = FOPEN(fname,"r",TEXT_MODE);

/*--------------------------------------------------------------------*/
/*       If we have no active file, we will allow processing to       */
/*       continue only if we were not expecting it to exist, such     */
/*       as it was being used for for remote batching only.           */
/*--------------------------------------------------------------------*/

   if ( stream == NULL )
   {
      if ( mustExist || (debuglevel > 1 ))
         printerr(fname);

      if ( mustExist )
         panic();
      else
         return KWFalse;

   }  /* if ( stream == NULL ) */

/*--------------------------------------------------------------------*/
/*                       Main loop to load file                       */
/*--------------------------------------------------------------------*/

   while (! feof( stream ) )
   {
      char buf[BUFSIZ];
      char *group;                  /* Name of group to add          */
      char *s;                      /* Work pointer                  */

      long low, high;               /* Bounds of active articles for
                                       group;                        */

      if (fgets(buf,BUFSIZ,stream) == NULL)
         break;                     /* Exit if end of file            */

/*--------------------------------------------------------------------*/
/*            Get group name; skip line if no data on line            */
/*--------------------------------------------------------------------*/

      group = strtok( buf, WHITESPACE );     /* First token is group  */

      if (group == NULL)
         continue;            /* Line is a empty; loop again         */

/*--------------------------------------------------------------------*/
/*                     Get highest article number                     */
/*--------------------------------------------------------------------*/

      s = strtok( NULL, WHITESPACE );

      if ( s == NULL )
         high = 0;
      else {
         high = atol( s );
         s = strtok( NULL, WHITESPACE );
      }

      if ( ! high )                 /* Always start with article 1   */
         high = 1;

/*--------------------------------------------------------------------*/
/*                     Get lowest article number                      */
/*--------------------------------------------------------------------*/

      if ( s == NULL )
         low  = 0;
      else {
         low = atol( s );
         s = strtok( NULL, WHITESPACE );
      }

/*--------------------------------------------------------------------*/
/*                        Get moderation flag                         */
/*--------------------------------------------------------------------*/

      if ( s == NULL )
      {
         s = "y";
         printmsg(0, "Adding default group information for %s",
                      group );
      }

      if ( !addGroup( group, high, low, *s ) )
      {
         printmsg(0,"loadActive: Unable to load group (duplicate?): %s",
                  group );
      }

   } /* while */

/*--------------------------------------------------------------------*/
/*           Done loading groups, close up the active file            */
/*--------------------------------------------------------------------*/

   if (fclose(stream))
      printerr( fname );

/*--------------------------------------------------------------------*/
/*     Verify we had a valid active file with at least one entry      */
/*--------------------------------------------------------------------*/

   if (top == NULL)
   {
      printmsg(0,"active: Active file %s is empty, cannot continue",
                  fname );
      panic();
   }

   return KWTrue;

} /* getActive */

/*--------------------------------------------------------------------*/
/*       m a k e L e v e l                                            */
/*                                                                    */
/*       Make a simple level name                                     */
/*--------------------------------------------------------------------*/

static char *
makeLevelName( const char *name )
{
   static char level[15];
   char *p;

/*--------------------------------------------------------------------*/
/*                       Create the level name                        */
/*--------------------------------------------------------------------*/

   strncpy( level, name, sizeof level - 1 );
   level[ sizeof level - 1] = '.';  /* Add fence post                */
   p = strchr( level, '.' );        /* Locate first period, if any   */

   if ( p != NULL )
     *p = '\0';                     /* Terminate the string          */

   return level;

} /* makeLevelName */

/*--------------------------------------------------------------------*/
/*       m a k e G r o u p N a m e                                    */
/*                                                                    */
/*       Given a leaf of the active group tree, make the name         */
/*       by recursively appending the prefix of the group name.       */
/*--------------------------------------------------------------------*/

static char *
makeGroupName( char *buf, GROUP *group )
{
   if ( group->parent == NULL )
      *buf = '\0';
   else {
      makeGroupName( buf, group->parent );
      strcat( buf, "." );
   }

   return strcat( buf, group->name );

} /* makeGroupName */

/*--------------------------------------------------------------------*/
/*       a d d N o d e                                                */
/*                                                                    */
/*       Get a node in the new group tree, creating it if needed      */
/*--------------------------------------------------------------------*/

static GROUP *
addNode( GROUP *first, GROUP *parent, char *name )
{
   static GROUP *blockAnchor = NULL;
   static size_t blockGroups;          /* Entries used in block         */

   GROUP *current = first;
   GROUP *previous = NULL;

   char *level = makeLevelName( name );

/*--------------------------------------------------------------------*/
/*                   Locate the name if it exists                     */
/*--------------------------------------------------------------------*/

   while( current != NULL )
   {
      int hit = strcmp( level, current->name );

      if ( ! hit )                  /* Did we find the exact name?   */
      {
         return current;            /* Yes --> Return it to caller   */
      }

#ifdef UDEBUG
         siblings++;
#endif

      if ( hit < 0 )                /* Name go before this?          */
         break;                     /* Yes --> Insert immediately    */

      previous = current;
      current = current->sibling;

   } /* while( current != NULL ) */

/*--------------------------------------------------------------------*/
/*                    We need to create a new node                    */
/*--------------------------------------------------------------------*/

   if ( blockAnchor == NULL )
   {
      blockAnchor = malloc( sizeof (*current) * GROUPS_PER_BLOCK );
      checkref( blockAnchor );
      blockGroups = 0;
   }

   current = blockAnchor++;
   blockGroups++;

   if ( blockGroups == GROUPS_PER_BLOCK ) /* Full block?             */
      blockAnchor = NULL;                 /* Get new one next pass   */

   memset( current, 0, sizeof *current );
   strcpy( current->name, level );

/*--------------------------------------------------------------------*/
/*                    Chain the node into the list                    */
/*--------------------------------------------------------------------*/

   current->parent = parent;

   if ( previous == NULL )
   {

      if ( parent != NULL )
         parent->child = current;   /* We're first node @ this level */

      current->sibling = first;     /* Put original first, if any,
                                       after us in this level's list */

      if ( top == first )           /* If previous first was top ... */
         top = current;             /* ... make us very top of tree. */

   }
   else {

      current->sibling = previous->sibling;
                                    /* Chain some after us, and ...  */
      previous->sibling = current;  /* ... insert us in list middle  */

   } /* else */

/*--------------------------------------------------------------------*/
/*                     Return newly created node                      */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   nodes++;
#endif

   return current;

} /* addNode */

/*--------------------------------------------------------------------*/
/*       a d d G r o u p                                              */
/*                                                                    */
/*       Add a new group to the active list                           */
/*--------------------------------------------------------------------*/

KWBoolean
addGroup( const char *group,
          const long high,
          const long low,
          const char moderation )
{
   GROUP *parent;
   char  *level;
   static GROUP *current = NULL;

   if ( current != NULL )
   {
      char buf[MAXGRP];
      char *fullName = makeGroupName( buf, current );

/*--------------------------------------------------------------------*/
/*      Determine the best natch for the node from our last insert    */
/*--------------------------------------------------------------------*/

      while ( current != NULL )
      {

         char *endPeriod = strrchr( fullName, '.' );
         size_t length;

         if ( endPeriod == NULL )
         {
            current = NULL;
            break;
         }

         length = (size_t) (endPeriod - fullName) + 1;

#ifdef UDEBUG
         parents++;

         printmsg(8,"Comparing \"%s\" == \"%s\" for %d",
                     fullName,
                     group,
                     length);
#endif

/*--------------------------------------------------------------------*/
/*       The actual check is that the names are equal except for      */
/*       the last qualifier, and that the last qualifier is           */
/*       greater for the new group to be added                        */
/*--------------------------------------------------------------------*/

         if ( equaln( fullName, group, length ) &&
              ( strcmp( fullName + length, group + length ) < 0))
         {
            level = (char *) group + length;

#ifdef UDEBUG
            printmsg(7,"Making level %s under %s next to %s",
                       level,
                       current->parent->name,
                       current->name );
#endif

            break;
         }

         current = current->parent;
         endPeriod = '\0';

      } /* while */

   } /* if ( current != NULL ) */

   if ( current == NULL )
   {
      current = top;
      parent = NULL;
      level = (char *) group;
   }
   else
      parent = current->parent;

/*--------------------------------------------------------------------*/
/*        Loop to walk tree to node, adding branches as needed        */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {

      current = addNode( current, parent, level );

      level = strchr( level, '.' ); /* Find next level of name       */

      if ( level != NULL )
      {
         parent = current;          /* Remember parent node          */
         current = parent->child;   /* Get first group on next level */
         level++;                   /* Step past period in name      */
      }
      else
         break;                     /* We're at node we need         */

   } /* for ( ;; ) */

/*--------------------------------------------------------------------*/
/*     If the node is alway initialized, return failure to caller     */
/*--------------------------------------------------------------------*/

   if ( current->moderation )
      return KWFalse;

/*--------------------------------------------------------------------*/
/*                        Initialize the node                         */
/*--------------------------------------------------------------------*/

   current->high = high;
   current->low  = low;

   switch( moderation )
   {
      case 'M':
      case 'N':
      case 'Y':
         current->moderation = (char) tolower( moderation );
         break;

      case 'm':
      case 'n':
      case 'y':
         current->moderation = moderation;
         break;

      default:
         printmsg(0,"addGroup: Invalid moderation flag for group %s, "
                    "changed to n" );
         current->moderation = 'n';
         break;

   } /* switch( moderation ) */

/*--------------------------------------------------------------------*/
/*                    Return success to the caller                    */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG

   groups++;

   printmsg(7 , "addGroup: Added group[%ld] %s (%ld %ld %c),"
                " parent %s%s%s",
                groups,
                  group,
                  high,
                  low,
                  moderation ? moderation : '-',
                  current->parent  ? current->parent->name  : "(none)",
                  current->sibling ? ", sibling " : "",
                  current->sibling ? current->sibling->name : "" );

   {
      char buf[MAXGRP];

      if ( ! equal( makeGroupName( buf, current ), group))
      {
         printmsg(0,"addGroup: Group %s added in wrong place as %s",
                     group,
                     buf );
         panic();
      }
   }

#endif

   return KWTrue;

} /* addGroup */

/*--------------------------------------------------------------------*/
/*       f i n d G r o u p                                            */
/*                                                                    */
/*       Locate an existing group in the tree                         */
/*--------------------------------------------------------------------*/

static GROUP *
findGroup( const char *group )
{
   char *name = (char *) group;

   GROUP *current;
   GROUP *nextLevel = top;

#ifdef UDEBUG
   searches++;
#endif

/*--------------------------------------------------------------------*/
/*               See if we previously found this group                */
/*--------------------------------------------------------------------*/

   if ( cachedGroup != NULL )
   {
      char fullName[MAXGRP];
      makeGroupName( fullName, cachedGroup );

      if ( equal( fullName, group ) )
      {
         cacheHits++;
         return cachedGroup;
      }

   } /* if ( cachedGroup != NULL ) */

/*--------------------------------------------------------------------*/
/*              Outer loop to work down through levels                */
/*--------------------------------------------------------------------*/

   while ( name != NULL )
   {
      char *levelName = makeLevelName( name );

      current = nextLevel;

/*--------------------------------------------------------------------*/
/*              Inner loop to locate simple name in list              */
/*--------------------------------------------------------------------*/

      while( current != NULL )
      {

         int hit = strcmp( levelName, current->name );

#ifdef UDEBUG
         searchNodes++;
#endif

         if ( ! hit )               /* Did we find the exact name?   */
            break;                  /* Go to next level of tree      */

         current = current->sibling;

         if (( hit < 0 ) || (current == NULL )) /* Name before this? */
            return NULL;            /* Yes --> Does not exist        */

      } /* while( current != NULL ) */

      name = strchr( name, '.');    /* Step to next level of name    */

      if ( name == NULL )
         break;                     /* We found target group         */
      else {
         name++;                    /* Step past period              */
         nextLevel  = current->child;  /* Move down tree as well     */
      }

   } /* while ( name != NULL ) */

/*--------------------------------------------------------------------*/
/*     We have the node, return it if valid, other report failure     */
/*--------------------------------------------------------------------*/

   if ( current->moderation )
      return current;
   else
      return NULL;

} /* findGroup */

/*--------------------------------------------------------------------*/
/*       d e l e t e G r o u p                                        */
/*                                                                    */
/*       Delete an existing news group                                */
/*--------------------------------------------------------------------*/

KWBoolean
deleteGroup( const char *name )
{
   GROUP *group = findGroup( name );

   if (( group != NULL ) && (group->moderation))
   {
      group->moderation = '\0';
      return KWTrue;
   }
   else
      return KWFalse;

}  /* deleteGroup */

/*--------------------------------------------------------------------*/
/*       g e t A r t i c l e N e w e s t                              */
/*                                                                    */
/*       Return next article sequence as a long                       */
/*--------------------------------------------------------------------*/

long
getArticleNewest( const char *name )
{
   GROUP *group = findGroup( name );

   if (( group != NULL ) && (group->moderation))
   {
      return group->high;
   }
   else
      return 0;

} /* getArticleNewest */

/*--------------------------------------------------------------------*/
/*       s e t A r t i c l e N e w e s t                              */
/*                                                                    */
/*       Set next article sequence as a long                          */
/*--------------------------------------------------------------------*/

KWBoolean
setArticleNewest( const char *name, const long new )
{
   GROUP *group = findGroup( name );

   if (( group != NULL ) && (group->moderation))
   {
      group->high = new;
      return KWTrue;
   }
   else
      return KWFalse;

} /* setArticleNewest */

/*--------------------------------------------------------------------*/
/*       g e t A r t i c l e O l d e s t                              */
/*                                                                    */
/*       Return next article sequence as a long, incrementing by      */
/*       user specified amount.                                       */
/*--------------------------------------------------------------------*/

long
getArticleOldest( const char *name )
{
   GROUP *group = findGroup( name );

   if (( group != NULL ) && (group->moderation))
   {
      return group->low;
   }
   else
      return 0;

} /* getArticleOldest */

/*--------------------------------------------------------------------*/
/*       s e t A r t i c l e O l d e s t                              */
/*                                                                    */
/*       Set next article sequence as a long                          */
/*--------------------------------------------------------------------*/

KWBoolean
setArticleOldest( const char *name, const long new )
{
   GROUP *group = findGroup( name );

   if (( group != NULL ) && (group->moderation))
   {
      group->low = new;
      return KWTrue;
   }
   else
      return KWFalse;

} /* setArticleOldest */

/*--------------------------------------------------------------------*/
/*       g e t M o d e r a t i o n                                    */
/*                                                                    */
/*       Return moderation status of news group                       */
/*--------------------------------------------------------------------*/

char
getModeration( const char *name )
{
   GROUP *group = findGroup( name );

   if ( group != NULL )
      return group->moderation;
   else
      return '\0';

} /* getModeration */

/*--------------------------------------------------------------------*/
/*       n e x t A c t i v e G r o u p                                */
/*                                                                    */
/*       Return next node in active group tree                        */
/*--------------------------------------------------------------------*/

static GROUP *
nextActiveGroup( void )
{
   GROUP *current = nextNode;

   if ( current != NULL )
      current->lap = top->lap;

/*--------------------------------------------------------------------*/
/*       Loop until we find a node to display next time or come up    */
/*       with a NULL point, which will end the search next call       */
/*--------------------------------------------------------------------*/

   while( ( nextNode != NULL ) && (nextNode->lap == top->lap ))
   {
      if (( nextNode->child != NULL ) &&
          (nextNode->child->lap != top->lap ))
         nextNode = nextNode->child;
      else if ( nextNode->sibling != NULL )
         nextNode = nextNode->sibling;
      else
         nextNode = nextNode->parent;

/*--------------------------------------------------------------------*/
/*         Next actually stop at a node which is not a group          */
/*--------------------------------------------------------------------*/

      if ( (nextNode != NULL) && ! nextNode->moderation )
         nextNode->lap = top->lap;

   } /* while */

/*--------------------------------------------------------------------*/
/*             Return the group we started the walk from              */
/*--------------------------------------------------------------------*/

   return current;

} /* nextActiveGroup */

/*--------------------------------------------------------------------*/
/*       s t a r t W a l k                                            */
/*                                                                    */
/*       Reset the current node of the tree to be walked to the       */
/*       top of the tree.                                             */
/*--------------------------------------------------------------------*/

void
startActiveWalk( void )
{
   nextNode = top;
   top->lap++;                      /* Use unique value each pass    */

   if ( ! top->moderation )         /* If top of tree not real group */
      nextActiveGroup();            /* ... skip past it in walk      */

} /* startWalk */

/*--------------------------------------------------------------------*/
/*       w a l k A c t i v e                                          */
/*                                                                    */
/*       Walk the active groups tree, returning the full name of      */
/*       one group per call.                                          */
/*--------------------------------------------------------------------*/

char *
walkActive( char *buf )
{
   GROUP *group = nextActiveGroup();

   if ( group == NULL )          /* End of the walk?                 */
      return NULL;               /* Yes --> Report same to caller    */
   else {

      cachedGroup = group;       /* Save this for findGroup          */
      return makeGroupName( buf, group );

   } /* else */

} /* walkActive */

/*--------------------------------------------------------------------*/
/*       w r i t e A c t i v e                                        */
/*                                                                    */
/*       Write an updated active file out from the tree               */
/*--------------------------------------------------------------------*/

void
writeActive()
{
   FILE *stream;
   char fname[FILENAME_MAX];
   GROUP *group;

   if ( top == NULL )
   {
      printmsg(0, "put_active: Attempt to update empty ACTIVE file");
      panic();
   }

   mkfilename( fname, E_confdir, ACTIVE );

   filebkup( fname );

   stream = FOPEN(fname, "w", TEXT_MODE);

   if (stream == NULL)
   {
      printmsg(0, "rnews: Cannot open active %s", fname );
      printerr(fname);
      panic();
   }

/*--------------------------------------------------------------------*/
/*           Loop to actually write out the updated groups            */
/*--------------------------------------------------------------------*/

   startActiveWalk();               /* Initialize the walk           */

   while( (group = nextActiveGroup() ) != NULL )
   {
      char buf[MAXGRP];

      makeGroupName( buf, group );

      fprintf( stream, "%s %ld %ld %c\n",
                        buf,
                        group->high,
                        group->low,
                        group->moderation );
   } /* while */

#ifdef UDEBUG
   printmsg( 1, "writeActive: Loaded %ld groups into %ld nodes, "
                "visiting %ld siblings and %ld parents.",
                groups,
                nodes,
                siblings,
                parents );

   printmsg( 1, "writeActive: Performed %ld searches, with %ld cache hits and "
                "visiting %ld nodes.",
                searches,
                cacheHits,
                searchNodes );
#endif

} /* writeActive */
