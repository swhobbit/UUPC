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
 *    $Id: active.c 1.25 1995/12/03 13:51:44 ahd Exp $
 *
 *    Revision history:
 *    $Log: active.c $
 *    Revision 1.25  1995/12/03 13:51:44  ahd
 *    Use binary search after tree is built
 *
 *    Revision 1.24  1995/12/02 14:18:33  ahd
 *    Various new debugging messages, move load to status to
 *    immediately after load is complete.
 *
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
/*       The basic structure used to load up the active list in       */
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
/*                                                                    */
/*       Once the lists are loaded, the top level is converted to     */
/*       a binary tree.  Then if any other list of sibling groups     */
/*       is searched, it too is converted on a demand basis into      */
/*       a binary tree.                                               */
/*--------------------------------------------------------------------*/

typedef struct _GROUP
{

   union
   {
      struct {
         char   flag;               /* Binary zero if namePtr used   */
         char   *namePtr;           /* Pointer to simple name        */
      } remote;
      char   name[15];              /* simple name                   */
   } n;

   char   moderation;               /* m, n, x, or y                  */

   union {
      struct {
         struct _GROUP UUFAR *left; /* Binary tree at this level     */
         struct _GROUP UUFAR *right;/* Binary tree at this level     */
      } s;
      struct _GROUP UUFAR *branch[2];  /* Binary tree as array       */
   } p;

   struct _GROUP UUFAR *parent;     /* Group at previous level       */
   struct _GROUP UUFAR *child;      /* First group at next level     */

   long   high;                     /* Next article number to store  */
   long   low;                      /* Lowest unexpired article num  */

} GROUP;

#define LEFT_BRANCH  0
#define RIGHT_BRANCH 1

#ifdef BIT32ENV
#define GROUPS_PER_BLOCK      (0x10000 / sizeof (GROUP))
#else
#define GROUPS_PER_BLOCK      200
#endif

static GROUP UUFAR *top = NULL;     /* Top of group tree             */
static GROUP UUFAR *cachedGroup = NULL;   /* Last group walked       */
static char *cachedGroupName = NULL;      /* Name of cachedGroup     */

currentfile();

static long groups = 0;             /* Total groups loaded           */
static long deletes = 0;            /* Deletes during processing     */
static long walked = 0;             /* Nodes written during output   */

#ifdef UDEBUG

static long nodes = 0;              /* Nodes in tree created         */
static long siblings = 0;           /* Nodes visited during load     */
static long parents = 0;            /* Parents recursed during load  */

static long searches = 0;           /* Number of searches performed  */
static long cacheHits = 0;          /* Number of cache hits in search*/
static long searchNodes = 0;        /* Nodes walked during search    */
static long searchLevels = 0;       /* Levels walked during search   */
static long balanced = 0;           /* Number of levels balanced     */
static long balancedBranches = 0;   /* None leaf nodes balanced      */

static long pushed = 0;             /* Current uses of pushGroup     */
static long maxPushed = 0;          /* MAx uses of pushGroup         */

#endif

/*--------------------------------------------------------------------*/
/*                Macros to handle tree balance flags                 */
/*--------------------------------------------------------------------*/

#define IS_UNBALANCED( _group )  \
           (((_group->p.s.left == NULL) && (_group->p.s.right != NULL)) || \
            ((_group->p.s.left != NULL) && (_group->p.s.right == NULL)))
#define SET_BALANCED( _group )

/*--------------------------------------------------------------------*/
/*                    Moderation/valid group flags                    */
/*--------------------------------------------------------------------*/

#define GET_MODERATION( _group ) _group->moderation

#define IS_GROUP( _group ) GET_MODERATION( _group )

#define SET_MODERATION( _group,_c ) _group->moderation = _c

#define SET_NOT_GROUP( _group )     SET_MODERATION( _group, 0)

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
      return group->n.name;         /* Yes --> It's a local array    */
   else if ( group->n.remote.namePtr )
      return group->n.remote.namePtr;  /* No --> Pointer to string   */
   else
       return "";                   /* Empty String!                 */

} /* getSimpleName */

#else

#define getSimpleName( _group ) ((char UUFAR *)                \
      ( _group->n.remote.flag ?                                \
            _group->n.name :                                   \
            (_group->n.remote.namePtr ? _group->n.remote.namePtr : "")))
