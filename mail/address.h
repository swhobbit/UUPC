/*--------------------------------------------------------------------*/
/*    a d d r e s s . h                                               */
/*                                                                    */
/*    Routines in address.c                                           */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1994 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.20 1994/02/20 19:16:21 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

void user_at_node( const char *address,
                  char *hispath,
                  char *hisnode,
                  char *hisuser );

char *HostAlias( char *host);

char *HostPath( char *host, char *best);

typedef enum {
      ADDRESSONLY = FALSE,
      FULLNAMEONLY = TRUE,
      FULLADDRESS }
      FULLNAME;

char *ExtractAddress( char *result, const char *column, FULLNAME fullname );
