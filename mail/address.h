/*--------------------------------------------------------------------*/
/*    a d d r e s s . h                                               */
/*                                                                    */
/*    Routines in address.c                                           */
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
 *    $Id: address.h 1.2 1994/03/07 06:12:42 ahd v1-12k $
 *
 *    Revision history:
 *    $Log: address.h $
 *    Revision 1.2  1994/03/07 06:12:42  ahd
 *    Add copyright information
 *
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
