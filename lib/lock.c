/*--------------------------------------------------------------------*/
/*    l o c k . c                                                     */
/*                                                                    */
/*    File locking functions for UUPC/extended                        */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lock.c 1.18 1999/01/04 03:52:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: lock.c $
 *    Revision 1.18  1999/01/04 03:52:28  ahd
 *    Annual copyright change
 *
 *    Revision 1.17  1998/03/09 01:18:19  ahd
 *    Update copyright
 *
 *    Revision 1.16  1995/03/11 22:26:18  ahd
 *    Use macro for file delete to allow special OS/2 processing
 *
 *    Revision 1.15  1995/01/07 16:13:03  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 */

/*--------------------------------------------------------------------*/
/*                   Standard library include files                   */
/*--------------------------------------------------------------------*/

#include "uupcmoah.h"

#include <process.h>
#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "dater.h"
#include "stater.h"
#include "lock.h"
#include "import.h"

/*--------------------------------------------------------------------*/
/*                      Define current file name                      */
/*--------------------------------------------------------------------*/

RCSID("$Id: lock.c 1.18 1999/01/04 03:52:28 ahd Exp $");

/*--------------------------------------------------------------------*/
/*                          Local variables                           */
/*--------------------------------------------------------------------*/

static FILE *locket = NULL;
static char lname[FILENAME_MAX];

KWBoolean locked = KWFalse;

/*--------------------------------------------------------------------*/
/*    L o c k S y s t e m                                             */
/*                                                                    */
/*    Get the lock for a system                                       */
/*--------------------------------------------------------------------*/

KWBoolean LockSystem( const char *system , long program )
{
   time_t age;
   char fname[FILENAME_MAX];
   char *extension;

/*--------------------------------------------------------------------*/
/*                Don't lock unless in multitask mode                 */
/*--------------------------------------------------------------------*/

   if ( ! bflag[ F_MULTITASK ] )
      return KWTrue;

/*--------------------------------------------------------------------*/
/*                Verify our lock is not already in use               */
/*--------------------------------------------------------------------*/

   if ( locket != NULL )
   {
      printmsg(0,
         "LockSystem: Attempt to lock %s with lock file %s already open",
         system,
         lname );
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                        Try to get the lock                         */
/*--------------------------------------------------------------------*/

   switch( program )
   {
      case B_NEWS:
         extension = "LCN";
         break;

      case B_UUSTAT:
         extension = "LCS";
         break;

      case B_UUCICO:
         extension = "LCK";
         break;

      case B_UUXQT:
         extension = "LCX";
         break;

      default:
         extension = "LCZ";
         break;
   } /* switch */

   sprintf( fname,
            "%s/locks.lck/%.8s.%s",
            E_spooldir,
            system,
            extension );

   importpath( lname, fname, system );

/*--------------------------------------------------------------------*/
/*    Determine if the lock exists, and unlink it if so.  If this     */
/*    fails, we can't get the lock, so we return gracefully.          */
/*    (This allows to bypass the "helpful" FOPEN retries of the       */
/*    failed open.                                                    */
/*--------------------------------------------------------------------*/

   if ( access( lname, 0 ) || !REMOVE( lname ))
      locket = FOPEN( lname, "w",TEXT_MODE );

   if ( locket == NULL )
   {
      long size;

      age = stater( lname, &size );

      printmsg(1, "System %s already locked since %s",
               system,
               (size > 0) ? dater( age, NULL ) : "UNKNOWN" );
      return KWFalse;
   }

   setvbuf( locket, NULL, _IONBF, 0);

   time( &age );
   fprintf( locket, "%s locked by process %ld at %.24s\n",
                    system, (long) getpid(), ctime( &age ));
   fflush( locket );          /* Force the file to exist on disk  */

/*--------------------------------------------------------------------*/
/*     We don't close the file, since the open file *is* the lock     */
/*--------------------------------------------------------------------*/

   locked = KWTrue;
   return KWTrue;

} /* LockSystem */

/*--------------------------------------------------------------------*/
/*    U n l o c k S y s t e m                                         */
/*                                                                    */
/*    Release a lock created by LockSystem                            */
/*--------------------------------------------------------------------*/

void UnlockSystem( void )
{

/*--------------------------------------------------------------------*/
/*                Don't lock unless in multitask mode                 */
/*--------------------------------------------------------------------*/

   if ( ! bflag[ F_MULTITASK ] )
      return;

/*--------------------------------------------------------------------*/
/*                       Verify we hold a lock                        */
/*--------------------------------------------------------------------*/

   if ( locket == NULL )
   {
      printmsg(0,"UnlockSystem: No lock held");
      panic();
   } /* if */

/*--------------------------------------------------------------------*/
/*                          Release the lock                          */
/*--------------------------------------------------------------------*/

   fclose( locket );
   locket = NULL;
   locked = KWFalse;

   REMOVE( lname );

} /* UnlockSystem */

/*--------------------------------------------------------------------*/
/*       P u s h L o c k                                              */
/*                                                                    */
/*       Save lock status in order to perform second lock             */
/*--------------------------------------------------------------------*/

void PushLock( LOCKSTACK *top )
{
   top->locket = locket;
   if ( locket !=  NULL )
   {
      strcpy( top->lname, lname );
      locket =  NULL;
   }
   locked = KWFalse;

}

/*--------------------------------------------------------------------*/
/*       P o p L o c k                                                */
/*                                                                    */
/*       Restore previous lock information                            */
/*--------------------------------------------------------------------*/

void PopLock( LOCKSTACK *top )
{
   locket = top->locket;

   if ( locket !=  NULL )
   {
      strcpy( lname, top->lname );
      locked = KWTrue;
   }

} /* PopLock */
