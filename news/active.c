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
 *    $Id: active.c 1.23 1995/11/30 12:48:42 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: active.c $
 *    Revision 1.23  1995/11/30 12:48:42  ahd
 *    Allow unlimited length news group simple names via backup
 *    allocation method.
 *
 *    Correct search function to handle missing groups properly
 *
 *    Revision 1.22  1995/09/24 19:10:36  ahd
 *    Correct debugging output
 *
 *    Revision 1.21  1995/09/04 02:13:41  ahd
 *    Move news group tree to FAR memory in 16 bit systems
 *
 *    Revision 1.20  1995/08/27 23:33:15  ahd
 *    Load and use ACTIVE file as tree structure
 *
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
#include <malloc.h>

#include "active.h"

/*--------------------------------------------------------------------*/
/*                       Local data structures                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Strictly speaking, the proper maximum length of a new        */
/*       group simple name (such as "rec" or "humor" or "funny")      */
/*       is 14 characters, plus a terminating NULL, based on the      */
/*       length limit of simple file names older UNIX systems.        */
/*       This is a reasonable limit and is short enough to hard       */
/*       allocate the character buffer as part of the GROUP tree      */
/*       structure (below).                                           */
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
   struct _GROUP UUFAR *parent;     /* Our parent group              */
   struct _GROUP UUFAR *sibling;    /* Next group at this level      */
   struct _GROUP UUFAR *child;      /* First group at next level     */
   union
   {
      struct {
         char   flag;
         char   *namePtr;           /* Pointer to simple name        */
      } remote;
      struct {
         char   name[15];           /* simple name                   */
      } local;
   } n;
   char   moderation;               /* Y, N, or M                    */
   char   lap;                      /* Iteration of walking tree     */

} GROUP;

#define GROUPS_PER_BLOCK      (0x4000 / sizeof (GROUP))

static GROUP UUFAR *top = NULL;     /* Top of group tree             */
static GROUP UUFAR *cachedGroup = NULL;   /* Last group searched for */

static GROUP UUFAR *nextNode;       /* Saves state while walking tree*/

currentfile();

#ifdef UDEBUG

static long groups = 0;             /* Total groups loaded           */
static long nodes = 0;              /* Nodes in tree created         */
static long siblings = 0;           /* Nodes visited during load     */
static long parents = 0;            /* Parents recursed during load  */

static long searches = 0;           /* Number of searches performed  */
static long cacheHits = 0;          /* Number of cache hits in search*/
static long searchNodes = 0;        /* Nodes walked during searched  */

#endif

/*--------------------------------------------------------------------*/
/*       l o a d A c t i v e                                          */
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

#ifdef UDEBUG
   printmsg( 1, "loadActive: %ld groups in %ld nodes, "
                "via %ld siblings and %ld parents.",
                groups,
                nodes,
                siblings,
                parents );
#endif

   return KWTrue;

} /* loadActive */

/*--------------------------------------------------------------------*/
/*       g e t S i m p l e N a m e                                    */
/*                                                                    */
/*       Get simple news group name                                   */
/*--------------------------------------------------------------------*/

#ifdef COMPILED_GET_SIMPLE_NAME

static char UUFAR *
getSimpleName( const GROUP UUFAR *group )
{
   if ( group->n.remote.flag )      /* First character in use?       */
      return group->n.local.name;   /* Yes --> It's a local array    */
   else if ( group->n.remote.namePtr )
      return group->n.remote.namePtr;  /* No --> Pointer to string   */
   else
       return "";                   /* Empty String!                 */

} /* getSimpleName */

#else

#define getSimpleName( _group ) ((char UUFAR *)                \
      ( _group->n.remote.flag ?                                \
            _group->n.local.name :                             \
            (_group->n.remote.namePtr ? _group->n.remote.namePtr : "")))
#endif

/*--------------------------------------------------------------------*/
/*       m a k e L e v e l                                            */
/*                                                                    */
/*       Make a simple level name                                     */
/*--------------------------------------------------------------------*/

