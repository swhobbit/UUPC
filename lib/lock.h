/*--------------------------------------------------------------------*/
/*    l o c k . c                                                     */
/*                                                                    */
/*    Locking functions for UUPC/extended                             */
/*                                                                    */
/*    Copyright (c) 1992 by Kendra Electronic Wonderworks; all        */
/*    rights reserved except those explicitly granted by the          */
/*    UUPC/extended license.                                          */
/*--------------------------------------------------------------------*/

extern boolean locked;

void UnlockSystem( void );

boolean LockSystem( const char *system , long program );
