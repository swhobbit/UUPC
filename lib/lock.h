/*--------------------------------------------------------------------*/
/*    l o c k . c                                                     */
/*                                                                    */
/*    Locking functions for UUPC/extended                             */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks; all        */
/*    rights reserved except those explicitly granted by the          */
/*    UUPC/extended license.                                          */
/*--------------------------------------------------------------------*/

typedef struct {
   char lname[FILENAME_MAX];
   FILE *locket;
   } LOCKSTACK;

extern boolean locked;

void UnlockSystem( void );

boolean LockSystem( const char *system , long program );

/*--------------------------------------------------------------------*/
/*                   Allow an additional local lock                   */
/*--------------------------------------------------------------------*/

void PushLock( LOCKSTACK *top );

void PopLock( LOCKSTACK *top );
