/*--------------------------------------------------------------------*/
/*    s t r p o o l . c                                               */
/*                                                                    */
/*    String dynamic literal pool management for UUPC/extended        */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks; all        */
/*    rights reserved except those explicitly granted by the          */
/*    UUPC/extended license.                                          */
/*--------------------------------------------------------------------*/

/*
 *    $Id$
 *
 *    $Log$
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"

/*--------------------------------------------------------------------*/
/*                          Local structures                          */
/*--------------------------------------------------------------------*/

typedef struct str_queue {
   struct str_queue *next_link;
   size_t used;
   char pool[BUFSIZ];
}  STR_QUEUE;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

void dump_pool( void );

static STR_QUEUE *anchor = NULL;
static const size_t pool_size = BUFSIZ;

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
/*    optionally scan the list for duplicates entries.                */
/*                                                                    */
/*    The duplicate string search can be questionable on a small      */
/*    system, because we have to walk the entire list to locate       */
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
   int len  = strlen( input );
   char *result;
   size_t best_fit = SHRT_MAX;
   size_t buffers = 0;

   STR_QUEUE *current = anchor;
   STR_QUEUE *last    = anchor;
   STR_QUEUE *save    = NULL;

/*--------------------------------------------------------------------*/
/*                      Perform best fit search                       */
/*--------------------------------------------------------------------*/

   while(current != NULL )
   {
      size_t available;
      char *target = current->pool;
      char *bufend = target + current->used;

/*--------------------------------------------------------------------*/
/*                 Scan current buffer for the string                 */
/*--------------------------------------------------------------------*/

      while( target < bufend )
      {
         int target_len = strlen( target );
         int diff =  target_len - len;

         if ((diff >= 0 ) && equal( target + diff, input))
            return target+diff;

         target += target_len + 1;  /* Step to start of next string  */

      } /* while( offset < current->used ) */

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
                                    have to chain new buffer in      */
      buffers ++;
      current = current->next_link;
   }  /* while */

/*--------------------------------------------------------------------*/
/*    We have no matching string, we have to insert the new string    */
/*    into our pool                                                   */
/*--------------------------------------------------------------------*/

   if ( save == NULL )           /* We find a buffer?                */
   {                             /* No --> Allocate a new one        */

      save = malloc( sizeof *save );
      checkptr(save, file, line);

      if ( anchor == NULL )
      {
         if ( debuglevel > 4 )
            atexit( dump_pool );
         anchor = save;
      }
      else
         last->next_link = save;

      save->used = 0;
      save->next_link = NULL;

      printmsg( 2,"strpool: Allocated pool %d,"
                  " input from %s(%d) is \"%s\"",
                  buffers + 1, file, line, input );
   }

/*--------------------------------------------------------------------*/
/*    Save the string, update memory available in current pool,       */
/*    and return to the caller with the new string                    */
/*--------------------------------------------------------------------*/

   result = strcpy( save->pool + save->used, input );
   save->used += len + 1;

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

/*--------------------------------------------------------------------*/
/*    d u m  p _ p o o l                                              */
/*                                                                    */
/*    Print the free storage pool                                     */
/*--------------------------------------------------------------------*/

void dump_pool( void )
{

   STR_QUEUE *current = anchor;
   int buffers = 0;

   printmsg(0,"Dumping free storage pool ...");

   while(current != NULL )
   {
      size_t offset = 0;
      size_t strings = 0;
      buffers ++;

      printmsg(0,"Buffer %d length is %d bytes",buffers, current->used);

      while( offset < current->used )
      {
         size_t target_len = strlen( current->pool + offset );
         strings ++;
         printmsg(0,"[%d,%02d,%02d]=\"%s\"",
                     buffers,
                     strings,
                     target_len,
                     current->pool + offset);
         offset += target_len +1;  /* Go to end of string            */

      } /* while( offset < current->used ) */

      current = current->next_link;

   }  /* while */
} /* dump_pool */