static char *
makeLevelName( const char *name )
{
   static char level[FILENAME_MAX];
   char *p;

/*--------------------------------------------------------------------*/
/*                       Create the level name                        */
/*--------------------------------------------------------------------*/

   strncpy( level, name, sizeof level - 1 );
   level[ sizeof level - 1] = '.';  /* Add fence post                */
   p = strchr( level, '.' );        /* Locate first period, if any   */

   if ( p != NULL )
   {
     *p = '\0';                     /* Terminate the string          */

     if ( p == (level + sizeof level - 1) )
        return NULL;
   }

   return level;

} /* makeLevelName */

/*--------------------------------------------------------------------*/
/*       m a k e G r o u p N a m e                                    */
/*                                                                    */
/*       Given a leaf of the active group tree, make the name         */
/*       by recursively appending the prefix of the group name.       */
/*--------------------------------------------------------------------*/

static char *
makeGroupName( char *buf, GROUP UUFAR *group )
{
   if ( group->parent == NULL )
      *buf = '\0';
   else {
      makeGroupName( buf, group->parent );
      strcat( buf, "." );
   }

   STRCAT( buf, getSimpleName( group ) );
   return buf;

} /* makeGroupName */

/*--------------------------------------------------------------------*/
/*       a d d N o d e                                                */
/*                                                                    */
/*       Get a node in the new group tree, creating it if needed      */
/*--------------------------------------------------------------------*/

