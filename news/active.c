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
/*       The red/black tree balancing and supporting rotate           */
/*       functions is adapted from "Classical Algorithms in C++"      */
/*       by Nicholas Wilt.                                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: active.C 1.26 1995/12/12 13:48:54 ahd Exp $
 *
 *    Revision history:
 *    $Log: active.C $
 *    Revision 1.26  1995/12/12 13:48:54  ahd
 *    Use binary tree for news group active file
 *    Use large buffers in news programs to avoid overflow of hist db recs
 *    Use true recursive function to walk entire active file
 *
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
#include <types.h>
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
/*                                                                    */
/*       The actual lists at each level are done as red-black         */
/*       trees, which provide good worst case performance with        */
/*       miminal overhead.                                            */
/*--------------------------------------------------------------------*/

#define F_COLOR_RED       0x01
#define F_VALID_GROUP     0x02
#define F_LONGGROUP       0x04

#define F_MODERATED       0x10
#define F_UNMODERATED     0x20
#define F_NOPOSTING       0x40

typedef struct _GROUP
{

   union
   {
      char   *namePtr;              /* Pointer to simple name        */
      char   name[15];              /* Simple name                   */
   } n;

   u_char flags;                    /* Bit flags, see above          */

   struct _GROUP UUFAR *left;       /* Binary tree at this level     */
   struct _GROUP UUFAR *right;      /* Binary tree at this level     */
   struct _GROUP UUFAR *parent;     /* Parent node in current level  */

   struct _GROUP UUFAR *prevLevel;  /* Group at previous level       */
   struct _GROUP UUFAR *nextLevel;  /* First group at next level     */

   long   high;                     /* Next article number to store  */
   long   low;                      /* Lowest unexpired article num  */

} GROUP;

#ifdef BIT32ENV
#define GROUPS_PER_BLOCK      (0x10000 / sizeof (GROUP))
#else
#define GROUPS_PER_BLOCK      200
#endif

static GROUP UUFAR *cachedGroup = NULL;   /* Last group walked       */
static char *cachedGroupName = NULL;      /* Name of cachedGroup     */

static GROUP UUFAR *topNode;        /* Top of group tree             */

currentfile();

static long groups = 0;             /* Total groups loaded           */
static long deletes = 0;            /* Deletes during processing     */
static long walked = 0;             /* Nodes written during output   */

#ifdef UDEBUG

static long nodes = 0;              /* Nodes in tree created         */
static long siblings = 0;           /* Nodes visited during load     */
static long prevLevels = 0;         /* Parents recursed during load  */

static long searches = 0;           /* Number of searches performed  */
static long cacheHits = 0;          /* Number of cache hits in search*/
static long searchNodes = 0;        /* Nodes walked during search    */
static long searchLevels = 0;       /* Levels walked during search   */

static long pushed = 0;             /* Current uses of pushGroup     */
static long maxPushed = 0;          /* MAx uses of pushGroup         */

#endif

/*--------------------------------------------------------------------*/
/*                    Moderation/valid group flags                    */
/*--------------------------------------------------------------------*/

#define bitOn( _group, _bitName ) (_group->flags = (u_char) \
         (_group->flags | (u_char) _bitName ))

#define bitOff( _group, _bitName ) (_group->flags = (u_char) \
         (_group->flags & (u_char) (0xff - _bitName) ))

#define GET_MODERATION( _group )    translateModeration( _group->flags )
#define SET_MODERATION( _group,_c ) bitOn( _group, encodeModeration( _c ))

#define IS_GROUP( _group )       ( _group->flags & F_VALID_GROUP )
#define SET_NOT_GROUP( _group )  bitOff( _group, F_VALID_GROUP )
#define SET_GROUP( _group )      bitOn( _group, F_VALID_GROUP )

#define setBlack( _group ) bitOff( _group, F_COLOR_RED )
#define setRed( _group )   bitOn( _group, F_COLOR_RED )
#define isRed( _group )    ( _group->flags & F_COLOR_RED )

#define SET_LONGGROUP( _group )   bitOn( _group, F_LONGGROUP )
#define IS_LONGGROUP( _group )    ( _group->flags & F_LONGGROUP )

/*--------------------------------------------------------------------*/
/*       g e t S i m p l e N a m e                                    */
/*                                                                    */
/*       Get simple news group name                                   */
/*--------------------------------------------------------------------*/

