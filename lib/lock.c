/*--------------------------------------------------------------------*/
/*    l o c k . c                                                     */
/*                                                                    */
/*    Locking functions for UUPC/extended                             */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks; all        */
/*    rights reserved except those explicitly granted by the          */
/*    UUPC/extended license.                                          */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lock.c 1.14 1994/02/19 04:43:20 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: lock.c $
 *    Revision 1.14  1994/02/19 04:43:20  ahd
 *    Use standard first header
 *
 *     Revision 1.13  1994/02/19  04:08:14  ahd
 *     Use standard first header
 *
 *     Revision 1.12  1994/02/19  03:53:01  ahd
 *     Use standard first header
 *
 *     Revision 1.11  1994/02/18  23:10:59  ahd
 *     Use standard first header
 *
 *     Revision 1.10  1993/11/30  04:18:14  ahd
 *     Add newline to fprintf()
 *
 *     Revision 1.9  1993/10/28  12:19:01  ahd
 *     Cosmetic time formatting twiddles and clean ups
 *
 *     Revision 1.8  1993/04/11  00:31:04  ahd
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.8  1993/04/11  00:31:04  ahd
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.7  1993/03/24  01:57:30  ahd
 *     Add string.h include
 *
 *     Revision 1.6  1993/03/06  22:48:23  ahd
 *     Allow nested locks
 *
 * Revision 1.5  1992/11/28  19:51:16  ahd
 * Issue lock based on lock file existence to avoid FOPEN retry loop
 *
 * Revision 1.4  1992/11/19  02:58:54  ahd
 * drop rcsid
 *
 * Revision 1.3  1992/11/19  02:54:47  ahd
 * Revision 1.2  1992/11/17  13:47:30  ahd
 * Do not buffer lock file
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
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

currentfile();

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

   if ( access( lname, 0 ) || !unlink( lname ))
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

   unlink( lname );

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