#endif

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
/*       m a k e L e v e l                                            */
/*                                                                    */
/*       Make a simple level name                                     */
/*--------------------------------------------------------------------*/

static char *
makeLevelName( const char *name )
{
   static char level[MAXGRP];
   char *p;

/*--------------------------------------------------------------------*/
/*                       Create the level name                        */
/*--------------------------------------------------------------------*/

   strncpy( level, name, sizeof level - 1 );
   level[ sizeof level - 1] = '.';  /* Add fence post                */
   p = strchr( level, '.' );        /* Locate first period, if any   */

   if ( p != NULL )
   {
      *p = '\0';                    /* Terminate the string          */

      if ( p == (level + sizeof level - 1) )
      {
         printmsg(0,"makeLevelName: Too long to use as newsgroup: %s",
                  name );
         panic();
      }

   } /* if ( p != NULL ) */

   return level;

} /* makeLevelName */

#if defined(UDEBUG) && defined(BIT32ENV)

/*--------------------------------------------------------------------*/
/*       p r i n t T r e e                                            */
/*                                                                    */
/*       Print current data tree                                      */
/*--------------------------------------------------------------------*/

static void
printTree( GROUP UUFAR *current, const char *side, const int depth )
{

   if ( current == NULL )
      return;

   if ( ! depth )
      printmsg(2,"printTree **** %s.%s ****",
                  current->parent ?
                     getSimpleName( current->parent ) : "(top)",
                  getSimpleName( current ) );

   printTree( current->p.s.right, "right", depth + 1 );

   {
      printmsg(IS_GROUP( current ) ? 3 : 4,
                  "printTree: %3d %*s%s (%s) %s",
                  depth,
                  depth % 50,
                  "",
                  getSimpleName( current ),
                  side,
                  IS_GROUP( current ) ? "\tgroup" : "" );
   }

   printTree( current->p.s.left, "left", depth + 1 );

} /* printTree */

#endif

/*--------------------------------------------------------------------*/
/*       b a l a n c e L i s t                                        */
/*                                                                    */
/*       Convert a linked list into a binary tree                     */
/*--------------------------------------------------------------------*/

static GROUP UUFAR *
balanceList( GROUP UUFAR * elder,
             const size_t siblings,
             const size_t forward,
             const size_t backward )
{
   size_t nodesPreceding;
   size_t nodesFollowing;
   size_t i;
   GROUP UUFAR *current;

/*--------------------------------------------------------------------*/
/*       Handle terminating case of a branch with only one right at   */
/*       most.                                                        */
/*--------------------------------------------------------------------*/

   if ( siblings == 1 )
   {
      elder->p.branch[backward]  = NULL;
      elder->p.branch[forward] = NULL;
      return elder;
   }

/*--------------------------------------------------------------------*/
/*       Step to center of tree.  Note if this branch only has two    */
/*       siblings, we make the right side the root to invert the      */
/*       NULL pointer from right to left side, resetting the          */
/*       unbalanced tree flag.                                        */
/*--------------------------------------------------------------------*/

   current = elder;

   nodesPreceding  = siblings / 2;
   nodesFollowing = siblings - nodesPreceding - 1;

   for ( i = 0; nodesPreceding > i; i++ )
      current = current->p.branch[forward];

   if ( nodesFollowing )
      current->p.branch[forward]  = balanceList( current->p.branch[forward],
                                                 nodesFollowing,
                                                 forward,
                                                 backward );
   else
      current->p.branch[forward] = NULL;

   if ( nodesPreceding )
      current->p.branch[backward] = balanceList( elder,
                                                 nodesPreceding,
                                                 forward,
                                                 backward );
   else
      current->p.branch[backward] = NULL;

   return current;

} /* balanceList */

/*--------------------------------------------------------------------*/
/*       b a l a n c e                                                */
/*                                                                    */
/*       Initialize balancing a branch of of our tree                 */
/*--------------------------------------------------------------------*/

