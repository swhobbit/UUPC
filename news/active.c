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
 *    $Id: active.c 1.33 1996/01/27 16:49:22 ahd Exp $
 *
 *    Revision history:
 *    $Log: active.c $
 *    Revision 1.33  1996/01/27 16:49:22  ahd
 *    Correct display moderation for duplicate groups
 *
 *    Revision 1.32  1996/01/07 14:14:40  ahd
 *    Dynamically allocate space for news group name trailing rest of
 *    active tree node, eliminating multiple level names entirely.
 *
 *    Revision 1.31  1996/01/04 03:59:30  ahd
 *    Perform stronger news group lemgth checking
 *    Delete extra cannot load news group message
 *
 *    Revision 1.30  1996/01/02 02:51:53  ahd
 *    Correct conditional debugging code to be consistent
 *
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

#define F_COLOR_RED       0x80
#define F_VALID_GROUP     0x40

#define F_MODERATED       0x20
#define F_UNMODERATED     0x10
#define F_NOPOSTING      (F_MODERATED | F_UNMODERATED)

typedef struct _GROUP
{

   struct _GROUP UUFAR *left;
   struct _GROUP UUFAR *right;
   struct _GROUP UUFAR *parent;

   long   high;                     /* Next article number to store  */
   long   low;                      /* Lowest unexpired article num  */

   unsigned char flags;             /* Bit flags, see above          */
   char name[3];                    /* First character of name       */

} GROUP;

#define BLOCK_SIZE           0x2000

static GROUP UUFAR *cachedGroup = NULL;   /* Last group walked       */

static GROUP UUFAR *topNode;        /* Top of group tree             */

currentfile();

static long groups = 0;             /* Total groups loaded           */
static long deletes = 0;            /* Deletes during processing     */
static long walked = 0;             /* Nodes written during output   */

#ifdef UDEBUG

static long siblings = 0;           /* Nodes visited during load     */

static long searches = 0;           /* Number of searches performed  */
static long cacheHits = 0;          /* Number of cache hits in search*/
static long searchNodes = 0;        /* Nodes walked during search    */

static long pushed = 0;             /* Current uses of pushGroup     */
static long maxPushed = 0;          /* MAx uses of pushGroup         */

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
/*       c r e a t e N o d e                                          */
/*                                                                    */
/*       Insert new node into our tree                                */
/*--------------------------------------------------------------------*/

GROUP UUFAR *
createNode( const char *groupName,
            GROUP UUFAR *parent )
{

   static char UUFAR *anchor = NULL;
   static size_t freeBytes = 0;      /* Bytes available in block   */

#ifdef UDEBUG
   static size_t bufferCount = 0;
#endif

   GROUP UUFAR *newNode;
   size_t length = strlen( groupName ) +
                   1 +
                   sizeof *newNode -
                   sizeof newNode->name;

/*--------------------------------------------------------------------*/
/*                    We need to create a new node                    */
/*--------------------------------------------------------------------*/

   if ( freeBytes < (sizeof (GROUP) + length ))
   {
      anchor = (char UUFAR *) MALLOC( BLOCK_SIZE );
      checkref( anchor );
      freeBytes = BLOCK_SIZE;

#ifdef UDEBUG
      printmsg(4, "createNode: Allocated block[%d] for %d bytes at "
#ifdef BIT32ENV
                   "%p"
#else
                   "%Fp"
#endif
                   " for group %ld",
               ++bufferCount,
               freeBytes,
               anchor,
               groups + 1 );
#endif

   }

   length  += sizeof (GROUP UUFAR *) - (length % sizeof (GROUP UUFAR *));
                                    /* Round to next boundary           */
   newNode = (GROUP UUFAR *) anchor;

   if ( length <= freeBytes )       /* Don't allow unsigned number to
                                       go negative because of rounding  */
   {
      anchor  += length;
      freeBytes -= length;
   }
   else
      freeBytes = 0;

   MEMSET( newNode, 0, sizeof *newNode );

   newNode->parent = parent;

   setRed( newNode );
   STRCPY( newNode->name, groupName );

/*--------------------------------------------------------------------*/
/*                     Return newly created node                      */
/*--------------------------------------------------------------------*/

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
insertNode( const char *name )
{

   GROUP UUFAR *x = topNode;
   GROUP UUFAR *y = NULL;
   GROUP UUFAR *addMe;
   GROUP UUFAR *newNode;
   int hit = 0;

/*--------------------------------------------------------------------*/
/*                   Locate the name if it exists                     */
/*--------------------------------------------------------------------*/

   while ( x != NULL )
   {
      hit = STRCMP( name, x->name);

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
      addMe = topNode  = createNode( name, NULL );
   else if ( hit > 0 )
      addMe = y->right = createNode( name, y );
   else
      addMe = y->left  = createNode( name, y );

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

   static size_t maxLength = 0;

/*--------------------------------------------------------------------*/
/*       Verify group name can be used on our file system.  The       */
/*       group name must fit in the standard buffer, and also on      */
/*       the file system, including any periods we need to insert.    */
/*--------------------------------------------------------------------*/

   if ( maxLength == 0 )
   {
      maxLength = FILENAME_MAX - (strlen( E_newsdir ) + (MAXGRP / 9));
                                    /* The 1/9 allows for an inserted
                                       period every nine characters. */

      if ( MAXGRP < maxLength )
         maxLength = MAXGRP;
   }

   if ( strlen( group ) >= maxLength )
   {
      printmsg(0, "addGroup: news group too long, "
                  "maximum allowed length is %d: %s",
                  maxLength,
                  group );
      return KWFalse;
   }

   current = insertNode( group );

/*--------------------------------------------------------------------*/
/*     If the node is alway initialized, return failure to caller     */
/*--------------------------------------------------------------------*/

   if ( IS_GROUP( current ))
   {
      printmsg(0, "Group %s already found, moderation status is %c",
                  group,
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

      addGroup( group, high, low, *s );

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
   printmsg( 1, "loadActive: %ld groups loaded via %ld siblings.",
                groups,
                siblings );

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

   if (( cachedGroup != NULL ) &&  !STRCMP( cachedGroup->name , group ))
   {
#ifdef UDEBUG
      cacheHits++;
#endif /* UDEBUG */

      return cachedGroup;

   } /* if */

/*--------------------------------------------------------------------*/
/*              Inner loop to locate simple name in list              */
/*--------------------------------------------------------------------*/

   while( current != NULL )
   {
      int hit = STRCMP( group, current->name);

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
setArticleNewest( const char *name, const long newSequence )
{
   GROUP UUFAR *group = findGroup( name );

   if (( group != NULL ) && IS_GROUP( group ))
   {
      group->high = newSequence;
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
setArticleOldest( const char *name, const long newSequence )
{
   GROUP UUFAR *group = findGroup( name );

   if (( group != NULL ) && IS_GROUP( group ))
   {
      group->low = newSequence;
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

         STRCPY( buf, node->name);

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

            (*walkOneGroup)( buf, optional );
         }

      } /* if ( IS_GROUP( node ) ) */

/*--------------------------------------------------------------------*/
/*       For the last node at this level, we perform tail end         */
/*       recursion to limit blowing the stack.                        */
/*--------------------------------------------------------------------*/

      node = node->right;

   } while ( node != NULL );

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
                   "maximum recursion %ld.",
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
