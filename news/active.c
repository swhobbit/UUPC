/*--------------------------------------------------------------------*/
/*       a c t i v e . c                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
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
 *    $Id: active.c 1.29 1996/01/01 23:57:29 ahd Exp $
 *
 *    Revision history:
 *    $Log: active.c $
 *    Revision 1.29  1996/01/01 23:57:29  ahd
 *    Use one large red/black tree for entire active file, not small trees at each level
 *
 *    Revision 1.28  1995/12/31 21:16:57  ahd
 *    Encode moderation as single byte
 *
 *    Revision 1.27  1995/12/26 16:55:43  ahd
 *    Use red/black tree for active entries
 *
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
#include <malloc.h>

#include "active.h"

/*--------------------------------------------------------------------*/
/*           Our active tree structure as a red/black tree            */
/*--------------------------------------------------------------------*/

#define F_COLOR_RED       0x01
#define F_VALID_GROUP     0x02
#define F_LONGGROUP       0x04

#define F_MODERATED       0x10
#define F_UNMODERATED     0x20
#define F_NOPOSTING       0x40

#if defined( BIT32ENV ) || defined( FAMILYAPI )
#define LEVEL_LENGTH     31
#else
#define LEVEL_LENGTH     19
#endif

typedef struct _GROUP
{

   struct _GROUP UUFAR *left;       /* Binary tree at this level     */
   struct _GROUP UUFAR *right;      /* Binary tree at this level     */
   struct _GROUP UUFAR *parent;     /* Parent node in current level  */

   struct _GROUP UUFAR *prevLevel;  /* Group at previous level       */

   long   high;                     /* Next article number to store  */
   long   low;                      /* Lowest unexpired article num  */

   union
   {
      char   UUFAR *namePtr;        /* Pointer to simple name        */
      char   name[LEVEL_LENGTH];    /* Simple name                   */
   } n;

   unsigned char flags;             /* Bit flags, see above          */

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

static long searches = 0;           /* Number of searches performed  */
static long cacheHits = 0;          /* Number of cache hits in search*/
static long searchNodes = 0;        /* Nodes walked during search    */

static long pushed = 0;             /* Current uses of pushGroup     */
static long maxPushed = 0;          /* MAx uses of pushGroup         */

static long longNames = 0;          /* Names which didn't fit        */
static long parents   = 0;          /* Parents we had to create      */

#endif

/*--------------------------------------------------------------------*/
/*                    Moderation/valid group flags                    */
/*--------------------------------------------------------------------*/

#define bitOn( _group, _bitName ) (_group->flags = (unsigned char) \
         (_group->flags | (unsigned char) _bitName ))

#define bitOff( _group, _bitName ) (_group->flags = (unsigned char) \
         (_group->flags & (unsigned char) (0xff - _bitName) ))

#define GET_MODERATION( _group )    translateModeration( _group->flags )
#define SET_MODERATION( _group, _c ) bitOn( _group, encodeModeration( _c ))

#define IS_GROUP( _group )       ( _group->flags & F_VALID_GROUP )
#define SET_NOT_GROUP( _group )  bitOff( _group, F_VALID_GROUP )
#define SET_GROUP( _group )      bitOn( _group, F_VALID_GROUP )

#define setBlack( _group ) bitOff( _group, F_COLOR_RED )
#define setRed( _group )   bitOn( _group, F_COLOR_RED )
#define isRed( _group )    ( _group->flags & F_COLOR_RED )

#define SET_LONGGROUP( _group )   bitOn( _group, F_LONGGROUP )
#define IS_LONGGROUP( _group )    ( _group->flags & F_LONGGROUP )

#define containsFullName( _group )    (_group->prevLevel == NULL)

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

unsigned char
encodeModeration( const char moderation )
{
   switch( moderation )
   {
      case 'y':
         return (unsigned char) F_UNMODERATED;

      case 'm':
         return (unsigned char) F_MODERATED;

      case 'n':
         return (unsigned char) F_NOPOSTING;

      default:
         return (unsigned char) 0;

   }

} /* encodeModeration */

/*--------------------------------------------------------------------*/
/*       t r a n s l a t e M o d e r a t i o n                        */
/*                                                                    */
/*       Translate bit flags to character moderation flag             */
/*--------------------------------------------------------------------*/

char
translateModeration( const unsigned char flag )
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

   if ( group->prevLevel != NULL )
   {
      buf = makeGroupName( buf, group->prevLevel );
      *buf++ = '.';
   }

   STRCPY( buf, getSimpleName( group ) );
   buf += strlen( buf );

   return buf;                      /* first unused character        */

} /* makeGroupName */

/*--------------------------------------------------------------------*/
/*       m a k e L e v e l                                            */
/*                                                                    */
/*       Make a simple level name                                     */
/*--------------------------------------------------------------------*/