static GROUP UUFAR *
balance( GROUP UUFAR * elder )
{
   GROUP UUFAR *current = elder;
   size_t siblings = 0;

   size_t forward, backward;

   if ( elder->p.s.left == NULL )
   {

      backward = LEFT_BRANCH;
      forward  = RIGHT_BRANCH;
   }
   else {
      backward = RIGHT_BRANCH;
      forward  = LEFT_BRANCH;
   }

/*--------------------------------------------------------------------*/
/*       If this branch has two nodes, we can't balance it using      */
/*       our simple routine.  We balance the two branches under       */
/*       this node, but we don't try to balance the rest of the       */
/*       linked list prior to this node.                              */
/*--------------------------------------------------------------------*/

   do {

      if ( current->p.branch[backward] != NULL )
      {
         current->p.branch[backward] = balance( current->p.branch[backward] );

         if ( current->p.branch[forward] != NULL )
            current->p.branch[forward] =  balance( current->p.branch[forward] );

         SET_BALANCED( elder );
         return elder;
      }

      siblings++;

   } while( (current = current->p.branch[forward]) != NULL );

   current = balanceList( elder, siblings, forward, backward );

#ifdef UDEBUG

   balanced++;
   balancedBranches += (long) siblings;

   if ( debuglevel > ((elder == top ) ? 3 : 2 ))
      printTree(  current, (elder == top ) ? "===> top <===" : "top", 0);

#endif

   SET_BALANCED( balanced );

   return current;

} /* balance */

/*--------------------------------------------------------------------*/
/*       c r e a t e N o d e                                          */
/*                                                                    */
/*       Create current level as right node of "previous", or         */
/*       under parent node if no previous node is specified.          */
/*--------------------------------------------------------------------*/

GROUP UUFAR *
createNode( const char *level,
            GROUP UUFAR *parent,
            GROUP UUFAR **node,
            GROUP UUFAR *left,
            GROUP UUFAR *right )
{
   static GROUP UUFAR *blockAnchor = NULL;
   static size_t blockGroups;          /* Entries used in block      */

/*--------------------------------------------------------------------*/
/*                    We need to create a new node                    */
/*--------------------------------------------------------------------*/

   if ( blockAnchor == NULL )
   {
      blockAnchor = MALLOC( sizeof (*(*node)) * GROUPS_PER_BLOCK );
      checkref( blockAnchor );

#ifdef UDEBUG
      printmsg(4,"createNode: Allocated new block of %ld group names",
               (long) GROUPS_PER_BLOCK );
#endif

      blockGroups = 0;
   }

   (*node) = blockAnchor++;
   blockGroups++;

   if ( blockGroups == GROUPS_PER_BLOCK ) /* Full block?             */
      blockAnchor = NULL;                 /* Get new one next pass   */

   MEMSET( (*node), 0, sizeof *(*node) );

   if ( strchr( level, '.' ))
   {
      printmsg(0,"Level %s contains period!", level );
      panic();
   }

/*--------------------------------------------------------------------*/
/*       We store the string into the actual structure if it fits,    */
/*       otherwise (for longer strings) we grab a string off our      */
/*       pool.  This balances the fast access of the local fixed      */
/*       buffer versus the expensive but flexible master string       */
/*       poll.                                                        */
/*--------------------------------------------------------------------*/

   if ( strlen( level ) >= sizeof (*node)->n.name )
      (*node)->n.remote.namePtr = newstr( level );
   else
      STRCPY( (*node)->n.name, level );

   (*node)->parent = parent;
   (*node)->p.s.right  = right;
   (*node)->p.s.left   = left;

#if  defined(UDEBUG) && defined(BIT32ENV)
      printmsg(9,"createNode: Inserted %s.%s, first child is %s, right node is %s",
                  parent  ? getSimpleName( parent ) : "(top)",
                  level,
                  parent  ? getSimpleName( parent->child ) : "(end of list)",
                  right   ? getSimpleName( right ) : "(end of list)");

#endif

/*--------------------------------------------------------------------*/
/*                     Return newly created node                      */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   nodes++;
#endif

   return *node;

} /* createNode */