#define getSimpleName( _group ) ((char UUFAR *)          \
            ( IS_LONGGROUP( _group ) ?                   \
               _group->n.namePtr : _group->n.name ))

/*--------------------------------------------------------------------*/
/*       e n c o d e M o d e r a t i o n                              */
/*                                                                    */
/*       Encode character moderation flag as bit flag                 */
/*--------------------------------------------------------------------*/
;

u_char
encodeModeration( const char moderation )
{
   switch( moderation )
   {
      case 'y':
         return (u_char) F_UNMODERATED;

      case 'm':
         return (u_char) F_MODERATED;

      case 'n':
         return (u_char) F_NOPOSTING;

      default:
         return (u_char) 0;

   }

} /* encodeModeration */

/*--------------------------------------------------------------------*/
/*       t r a n s l a t e M o d e r a t i o n                        */
/*                                                                    */
/*       Translate bit flags to character moderation flag             */
/*--------------------------------------------------------------------*/

char
translateModeration( const u_char flag )
{
   switch( flag & ( F_MODERATED | F_UNMODERATED | F_NOPOSTING ))
   {
      case F_MODERATED:
         return 'm';

      case F_UNMODERATED:
         return 'y';

      case F_NOPOSTING:
         return 'n';

      default:
         return 'x';

   }

} /* translateModeration */

/*--------------------------------------------------------------------*/
/*       m a k e G r o u p N a m e                                    */
/*                                                                    */
/*       Given a leaf of the active group tree, make the name         */
/*       by recursively appending the prefix of the group name.       */
/*--------------------------------------------------------------------*/

static char *
makeGroupName( char *buf, GROUP UUFAR *group )
{

   if ( group->prevLevel == NULL )
      *buf = '\0';
   else {
      makeGroupName( buf, group->prevLevel );
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
                  current->prevLevel ?
                     getSimpleName( current->prevLevel ) : "(top)",
                  getSimpleName( current ) );

   printTree( current->right, "right", depth + 1 );

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

   printTree( current->left, "left", depth + 1 );

} /* printTree */

#endif

/*--------------------------------------------------------------------*/
/*       c r e a t e N o d e                                          */
/*                                                                    */
/*       Create current level as right node of "previous", or         */
/*       under previous level node if no previous node is specified.  */
/*--------------------------------------------------------------------*/

GROUP UUFAR *
createNode( const char *level,
            GROUP UUFAR *prevLevel,
            GROUP UUFAR *parent )
{

   static GROUP UUFAR *newNode     = NULL;
   static GROUP UUFAR *blockAnchor = NULL;
   static size_t blockGroups;          /* Entries used in block      */

/*--------------------------------------------------------------------*/
/*                    We need to create a new node                    */
/*--------------------------------------------------------------------*/

   if ( blockAnchor == NULL )
   {
      blockAnchor = MALLOC( sizeof (*newNode) * GROUPS_PER_BLOCK );
      checkref( blockAnchor );

#ifdef UDEBUG
      printmsg(4,"createNode: Allocated new block of %ld group names",
               (long) GROUPS_PER_BLOCK );
#endif

      blockGroups = 0;
   }

   newNode = blockAnchor++;
   blockGroups++;

   if ( blockGroups == GROUPS_PER_BLOCK ) /* Full block?             */
      blockAnchor = NULL;                 /* Get new one next pass   */

   MEMSET( newNode, 0, sizeof *newNode );

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

   if ( strlen( level ) >= sizeof newNode->n.name )
   {
      newNode->n.namePtr = newstr( level );
      SET_LONGGROUP( newNode );
   }
   else
      STRCPY( newNode->n.name, level );

   newNode->prevLevel = prevLevel;
   newNode->parent    = parent;

   setRed( newNode );

/*--------------------------------------------------------------------*/
/*                     Return newly created node                      */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   nodes++;
#endif

   return newNode;

} /* createNode */

/*--------------------------------------------------------------------*/
/*       L e f t R o t a t e                                          */
/*                                                                    */
/*       Rotate the specified branch to the left                      */
/*--------------------------------------------------------------------*/

