/*--------------------------------------------------------------------*/
/*    s t r p o o l . c                                               */
/*                                                                    */
/*    String dynamic literal pool management for UUPC/extended        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*
 *    $Id: strpool.c 1.7 1994/02/20 19:07:38 ahd Exp $
 *
 *    $Log: strpool.c $
 *     Revision 1.7  1994/02/20  19:07:38  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.6  1993/12/24  05:12:54  ahd
 *     New format for checkptr call
 *
 *     Revision 1.5  1993/10/12  00:48:44  ahd
 *     Normalize comments
 *
 *     Revision 1.4  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 * Revision 1.3  1992/12/04  01:00:27  ahd
 * Delete allocating pool message; now handled by pools allocated summary
 *
 * Revision 1.2  1992/12/01  04:37:03  ahd
 * Add SpeedOverMemory
 *
 * Revision 1.1  1992/11/22  20:58:55  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <limits.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          Local structures                          */
/*--------------------------------------------------------------------*/

typedef struct str_queue {
   struct str_queue *next_link;
   size_t used;
   char pool[BUFSIZ - sizeof (size_t) - sizeof (struct str_queue *) ];
}  STR_QUEUE;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static STR_QUEUE *anchor = NULL;
static const size_t pool_size = sizeof anchor->pool;
static int pools      = 0;

#ifdef UDEBUG

static int strings    = 0;
static int used       = 0;
static int duplicates = 0;
static int saved      = 0;

#endif

/*--------------------------------------------------------------------*/
/*    The problem:  UUPC/extended allocates large number of small     */
/*    (<< 50 characters) string variables off the heap which are      */
/*    never modified and never deallocated.  This means that the      */
/*    possibly duplicate and relatively large overhead required by    */
/*    malloc and causes these variables to waste space.               */
/*                                                                    */
/*    The solution:  We use this routine to maintain our own local    */
/*    pool of storage for allocating NULL terminated strings out      */
/*    of a chain of large buffers.  This allows us to both reduce     */
/*    storage overhead by placing the strings end to end, and to      */
/*    optionally scan the list for duplicate entries.                 */
/*                                                                    */
/*    The duplicate string search can be questionable on a small      */
/*    slow system, because we have to walk the entire list to locate  */
/*    the duplicate.  However, a smaller system is less likely to     */
/*    have a large dynamic string pool, so we take the hit anyway.    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    s t r p o o l                                                   */
/*                                                                    */
/*    Allocate a string from the string poll                          */
/*--------------------------------------------------------------------*/

char *strpool( const char *input , const char *file, size_t line)
{
   int len;
   int best_fit = SHRT_MAX;
   char *result;

   STR_QUEUE *current = anchor;
   STR_QUEUE *last    = anchor;
   STR_QUEUE *save    = NULL;

#ifdef __DEBUG_ALLOC__
   _heap_check();
#endif

   if ( input == NULL )
   {
      printmsg(0,"strpool: NULL pointer passed to newstr()");
      bugout( line, file );      /* Become Info Highway Roadkill     */
   }

   len  = strlen( input );

/*--------------------------------------------------------------------*/
/*                      Perform best fit search                       */
/*--------------------------------------------------------------------*/

   while(current != NULL )
   {
      int available;

/*--------------------------------------------------------------------*/
/*                 Scan current buffer for the string                 */
/*--------------------------------------------------------------------*/

      if ( ! bflag[ F_SPEEDOVERMEMORY ] )
      {
         char *target = current->pool;
         char *bufend = target + current->used;

         while( target < bufend )
         {
            int target_len = strlen( target );
            int diff =  target_len - len;

            if ((diff >= 0 ) && equal( target + diff, input))
            {

#ifdef UDEBUG
               duplicates ++;
               saved += len + 1;
#endif
               return target+diff;
            }

            target += target_len + 1;  /* Step to start of next string */

         } /* while( offset < current->used ) */
      }  /* if */

/*--------------------------------------------------------------------*/
/*    No string in this buffer, look for best fit in case we need     */
/*    to allocate the string from scratch                             */
/*--------------------------------------------------------------------*/

      available = pool_size - current->used;

      if (( available < best_fit) && (available > len ))
      {
         best_fit = available;
         save     = current;
      }
      else
         last =  current;        /* Save last buffer in case we
                                    have to chain new buffer in       */
      current = current->next_link;
   }  /* while */

/*--------------------------------------------------------------------*/
/*    We have no matching string, we have to insert the new string    */
/*    into our pool                                                   */
/*--------------------------------------------------------------------*/

   if ( save == NULL )           /* We find a buffer?                 */
   {                             /* No --> Allocate a new one         */
      pools ++;

      save = malloc( sizeof *save );
      if ( !save) checkptr( file, line);

      if ( anchor == NULL )
      {

#ifdef UDEBUG
         atexit( dump_pool );
#endif

         anchor = save;
      }
      else
         last->next_link = save;

      save->used = 0;
      save->next_link = NULL;
   }

/*--------------------------------------------------------------------*/
/*    Save the string, update memory available in current pool,       */
/*    and return to the caller with the new string                    */
/*--------------------------------------------------------------------*/

   result = strcpy( save->pool + save->used, input );
   save->used += len + 1;

#ifdef UDEBUG
   strings ++;
   used    += len + 1;
#endif

   return result;

 } /* strpool */

/*--------------------------------------------------------------------*/
/*    s a f e f r e e                                                 */
/*                                                                    */
/*    Insure we are not freeing memory saved for a pool               */
/*--------------------------------------------------------------------*/

void safefree( void *input , const char *file, size_t line)
{
   STR_QUEUE *current = anchor;
   int buffers = 0;
   while( current != NULL )
   {
      buffers ++;
      if (( input > (void *) current ) &&
          (input < (void *) (current->pool + pool_size ) ))
      {
         printmsg(0,"Attempt to free string \"%s\" allocated via newstr() in pool %d",
                  input , buffers );

         bugout( line, file);
      }

      current = current->next_link;
   }

#undef free

   free(input);

} /* safefree */

#ifdef UDEBUG

/*--------------------------------------------------------------------*/
/*    d u m  p _ p o o l                                              */
/*                                                                    */
/*    Print the free storage pool                                     */
/*--------------------------------------------------------------------*/

void dump_pool( void )
{

   STR_QUEUE *current = anchor;
   int buffers = 0;

   printmsg(3,"Allocated %d bytes in %d strings "
              "requiring %d pools of %d bytes each",
              used, strings, pools, pool_size );

   if ( duplicates )
      printmsg(3,"Saved %d bytes in %d redundant strings",
               saved, duplicates);

   if ( debuglevel >= 5 )
   while(current != NULL )
   {
      size_t offset = 0;
      size_t strings = 0;
      buffers ++;

      printmsg(5,"Buffer %d length is %d bytes",buffers, current->used);

      while( offset < current->used )
      {
         size_t target_len = strlen( current->pool + offset );
         strings ++;
         printmsg(5,"[%d,%02d,%02d]=\"%s\"",
                     buffers,
                     strings,
                     target_len,
                     current->pool + offset);
         offset += target_len +1;  /* Go to end of string             */

      } /* while( offset < current->used ) */

      current = current->next_link;

   }  /* while */

#ifdef __DEBUG_ALLOC__
   _dump_allocated( 16 );
#endif

} /* dump_pool */

#endif