/*--------------------------------------------------------------------*/
/*       i n s e r t N o d e                                          */
/*                                                                    */
/*       Locate a node in the group tree, creating it if needed.      */
/*                                                                    */
/*       This function is strongly biased towards inserting nodes     */
/*       into the right hand side of tree, since the simple           */
/*       balance routine we use is biased towards this as well.       */
/*       This normally works very fast since the active file is       */
/*       sorted when written out and this function correctly          */
/*       handles quickly inserting in strict ascending sequence.      */
/*                                                                    */
/*       If we have to insert on the left to keep the tree            */
/*       ordered we can and will, although balancing of the           */
/*       particular list will impacted.  This in turn may impact      */
/*       search speed, although not the final result.                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       NOTE NOTE NOTE                                               */
/*                                                                    */
/*       Because we initialize linear to FALSE below, the right       */
/*       hand bias is disabled.  I don't know what the exact          */
/*       problem is, and don't have time to fix it at this point.     */
/*                                                                    */
/*       Send any improved version to software@kew.com                */
/*--------------------------------------------------------------------*/

static GROUP UUFAR *
insertNode( GROUP UUFAR *current, GROUP UUFAR *parent, char *name )
{

   char *level = makeLevelName( name );
   static KWBoolean linear = KWFalse;     /* Lock into slow mode!    */

   if ( current == NULL )
   {
      if ( parent == NULL )
         return createNode( level, NULL, &top, NULL, NULL );
      else
         return createNode( level, parent, &parent->child, NULL, NULL );
   }

/*--------------------------------------------------------------------*/
/*                   Locate the name if it exists                     */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {
      int hit = STRCMP( level, getSimpleName( current ) );

      if ( ! hit )                  /* Did we find the exact name?   */
         return current;            /* Yes --> Return it to caller   */
      else if ( hit > 0 )           /* We greater than current?      */
      {
         if ( current->p.s.left != NULL )
         {
            if ( linear )
            {
               linear = KWFalse;
               printmsg(1,"insertNode: active file not sorted on left,"
                           " switching to slow insert mode after %ld groups",
                           groups );
            }
            current = current->p.s.left;
         }
         else if (( current == top ) && linear)
            return createNode( level, parent, &top, NULL, current);
         else if ((parent != NULL) && ( parent->child == current ) && linear )
            return createNode( level, parent, &(parent->child), NULL, current);
         else
            return createNode( level, parent, &(current->p.s.left), NULL, NULL );
      }
      else {
         if ( linear )
         {
            linear = KWFalse;
            printmsg(1,"insertNode: active file not sorted on right,"
                       " switching to slow insert mode after %ld groups",
                       groups );
         }
         if ( current->p.s.right == NULL )
            return createNode( level, parent, &(current->p.s.right), NULL, NULL );
         else {
            current = current->p.s.right;
         }
      }

#ifdef UDEBUG
      siblings++;
#endif

   } /* for ( ;; ) */

#ifndef __TURBOC__
   return NULL;
#endif

} /* insertNode */

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

      current = insertNode( current, parent, level );

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

   if ( IS_GROUP( current ))
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
      case 'X':
      case 'Y':
         SET_MODERATION(current, (char) tolower( moderation ) );
         break;

      case 'm':
      case 'n':
      case 'x':
      case 'y':
         SET_MODERATION(current, moderation);
         break;

      default:
         printmsg(0,"addGroup: Invalid moderation flag %c for group %s, "
                    "changed to x",
                     (char) (isgraph( moderation ) ? moderation : '?'),
                     group );
         SET_MODERATION(current,'x' );
         break;

   } /* switch( moderation ) */

/*--------------------------------------------------------------------*/
/*                    Return success to the caller                    */
/*--------------------------------------------------------------------*/

   groups++;

#ifdef UDEBUG
#ifdef BIT32ENV

   if ( debuglevel > 6 )            /* Avoid getSimpleName calls     */
      printmsg(7 , "addGroup: Added group[%ld] %s (%ld %ld %c),"
                " parent %s%s%s",
                groups,
                  group,
                  high,
                  low,
                  moderation ? moderation : '-',
                  current->parent  ? getSimpleName( current->parent ) :
                                     "(none)",
                  current->p.s.right ? ", sibling " : "",
                  current->p.s.right ? getSimpleName( current->p.s.right ) :
                                       "" );
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
/*       l o a d A c t i v e                                          */
/*                                                                    */
/*       Load the new active file for processing                      */
/*--------------------------------------------------------------------*/

