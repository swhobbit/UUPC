/*--------------------------------------------------------------------*/
/*    s t r p o o l . c                                               */
/*                                                                    */
/*    String dynamic literal pool management for UUPC/extended        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*
 *    $Id: strpool.c 1.22 2001/03/12 13:52:56 ahd v1-13k $
 *
 *    $Log: strpool.c $
 *    Revision 1.22  2001/03/12 13:52:56  ahd
 *    Annual copyright update
 *
 *    Revision 1.21  2000/05/12 12:29:45  ahd
 *    Annual copyright update
 *
 *    Revision 1.20  1999/01/08 02:20:43  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.19  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.18  1998/11/24 03:08:50  ahd
 *    Use KW_BUFSIZ for strpool size
 *
 *    Revision 1.17  1998/03/01 01:25:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.16  1997/11/21 18:15:18  ahd
 *    Command processing stub SMTP daemon
 *
 *    Revision 1.15  1997/03/31 07:07:02  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.14  1996/11/18 04:46:49  ahd
 *    Normalize arguments to bugout
 *    Reset title after exec of sub-modules
 *    Normalize host status names to use HS_ prefix
 *
 *    Revision 1.13  1996/01/01 20:53:36  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1995/02/20 00:40:12  ahd
 *    Correct C compiler warnings
 *
 *    Revision 1.11  1995/01/09 12:35:15  ahd
 *    Correct VC++ compiler warnings
 *
 *    Revision 1.10  1994/12/22 00:11:31  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1994/04/24 20:35:08  ahd
 *    Optimize processing by saving length of strings
 *    Insure extremely long strings are allocated properly
 *
 *     Revision 1.8  1994/02/21  16:38:58  ahd
 *     Delete C++ style comment
 *
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

RCSID("$Id: strpool.c 1.22 2001/03/12 13:52:56 ahd v1-13k $");

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          Local structures                          */
/*--------------------------------------------------------------------*/

typedef struct str_queue {
   struct str_queue *next_link;
   size_t used;
   char pool[KW_BUFSIZ - sizeof (size_t) - sizeof (struct str_queue *) ];
}  STR_QUEUE;

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static STR_QUEUE *anchor = NULL;
static const size_t pool_size = sizeof anchor->pool;
static int pools      = 0;

#ifdef UDEBUG

static int strings    = 0;
static int duplicates = 0;
static long used      = 0;
static long saved     = 0;

#endif

/*--------------------------------------------------------------------*/
/*    The problem:  UUPC/extended allocates large number of small     */
/*    (<< 50 characters) string variables off the heap which are      */
/*    never modified and never deallocated.  This means that the      */
/*    possibly duplicate and relatively large overhead required by    */
/*    malloc causes these variables to waste space.                   */
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

char *strpool( const char *input , const char UUFAR *file, size_t line)
{
   unsigned len;
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
      bugout( file, line );      /* Become Info Highway Roadkill     */
   }

   len  = strlen( input );

/*--------------------------------------------------------------------*/
/*                     Handle over length strings                     */
/*--------------------------------------------------------------------*/

   if ( len > UCHAR_MAX )
   {
      result = strdup( input );

      if ( !result)
         checkptr( file, line);
      return result;
  }

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
            int target_len = (unsigned char) *target++;
            int diff =  target_len - (int) len;

            if ((diff >= 0 ) && equal( target + diff, input))
            {

#ifdef UDEBUG
               duplicates ++;
               saved += (long) len + 2;
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

      available = (int) (pool_size - current->used);

      if (( available < (int) best_fit) && (available > (int) (len+1) ))
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

      if ( !save)
         checkptr( file, line);

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

   result = save->pool + save->used;
   *result = (char) ((unsigned char) len);
   strcpy( ++result, input );
   save->used += len + 2;

#ifdef UDEBUG
   strings ++;
   used    += (long) len + 2;
#endif

   return result;

 } /* strpool */

/*--------------------------------------------------------------------*/
/*    s a f e f r e e                                                 */
/*                                                                    */
/*    Insure we are not freeing memory saved for a pool               */
/*--------------------------------------------------------------------*/

void safefree( void *input , const char UUFAR *file, size_t line)
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

         bugout( file, line);
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

   printmsg(3,"Allocated %ld bytes in %d strings "
              "requiring %d pools of %d bytes each",
              used, strings, pools, pool_size );

   if ( duplicates )
      printmsg(3,"Saved %ld bytes in %d redundant strings",
               saved, duplicates);

   if ( debuglevel >= 10 )
   while(current != NULL )
   {
      size_t offset = 0;
      size_t strings = 0;
      buffers ++;

      printmsg(5,"Buffer %d length is %d bytes",buffers, current->used);

      while( offset < current->used )
      {
         size_t target_len = (unsigned char) *(current->pool + offset++);
         strings ++;
         printmsg(5,"[%d,%02d,%02d]=\"%s\"",
                     buffers,
                     strings,
                     target_len,
                     current->pool + offset);
         offset += target_len + 1;  /* Go to end of string             */

      } /* while( offset < current->used ) */

      current = current->next_link;

   }  /* while */

#ifdef __DEBUG_ALLOC__
   _dump_allocated( 16 );
#endif

} /* dump_pool */

#endif