GROUP UUFAR *
LeftRotate(GROUP UUFAR *root, GROUP UUFAR *current)
{
    GROUP UUFAR *ret = root;
    GROUP UUFAR *y = current->right;

    current->right = y->left;

    if (current->right)
        current->right->parent = (GROUP UUFAR *) current;

    y->parent = current->parent;

    if (current->parent) {
        if (current == current->parent->left)
            current->parent->left = y;
        else
            current->parent->right = y;
    }
    else
        ret = y;

    y->left = (GROUP UUFAR *) current;
    current->parent = y;

    return ret;

} /* LeftRotate */

/*--------------------------------------------------------------------*/
/*       R i g h t R o t a t e                                        */
/*                                                                    */
/*       Rotate current node in tree right                            */
/*--------------------------------------------------------------------*/

GROUP UUFAR *
RightRotate(GROUP UUFAR *root, GROUP UUFAR *current)
{
    GROUP UUFAR *ret = root;
    GROUP UUFAR *x = current->left;

    current->left = x->right;

    if (current->left)
        current->left->parent = (GROUP UUFAR *) current;

    x->parent = current->parent;

    if (current->parent) {

        if (current == current->parent->left)
          current->parent->left = x;
        else
          current->parent->right = x;

    }
    else
        ret = x;

    x->right = (GROUP UUFAR *) current;

    current->parent = x;

    return ret;
}

/*--------------------------------------------------------------------*/
/*       i n s e r t N o d e                                          */
/*                                                                    */
/*       Locate a node in the group tree, creating it if needed.      */
/*--------------------------------------------------------------------*/

static GROUP UUFAR *
insertNode( GROUP UUFAR *root, GROUP UUFAR *prevLevel, char *name )
{

   GROUP UUFAR *x = root;
   GROUP UUFAR *y = NULL;
   GROUP UUFAR *addMe;
   GROUP UUFAR *newNode;
   int hit = 0;
   char *level = makeLevelName( name );

/*--------------------------------------------------------------------*/
/*                   Locate the name if it exists                     */
/*--------------------------------------------------------------------*/

   while ( x != NULL )
   {
      hit = STRCMP( level, getSimpleName( x ) );

      y = x;

      if ( ! hit )                  /* Did we find the exact name?   */
         return x;                  /* Yes --> Return it to caller   */
      else if ( hit > 0 )           /* We greater than current?      */
         x = x->right;
      else
         x = x->left;

#ifdef UDEBUG
      siblings++;
#endif

   } /* while ( x != NULL ) */

   if ( y == NULL )
      addMe = root = createNode( level, prevLevel, NULL );
   else if ( hit > 0 )
      addMe = y->right = createNode( level, prevLevel, y );
   else
      addMe = y->left    = createNode( level, prevLevel, y );

   newNode = addMe;                 /* Save for caller               */

/*--------------------------------------------------------------------*/
/*                    Insure the tree is balanced                     */
/*--------------------------------------------------------------------*/

   while (addMe != root &&
          addMe->parent->parent &&
          isRed( addMe->parent )) {

       GROUP UUFAR *y;

       if (addMe->parent == addMe->parent->parent->left) {

           y = addMe->parent->parent->right;

           if (y && isRed( y )) {

               /* Case 1: x's uncle is red                            */

               setBlack( addMe->parent );
               setBlack( y );
               setRed( addMe->parent->parent );
               addMe = addMe->parent->parent;
           }
           else {
               if (addMe == addMe->parent->right) {

                   /* Case 2: x is a right child                      */
                   /* Rotate to transform to case 3                   */

                   addMe = addMe->parent;
                   root = LeftRotate(root, addMe);
               }

               /* Case 3: x is a left child                           */

               setBlack( addMe->parent );

               if (addMe->parent->parent) {

                   setRed( addMe->parent->parent );
                   root = RightRotate(root,addMe->parent->parent);

               }

               /* The while loop will terminate                       */
               /* on the next iteration.                              */
           }
       }
       else {

           y = addMe->parent->parent->left;

           if (y && isRed( y )) {

               setBlack( addMe->parent );
               setBlack( y );
               setRed( addMe->parent->parent );
               addMe = addMe->parent->parent;

           }
           else {

               if (addMe == addMe->parent->left) {
                 addMe = addMe->parent;
                 root = RightRotate(root,addMe);
               }

               setBlack( addMe->parent );

               if (addMe->parent->parent) {
                   setRed( addMe->parent->parent );
                   root = LeftRotate(root,addMe->parent->parent);
               }
           }
       }
   }

   setBlack( root );

/*--------------------------------------------------------------------*/
/*        Always keep our root pointer for this level current         */
/*--------------------------------------------------------------------*/

   if ( prevLevel == NULL )
      topNode = root;
   else
      prevLevel->nextLevel = root;

/*--------------------------------------------------------------------*/
/*                 Return the new node to the caller                  */
/*--------------------------------------------------------------------*/

   return newNode;

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
   GROUP UUFAR *prevLevel;
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
            return KWFalse;         /* Perfect match, group exists!   */
         }

         index += 1;                /* Step to next char in each string*/

      } /* while( fullName[index] == group [index] ) */