KWBoolean
loadActive( const KWBoolean mustExist )
{
   char fname[FILENAME_MAX];
   FILE *stream;
   GROUP UUFAR *current;

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

/*--------------------------------------------------------------------*/
/*       Balance second level of tree, assuming we're at most of a    */
/*       pair simple linked lists off the top node.  (If any trees    */
/*       got inserted, we don't bother to balance them; this is       */
/*       rare, and searches will be still valid, if slower.)          */
/*--------------------------------------------------------------------*/

   current = top;

   do {

      if ( current->child != NULL )
         current->child = balance( current->child );

   } while(( current = current->p.s.right ) != NULL );

   current = top;

   while(( current = current->p.s.left ) != NULL )
   {
      if ( current->child != NULL )
         current->child = balance( current->child );
   }


/*--------------------------------------------------------------------*/
/*                    Also balance top level of tree                  */
/*--------------------------------------------------------------------*/

   top = balance( top );            /* Balance top level of tree     */

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

   if (( cachedGroupName != NULL ) &&  equal( cachedGroupName , group ))
   {
#ifdef UDEBUG
      cacheHits++;
#endif /* UDEBUG */
      return cachedGroup;

   } /* if ( cachedGroupName != NULL ) */

/*--------------------------------------------------------------------*/
/*              Outer loop to work down through levels                */
/*--------------------------------------------------------------------*/

   while ( name != NULL )
   {
      char *levelName = makeLevelName( name );

      current = nextLevel;

#ifdef UDEBUG
      searchLevels++;
#endif

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

         if ( hit > 0 )
            current = current->p.s.left;
         else if ( hit < 0 )
            current = current->p.s.right;

         if (current == NULL )      /* Name before this?             */
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

         nextLevel = current->child;  /* Move down tree as well      */

         if (( nextLevel != NULL ) &&
               IS_UNBALANCED( nextLevel ))
         {
               nextLevel = nextLevel->parent->child =
                           balance( nextLevel );
         }
      }

   } /* while ( name != NULL ) */

/*--------------------------------------------------------------------*/
/*     We have the node, return it if valid, other report failure     */
/*--------------------------------------------------------------------*/

   if ( IS_GROUP( current ))
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

   if (( group != NULL ) && IS_GROUP( group ))
   {

      deletes++;
      SET_NOT_GROUP( group );

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

   if (( group != NULL ) && IS_GROUP( group ))
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

   if (( group != NULL ) && IS_GROUP( group ))
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

   if (( group != NULL ) && IS_GROUP( group ))
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

   if (( group != NULL ) && IS_GROUP( group ))
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

   if (( group != NULL ) && IS_GROUP( group ))
      return GET_MODERATION( group );
   else
      return '\0';

} /* getModeration */

/*--------------------------------------------------------------------*/
/*       n e x t A c t i v e G r o u p                                */
/*                                                                    */
/*       Return next node in active group tree                        */
/*--------------------------------------------------------------------*/

static void
nextActiveGroup(  GROUP UUFAR *node,
                  void (*walkOneGroup)(const char *, void *),
                  void *optional )
{

#ifdef UDEBUG
   if ( ++pushed > maxPushed )
      maxPushed = pushed;
#endif

/*--------------------------------------------------------------------*/
/*       Loop until we find a node to display next time or come up    */
/*       with a NULL point, which will end the search next call       */
/*--------------------------------------------------------------------*/

   do {

/*--------------------------------------------------------------------*/
/*       Since we would have to perform four different checks, we     */
/*       always just recursively walk the right hand node rather      */
/*       than attempting tail-end recursion.                          */
/*--------------------------------------------------------------------*/

      if (node->p.s.right != NULL)
         nextActiveGroup( node->p.s.right, walkOneGroup, optional );

/*--------------------------------------------------------------------*/
/*       Handle the current node; default action is to perform        */
/*       output our updated active file.                              */
/*--------------------------------------------------------------------*/

      if ( IS_GROUP( node ) )
      {

         char buf[MAXGRP];

         makeGroupName( buf, node );

         if ( walkOneGroup == NULL )
         {
            walked++;               /* Count for error checking      */

            fprintf( (FILE *) optional, "%s %ld %ld %c\n",
                              buf,
                              node->high,
                              node->low,
                              GET_MODERATION( node ));
         }
         else {

            cachedGroup = node;
            cachedGroupName = buf;
            (*walkOneGroup)( buf, optional );
         }

      } /* if ( IS_GROUP( node ) ) */

/*--------------------------------------------------------------------*/
/*       For the last node at this level, we perform tail end         */
/*       recursive to limit blowing the stack.                        */
/*                                                                    */
/*       Note that for child groups, we balance the level if          */
/*       calling an external function; that's because any search      */
/*       from that function could redistribute the tree, causing      */
/*       us serious problems, and to avoid overly deep recursions.    */
/*--------------------------------------------------------------------*/

      if ( node->child != NULL )
      {

#ifndef BIT32ENV

/*--------------------------------------------------------------------*/
/*       To limit stack recursion in bit 16 environments, we          */
/*       balance the tree as we walk it.  This doesn't help           */
/*       performance, so we don't bother in 32 bit environments.      */
/*--------------------------------------------------------------------*/

         if (IS_UNBALANCED( node->child) )
            node->child = balance( node->child );

#endif

         if ( node->p.s.left == NULL )
            node = node->child;
         else {
            nextActiveGroup( node->child, walkOneGroup, optional );
            node = node->p.s.left;
         }
      }
      else
         node = node->p.s.left;

   } while ( node != NULL );

   cachedGroupName = NULL;          /* reset now invalid pointer     */

#ifdef UDEBUG
   pushed--;
#endif

} /* nextActiveGroup */