static GROUP UUFAR *
addNode( GROUP UUFAR *first, GROUP UUFAR *parent, char *name )
{
   static GROUP UUFAR *blockAnchor = NULL;
   static size_t blockGroups;          /* Entries used in block         */

   GROUP UUFAR *current = first;
   GROUP UUFAR *previous = NULL;

   char *level = makeLevelName( name );

   if ( level == NULL )
   {
      return NULL;
   }

/*--------------------------------------------------------------------*/
/*                   Locate the name if it exists                     */
/*--------------------------------------------------------------------*/

   while( current != NULL )
   {
      int hit = STRCMP( level, getSimpleName( current ) );

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
      blockAnchor = MALLOC( sizeof (*current) * GROUPS_PER_BLOCK );
      checkref( blockAnchor );
#ifdef UDEBUG
      printmsg(4,"addNode: Allocated new block of %ld group names",
               (long) GROUPS_PER_BLOCK );
#endif
      blockGroups = 0;
   }

   current = blockAnchor++;
   blockGroups++;

   if ( blockGroups == GROUPS_PER_BLOCK ) /* Full block?             */
      blockAnchor = NULL;                 /* Get new one next pass   */

   MEMSET( current, 0, sizeof *current );

/*--------------------------------------------------------------------*/
/*       We store the string into the actual structure if it fits,    */
/*       otherwise (for longer strings) we grab a string off our      */
/*       pool.  This balances the fast access of the local fixed      */
/*       buffer versus the expensive but flexible master string       */
/*       poll.                                                        */
/*--------------------------------------------------------------------*/

   if ( strlen( level ) >= sizeof current->n.local.name )
      current->n.remote.namePtr = newstr( level );
   else
      STRCPY( current->n.local.name, level );

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
   GROUP UUFAR *parent;
   char  *level = (char *) group;
   static GROUP UUFAR *current = NULL;

   if ( current != NULL )
   {
      char buf[MAXGRP];
      char *fullName = makeGroupName( buf, current );

      size_t index = 0;
      char *lastMatch = NULL;       /* Presume no match at all       */

#ifdef UDEBUG
      int backSteps = 0;
#endif

/*--------------------------------------------------------------------*/
/*      Determine the best match for the node from our last insert    */
/*--------------------------------------------------------------------*/

      while( fullName[index] == group[index] )
      {
         if ( fullName[index] == '.' )
         {
            lastMatch = fullName + index;
            level     = (char *) group + index +1;
         }
         else if ( fullName[index] == '\0' )
         {
#ifdef UDEBUG
            printmsg(10,"Perfect match for %s and %s",
                        fullName, group );
#endif
            return KWFalse;         /* Perfect match, group exists!     */
         }

         index += 1;                /* Step to next char in each string */

      } /* while( fullName[index] == group [index] ) */

/*--------------------------------------------------------------------*/
/*    Back up the tree as far as we have to for the correct branch    */
/*--------------------------------------------------------------------*/

      if ( lastMatch == NULL)
         current = NULL;
      else {

         while( (lastMatch = strchr( lastMatch + 1 ,'.' )) != NULL )
         {
            current = current->parent;
#ifdef UDEBUG
            backSteps++;
#endif
         }

/*--------------------------------------------------------------------*/
/*   If group is out of order, begin at front of siblings on insert   */
/*--------------------------------------------------------------------*/

         if ( fullName[index] > group[index] )
         {
            current = current->parent;
            if ( current != NULL )
               current = current->child;
         }

      } /* else */

#ifdef UDEBUG2
      printmsg(9,"Matched %s to %s for %d, %d levels didn't match.",
                  fullName,
                  group,
                  index,
                  backSteps );
#endif
#ifdef UDEBUG
      parents += backSteps;
#endif

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

      if ( current == NULL )
      {
         printmsg(0,"Cannot add group %s (component of name too long?)",
                     group );
         return KWFalse;
      }

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
   {
      char buf[MAXGRP];

      printmsg(0,"Group %s already found as %s, moderation status is %c",
                  group,
                  makeGroupName( buf, current ),
                  current->moderation );
      return KWFalse;
   }

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

      case 'x':
      case 'm':
      case 'n':
      case 'y':
         current->moderation = moderation;
         break;

      default:
         printmsg(0,"addGroup: Invalid moderation flag %c for group %s, "
                    "changed to x",
                     (char) (isgraph( moderation ) ? moderation : '?'),
                     group );
         current->moderation = 'x';
         break;

   } /* switch( moderation ) */

/*--------------------------------------------------------------------*/
/*                    Return success to the caller                    */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG

   groups++;

#ifdef BIT32ENV
   if ( debuglevel > 6 )            /* Avoid getSimpleName calls     */
      printmsg(7 , "addGroup: Added group[%ld] %s (%ld %ld %c),"
                " parent %s%s%s",
                groups,
                  group,
                  high,
                  low,
                  moderation ? moderation : '-',
                  current->parent  ? getSimpleName( current->parent ) : "(none)",
                  current->sibling ? ", sibling " : "",
                  current->sibling ? getSimpleName( current->sibling ) : "" );
#endif

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

static GROUP UUFAR *
findGroup( const char *group )
{
   char *name = (char *) group;

   GROUP UUFAR *current = NULL;
   GROUP UUFAR *nextLevel = top;

#ifdef UDEBUG
   searches++;
#endif

   if ( name == NULL )
   {
      printmsg(0,"findGroup: Invalid call (null pointer parameter)");
      panic();
   }

/*--------------------------------------------------------------------*/
/*               See if we previously found this group                */
/*--------------------------------------------------------------------*/

   if ( cachedGroup != NULL )
   {
      char fullName[MAXGRP];
      makeGroupName( fullName, cachedGroup );

      if ( equal( fullName, group ) )
      {
#ifdef UDEBUG
         cacheHits++;
#endif /* UDEBUG */
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

         int hit = STRCMP( levelName, getSimpleName( current ) );

#ifdef UDEBUG
         searchNodes++;
#endif

         if ( ! hit )               /* Did we find the exact name?   */
            break;                  /* Go to next level of tree      */

         current = current->sibling;

         if (( hit < 0 ) || (current == NULL )) /* Name before this? */
            return NULL;            /* Yes --> Does not exist        */

      } /* while( current != NULL ) */

      if ( current == NULL )
      {

#ifdef UDEBUG
         printmsg(6,"findGroup: Did not find group %s", group );
#endif

         return NULL;
      }

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
   GROUP UUFAR *group = findGroup( name );

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
   GROUP UUFAR *group = findGroup( name );

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
   GROUP UUFAR *group = findGroup( name );

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
   GROUP UUFAR *group = findGroup( name );

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
   GROUP UUFAR *group = findGroup( name );

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
   GROUP UUFAR *group = findGroup( name );

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

static GROUP UUFAR *
nextActiveGroup( void )
{
   GROUP UUFAR *current = nextNode;

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
   GROUP UUFAR *group = nextActiveGroup();

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
   GROUP UUFAR *group;

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
   if ( searches > cacheHits )
      printmsg( 1, "writeActive: %ld searches, %ld cache hits and "
                   "%ld nodes (%ld/search)",
                   searches,
                   cacheHits,
                   searchNodes,
                   searchNodes / (searches - cacheHits));
#endif

} /* writeActive */