/*--------------------------------------------------------------------*/
/*    Back up the tree as far as we have to for the correct branch    */
/*--------------------------------------------------------------------*/

      if ( lastMatch == NULL)
         current = NULL;
      else {

         while( (lastMatch = strchr( lastMatch + 1 ,'.' )) != NULL )
         {
            current = current->prevLevel;
#ifdef UDEBUG
            backSteps++;
#endif
         }

/*--------------------------------------------------------------------*/
/*                Begin at front of siblings on insert                */
/*--------------------------------------------------------------------*/

         current = current->prevLevel;

         if ( current != NULL )
            current = current->nextLevel;

      } /* else */

#ifdef UDEBUG2
      printmsg(9,"Matched %s to %s for %d, %d levels didn't match.",
                  fullName,
                  group,
                  index,
                  backSteps );
#endif

#ifdef UDEBUG
      prevLevels += backSteps;
#endif

   } /* if ( current != NULL ) */

   if ( current == NULL )
   {
      current = topNode;
      prevLevel = NULL;
      level = (char *) group;
   }
   else
      prevLevel = current->prevLevel;

/*--------------------------------------------------------------------*/
/*        Loop to walk tree to node, adding branches as needed        */
/*--------------------------------------------------------------------*/

   for ( ;; )
   {

      current = insertNode( current, prevLevel, level );

      if ( current == NULL )
      {
         printmsg(0,"Cannot add group %s (component of name too long?)",
                     group );
         return KWFalse;
      }

      level = strchr( level, '.' ); /* Find next level of name       */

      if ( level != NULL )
      {
         prevLevel = current;       /* Remember prevLevel node       */
         current = prevLevel->nextLevel; /* First group on next level*/
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
                  GET_MODERATION( current ) );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                        Initialize the node                         */
/*--------------------------------------------------------------------*/

   current->high = high;
   current->low  = low;
   SET_GROUP( current );

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
                  current->prevLevel  ? getSimpleName( current->prevLevel ) :
                                     "(none)",
                  current->right ? ", sibling " : "",
                  current->right ? getSimpleName( current->right ) :
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

   if (topNode == NULL)
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
                prevLevels );
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
   GROUP UUFAR *nextLevel = topNode;

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
            current = current->right;
         else if ( hit < 0 )
            current = current->left;

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

         nextLevel = current->nextLevel;  /* Move down tree as well  */

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
/*       always just recursively walk the left hand node rather       */
/*       than attempting tail-end recursion.                          */
/*--------------------------------------------------------------------*/

      if (node->left != NULL)
         nextActiveGroup( node->left, walkOneGroup, optional );

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
/*       recursion to limit blowing the stack.                        */
/*--------------------------------------------------------------------*/

      if ( node->nextLevel != NULL )
      {

         if ( node->right == NULL )
            node = node->nextLevel;
         else {
            nextActiveGroup( node->nextLevel, walkOneGroup, optional );
            node = node->right;
         }
      }
      else
         node = node->right;

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
      nextActiveGroup( topNode, walkOneGroup, optional);

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

   if ( topNode == NULL )
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

   nextActiveGroup( topNode, 0, (void *) stream );

   fclose( stream );

#ifdef UDEBUG

   if ( searches > cacheHits )
   {
      long realSearches = searches - cacheHits;

      printmsg( 1, "writeActive: %ld searches, %ld cache hits, "
                   "%ld tree searches, maximum depth %ld.",
                   searches,
                   cacheHits,
                   realSearches,
                   maxPushed);

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