/*--------------------------------------------------------------------*/
/*       s t a r t W a l k                                            */
/*                                                                    */
/*       Reset the current node of the tree to be walked to the       */
/*       top of the tree.                                             */
/*--------------------------------------------------------------------*/

void
startActiveWalk( void (*walkOneGroup)(const char *, void *),
                 void *optional )
{
   if ( walkOneGroup == NULL )      /* They want default action?     */
      panic();                      /* Not allowed from extern calls */
   else
      nextActiveGroup( top, walkOneGroup, optional);

} /* startWalk */

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
   char aname[FILENAME_MAX];

   if ( top == NULL )
   {
      printmsg(0, "put_active: Attempt to update empty ACTIVE file");
      panic();
   }

   mkdirfilename( fname, E_confdir, "tmp" );

   stream = FOPEN(fname, "w", TEXT_MODE);

   if (stream == NULL)
   {
      printmsg(0, "writeActive: Cannot open active %s", fname );
      printerr(fname);
      panic();
   }

/*--------------------------------------------------------------------*/
/*           Loop to actually write out the updated groups            */
/*--------------------------------------------------------------------*/

   nextActiveGroup( top, 0, (void *) stream );

   fclose( stream );

#ifdef UDEBUG
   printmsg( 1, "writeActive: Balanced %ld hierarchies with %ld "
                "branches, maximum depth %ld",
               balanced,
               balancedBranches,
               maxPushed);

   if ( searches > cacheHits )
   {
      long realSearches = searches - cacheHits;

      printmsg( 1, "writeActive: %ld searches, %ld cache hits, "
                   "%ld tree searches.",
                   searches,
                   cacheHits,
                   realSearches );

      printmsg( 1, "writeActive: %ld levels (%ld.%ld/search), "
                   "%ld nodes (%ld.%ld/search), "
                   "%ld.%ld nodes/level searched.",
                   searchLevels,
                   searchLevels / realSearches,
                   ((searchLevels * 10) / realSearches + 5) % 10,
                   searchNodes,
                   searchNodes / realSearches,
                   ((searchNodes * 10) / realSearches + 5) % 10,
                   searchNodes / searchLevels,
                   ((searchNodes * 10) / searchLevels + 5) % 10);
   }
#endif

   if ( groups != ( walked + deletes ))
   {
      printmsg(0,"Internal error: Loaded %ld groups, "
                 "deleted %ld, wrote %ld groups",
                 groups,
                 deletes,
                 walked );
      panic();
   }

/*--------------------------------------------------------------------*/
/*      Rename the temporary active to the true active file name      */
/*--------------------------------------------------------------------*/

   mkfilename( aname, E_confdir, ACTIVE );
   filebkup( aname );
   REMOVE( aname );

   if ( rename( fname, aname ) )
   {
      printerr( aname );
      panic();
   } /* if ( rename( fname, aname ) ) */

} /* writeActive */
