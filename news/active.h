#ifndef _ACTIVE_H
#define _ACTIVE_H

/*--------------------------------------------------------------------*/
/*       a c t i v e . h                                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2002 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: active.h 1.17 2001/03/12 13:50:09 ahd v1-13k $
 *
 *    Revision history:
 *    $Log: active.h $
 *    Revision 1.17  2001/03/12 13:50:09  ahd
 *    Annual copyright update
 *
 *    Revision 1.16  2000/05/12 12:27:02  ahd
 *    Annual copyright update
 *
 *    Revision 1.15  1999/01/04 03:55:33  ahd
 *    Annual copyright change
 *
 *    Revision 1.14  1998/03/01 01:31:10  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.13  1997/04/24 01:39:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.12  1996/01/01 23:58:40  ahd
 *    Annual copyright update
 *
 *    Revision 1.11  1995/12/12 13:51:10  ahd
 *    Use true recursive function to talk active group tree
 *
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
