/*--------------------------------------------------------------------*/
/*       m a k e b u f . c                                            */
/*                                                                    */
/*       heap memory poll management routines; buffers are            */
/*       allocated off a LIFO queue; freeing of a buffer causes       */
/*       all buffers allocated it after to also be freed              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: makebuf.c 1.2 1996/11/19 00:14:35 ahd v1-13f $
 *
 *    $Log: makebuf.c $
 *    Revision 1.2  1996/11/19 00:14:35  ahd
 *    Add more debugging information
 *
 *    Revision 1.1  1996/11/18 04:46:49  ahd
 *    Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*                        System include files                        */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

/*--------------------------------------------------------------------*/
/*                        Local include files                         */
/*--------------------------------------------------------------------*/

#include "makebuf.h"

/*--------------------------------------------------------------------*/
/*                           RCS signature                            */
/*--------------------------------------------------------------------*/

RCSID( "$Id: makebuf.c 1.2 1996/11/19 00:14:35 ahd v1-13f $" );

/*--------------------------------------------------------------------*/
/*                     Local typedefs and defines                     */
/*--------------------------------------------------------------------*/

#define SIGNATURE ((long) 0xaaccddff)

typedef struct _BUFQUEUE
{
   long signature;
#ifdef UDEBUG
   unsigned long length;
   int entry;
#endif
   void *userBuffer;
   struct _BUFQUEUE *previous;
} BUFQUEUE;

static BUFQUEUE *top = NULL;

#ifdef UDEBUG
static int entries = 0;
#endif

/*--------------------------------------------------------------------*/
/*       m a k e B u f                                                */
/*                                                                    */
/*       Allocate a buffer off the heap onto our queue                */
/*--------------------------------------------------------------------*/

void
*makeBuf( const size_t length, const char *file, const size_t line)
{
   BUFQUEUE *current;

   if ( length == 0 )
   {
      printmsg(0,"makeBuf: Invalid request for zero byte buffer");
      bugout( file, line );
   }

   current = malloc( length + sizeof (BUFQUEUE) );

/*--------------------------------------------------------------------*/
/*              Verify our buffer was properly allocated              */
/*--------------------------------------------------------------------*/

   if ( !current )
      checkptr( file, line);

/*--------------------------------------------------------------------*/
/*                  Chain new buffer to top of queue                  */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   entries++;

   current->length = length;
   current->entry = entries;
#endif

   current->signature  = SIGNATURE;
   current->userBuffer = (void *) (sizeof (BUFQUEUE) + (char *) current);
   current->previous   = top;
   top = current;

   return current->userBuffer;

} /* makeBuf */

/*--------------------------------------------------------------------*/
/*       f r e e B u f                                                */
/*                                                                    */
/*       Free a buffer allocated by makeBuf, and all buffers          */
/*       allocated after it                                           */
/*--------------------------------------------------------------------*/

void
freeBuf( void *oldBuffer, const char *file, const size_t line )
{
   while( top != NULL )
   {
      BUFQUEUE *save = top;

      KWBoolean done = (KWBoolean) ( top->userBuffer == oldBuffer ?
                                       KWTrue : KWFalse );

      if ( top->signature != SIGNATURE )
      {
         printmsg(0, "freeBuf: Invalid buffer queue at %p", top );
         bugout( file, line );
      }

      if ( top->userBuffer == oldBuffer )
         done = KWTrue;

#ifdef UDEBUG
      printmsg(done ? 8 : 2,"%s(%d): freeBuf: %slicit "
                  "free of %p (%d,%d) for %u bytes",
                  file,
                  line,
                  done ? "ex" : "im",
                  top,
                  top->entry,
                  entries,
                  top->length );

      entries--;
#endif

/*--------------------------------------------------------------------*/
/*            Pop the current entry off the stack and free it         */
/*--------------------------------------------------------------------*/

      top = top->previous;
      free( save );

      if ( done )
         return;

   } /* while( top != NULL ) */

/*--------------------------------------------------------------------*/
/*       If we didn't find the block to free, we have a serious       */
/*       internal error -- report it and exit.                        */
/*--------------------------------------------------------------------*/

   printmsg( 0, "freeBuf: Unable to locate requested memory block at %p",
                 oldBuffer );
   bugout( file, line );

} /* freeBuf */
