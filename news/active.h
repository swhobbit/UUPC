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
 *    $Id: active.h 1.10 1995/08/27 23:33:26 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: active.h $
 *    Revision 1.10  1995/08/27 23:33:26  ahd
 *    Load and use ACTIVE file as tree structure
 *
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

void
startActiveWalk( void (*walkOneNode)(const char *, void *),
                 void *optional );

char
getModeration( const char *name );

void
writeActive( void );

#ifndef MAXGRP
#define MAXGRP FILENAME_MAX   /* Max length of a news group name      */
#endif

#endif /* _ACTIVE_H */
