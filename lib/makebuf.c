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
 *    $Id: lib.h 1.36 1996/03/18 03:48:14 ahd Exp $
 *
 *    $Log$
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

RCSID( "$Id$" );

/*--------------------------------------------------------------------*/
/*                     Local typedefs and defines                     */
/*--------------------------------------------------------------------*/

#define SIGNATURE ((long) 0xaaccddff)

typedef struct _BUFQUEUE
{
   long signature;
#ifdef UDEBUG
   unsigned long length;
#endif
   void *userBuffer;
   struct _BUFQUEUE *previous;
} BUFQUEUE;

static BUFQUEUE *top = NULL;

/*--------------------------------------------------------------------*/
/*       m a k e B u f                                                */
/*                                                                    */
/*       Allocate a buffer off the heap onto our queue                */
/*--------------------------------------------------------------------*/

void
*makeBuf( const size_t length, const char *file, const size_t line)
{
   BUFQUEUE *current = malloc( length + sizeof (BUFQUEUE) );

   if ( length == 0 )
   {
      printmsg(0,"makeBuf: Invalid request for zero byte buffer");
      bugout( file, line );
   }


/*--------------------------------------------------------------------*/
/*              Verify our buffer was properly allocated              */
/*--------------------------------------------------------------------*/

   if ( !current )
      checkptr( file, line);

/*--------------------------------------------------------------------*/
/*                  Chain new buffer to top of queue                  */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
   current->length = length;
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
      printmsg(done ? 8 : 2,"%s(%d): freeBuf: %slicit free of %p for %u bytes",
                  file,
                  line,
                  done ? "ex" : "im",
                  top,
                  top->length );
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
