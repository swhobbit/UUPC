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
 *    $Header: E:\SRC\UUPC\LIB\RCS\LOCK.C 1.3 1992/11/19 02:54:47 ahd Exp $
 *
 *    Revision history:
 *    $Log: LOCK.C $
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

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <process.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "lib.h"
#include "dater.h"
#include "stater.h"
#include "lock.h"
#include "hlib.h"
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

boolean locked = FALSE;

/*--------------------------------------------------------------------*/
/*    L o c k S y s t e m                                             */
/*                                                                    */
/*    Get the lock for a system                                       */
/*--------------------------------------------------------------------*/

boolean LockSystem( const char *system , long program )
{
   time_t age;
   char fname[FILENAME_MAX];

/*--------------------------------------------------------------------*/
/*                Don't lock unless in multitask mode                 */
/*--------------------------------------------------------------------*/

   if ( ! bflag[ F_MULTITASK ] )
      return TRUE;

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

   sprintf( fname,
            "%s/locks.lck/%.8s.%s",
            E_spooldir,
            system,
            (program == B_UUXQT ) ? "LCX" : "LCK" );

   importpath( lname, fname, system );

   locket = FOPEN( lname, "w", TEXT );

   if ( locket == NULL )
   {
      long size;

      age = stater( lname, &size );

      printmsg(1, "System %s already locked since %s",
               system,
               (size > 0) ? dater( age, NULL ) : "UNKNOWN" );
      return FALSE;
   }

   setvbuf( locket, NULL, _IONBF, 0);

   time( &age );
   fprintf( locket, "%s locked by process %ld at %s",
                    system, (long) getpid(), ctime( &age ));
   fflush( locket );          /* Force the file to exist on disk  */

/*--------------------------------------------------------------------*/
/*     We don't close the file, since the open file *is* the lock     */
/*--------------------------------------------------------------------*/

   locked = TRUE;
   return TRUE;

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
   locked = FALSE;

   unlink( lname );

} /* UnlockSystem */
