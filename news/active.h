#ifndef _ACTIVE_H
#define _ACTIVE_H

/*--------------------------------------------------------------------*/
/*       a c t i v e . h                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.32 1995/07/21 13:28:20 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

KWBoolean
loadActive( const KWBoolean mustExist );

KWBoolean
addGroup( const char *group,
          const long high,
          const long low,
          const char moderation );

KWBoolean
deleteGroup( const char *name );

long
getArticleOldest( const char *name );

long
getArticleNewest( const char *name );

KWBoolean
setArticleNewest( const char *name, const long high );

KWBoolean
setArticleOldest( const char *name, const long low );

char *
walkActive( char *buf );

char
getModeration( const char *name );

void
writeActive( void );

void
startActiveWalk( void );

#ifndef MAXGRP
#define MAXGRP FILENAME_MAX   /* Max length of a news group name      */
#endif

#endif /* _ACTIVE_H */