static const char *
makeLevelName( const char *name )
{
   static char level[MAXGRP];
   char *p;
   char *period = NULL;
   size_t length = strlen( name );

/*--------------------------------------------------------------------*/
/*                       Create the level name                        */
/*--------------------------------------------------------------------*/

   if ( length < LEVEL_LENGTH )
      return name;

/*--------------------------------------------------------------------*/
/*       As we search for a good breaking point, we never allow a     */
/*       trailing period to be the only character left in the         */
/*       buffer after the current level, as this could not be         */
/*       reconstructed by makeGroupName().                            */
/*--------------------------------------------------------------------*/


   p = (char *) name + (LEVEL_LENGTH - 1);

   do {

      if ( *p == '.' )
         period = p;

   } while ((period == NULL ) && ( --p > name ));

   if ( period == NULL )
      period = strchr( name + LEVEL_LENGTH, '.' );

/*--------------------------------------------------------------------*/
/*       If there is no period (other than a possible trailing        */
/*       period) return the full string for the current level.        */
/*--------------------------------------------------------------------*/

   if (( period == NULL ) || (period == (name + length - 1 )))
      return name;

/*--------------------------------------------------------------------*/
/*     We have a valid termination point, copy the string as needed   */
/*--------------------------------------------------------------------*/

   length = (size_t) (period - name);
   memcpy( level, name, length );
   level[length] = '\0';

   return level;

} /* makeLevelName */

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

   static GROUP UUFAR *blockAnchor = NULL;
   static size_t blockGroups;          /* Entries used in block      */

   size_t length = strlen( level );
   GROUP UUFAR *newNode;

/*--------------------------------------------------------------------*/
/*                    We need to create a new node                    */
/*--------------------------------------------------------------------*/

   if ( blockAnchor == NULL )
   {
      blockAnchor = MALLOC( sizeof (*blockAnchor) * GROUPS_PER_BLOCK );
      checkref( blockAnchor );

#ifdef UDEBUG
      printmsg(4, "createNode: Allocated new block of %ld group names",
               (long) GROUPS_PER_BLOCK );
#endif

      blockGroups = 0;
   }

   newNode = blockAnchor++;
   blockGroups++;

   if ( blockGroups == GROUPS_PER_BLOCK ) /* Full block?             */
      blockAnchor = NULL;                 /* Get new one next pass   */

   MEMSET( newNode, 0, sizeof *newNode );

   newNode->prevLevel = prevLevel;
   newNode->parent    = parent;

   setRed( newNode );

/*--------------------------------------------------------------------*/
/*       We store the string into the actual structure if it fits,    */
/*       otherwise (for longer strings) we grab a string off our      */
/*       off far string memory.  This prevents a near heap shortage   */
/*       in 16 bit environments but may limit the total number of     */
/*       groups in a 640K (DOS) environment.                          */
/*--------------------------------------------------------------------*/

   if ( length >= sizeof newNode->n.name )
   {
#ifdef BIT32ENV
      newNode->n.namePtr = newstr( level );
#else
      newNode->n.namePtr = MALLOC( length + 1 );
      checkref( newNode->n.namePtr );
      STRCPY( newNode->n.namePtr, level );
#endif
      SET_LONGGROUP( newNode );

#ifdef UDEBUG
      longNames++;
      printmsg(2,"createNode: overlength name (%d bytes, %d allowed): %s",
                  (size_t) length,
                  (size_t) sizeof newNode->n.name - 1,
                  level );
#endif

   }
   else
      STRCPY( newNode->n.name, level );

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
insertNode( GROUP UUFAR *prevLevel, const char *name )
{

   GROUP UUFAR *x = topNode;
   GROUP UUFAR *y = NULL;
   GROUP UUFAR *addMe;
   GROUP UUFAR *newNode;
   char fullName[MAXGRP];
   int hit = 0;

/*--------------------------------------------------------------------*/
/*                    Make name used for comparing                    */
/*--------------------------------------------------------------------*/

   if ( prevLevel == NULL )
      strcpy( fullName, name );
   else {
      char *end = makeGroupName( fullName, prevLevel );
      *end++ = '.';
      strcpy( end, name );
   }

/*--------------------------------------------------------------------*/
/*                   Locate the name if it exists                     */
/*--------------------------------------------------------------------*/

   while ( x != NULL )
   {
      if ( containsFullName( x ))
         hit = STRCMP( fullName, getSimpleName( x ));
      else {
         char buf[MAXGRP];
         makeGroupName( buf, x );
         hit = strcmp( fullName, buf );

#ifdef UDEBUG
         parents++;
#endif

      } /* else */

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
      addMe = topNode  = createNode( name, prevLevel, NULL );
   else if ( hit > 0 )
      addMe = y->right = createNode( name, prevLevel, y );
   else
      addMe = y->left  = createNode( name, prevLevel, y );

   newNode = addMe;                 /* Save for caller               */

/*--------------------------------------------------------------------*/
/*                    Insure the tree is balanced                     */
/*--------------------------------------------------------------------*/

   while (addMe != topNode &&
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
                   topNode = LeftRotate(topNode, addMe);
               }

               /* Case 3: x is a left child                           */

               setBlack( addMe->parent );

               if (addMe->parent->parent) {

                   setRed( addMe->parent->parent );
                   topNode = RightRotate(topNode, addMe->parent->parent);

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
                 topNode = RightRotate(topNode, addMe);
               }

               setBlack( addMe->parent );

               if (addMe->parent->parent) {
                   setRed( addMe->parent->parent );
                   topNode = LeftRotate(topNode, addMe->parent->parent);
               }
           }
       }
   }

   setBlack( topNode );

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
   GROUP UUFAR *current = NULL;
   GROUP UUFAR *prevLevel;
   char  *rest = (char *) group;

/*--------------------------------------------------------------------*/
/*                   Verify grouop name can be used                   */
/*--------------------------------------------------------------------*/

   if ( strlen( group ) >= MAXGRP )
   {
      printmsg(0, "addGroup: Name too long to use as news group: %s",
                 group );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*        Loop to walk tree to node, adding branches as needed        */
/*--------------------------------------------------------------------*/

   while( *rest )
   {
      const char *level = makeLevelName( rest );

      prevLevel = current;
      rest += strlen( level );

      if ( *rest == '.' )
         rest += 1;

      current = insertNode( prevLevel, level );

   } /* while( *rest ) */

/*--------------------------------------------------------------------*/
/*     If the node is alway initialized, return failure to caller     */
/*--------------------------------------------------------------------*/

   if ( IS_GROUP( current ))
   {
      char buf[MAXGRP];
      makeGroupName( buf, current ),

      printmsg(0, "Group %s already found as %s, moderation status is %c",
                  group,
                  buf,
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
         printmsg(0, "addGroup: Invalid moderation flag %c for group %s, "
                    "changed to x",
                     (char) (isgraph( moderation ) ? moderation : '?'),
                     group );
         SET_MODERATION(current, 'x' );
         break;

   } /* switch( moderation ) */

/*--------------------------------------------------------------------*/
/*                    Return success to the caller                    */
/*--------------------------------------------------------------------*/

   groups++;

#ifdef UDEBUG

   {
      char buf[MAXGRP];

      makeGroupName( buf, current );

      if ( ! equal( buf, group))
      {
         printmsg(0, "addGroup: Group %s added in wrong place as %s",
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
   stream = FOPEN(fname, "r", TEXT_MODE);

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

      if (fgets(buf, BUFSIZ, stream) == NULL)
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
         printmsg(0, "loadActive: Unable to load group (duplicate?): %s",
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
      printmsg(0, "active: Active file %s is empty, cannot continue",
                  fname );
      panic();
   }

#ifdef UDEBUG
   printmsg( 1, "loadActive: %ld groups in %ld nodes, "
                "via %ld siblings (%ld parented).  "
                "%ld overlength names.",
                groups,
                nodes,
                siblings,
                parents,
                longNames );

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

   GROUP UUFAR *current = topNode;

#ifdef UDEBUG
   searches++;
#endif

   if ( name == NULL )
   {
      printmsg(0, "findGroup: Invalid call (null pointer parameter)");
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
/*              Inner loop to locate simple name in list              */
/*--------------------------------------------------------------------*/

   while( current != NULL )
   {
      int hit;

      if ( containsFullName( current ))
         hit = STRCMP( group, getSimpleName( current ));
      else {

         char buf[MAXGRP];

         makeGroupName( buf, current );

         hit = strcmp( group, buf );

      }  /* else */

#ifdef UDEBUG
      searchNodes++;
#endif

      if ( ! hit )               /* Did we find the exact name?   */
         break;                  /* Go to next level of tree      */

      if ( hit > 0 )
         current = current->right;
      else if ( hit < 0 )
         current = current->left;

   } /* while( current != NULL ) */

   if ( current == NULL )
   {

#ifdef UDEBUG
      printmsg(6, "findGroup: Did not find group %s", group );
#endif

      return NULL;
   }

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

      printmsg( 1, "writeActive: %ld searches, %ld search cache hits, "
                   "maximum tree depth %ld.",
                   searches,
                   cacheHits,
                   maxPushed);

      printmsg( 1, "writeActive: "
                   "%ld tree searches, "
                   "%ld nodes (%ld.%ld/search) ",
                   realSearches,
                   searchNodes,
                   searchNodes / realSearches,
                   ((searchNodes * 10) / realSearches + 5) % 10 );
   }
#endif

   if ( groups != ( walked + deletes ))
   {
      printmsg(0, "Internal error: Loaded %ld groups, "
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
