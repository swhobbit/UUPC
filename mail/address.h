/*--------------------------------------------------------------------*/
/*       a d d r e s s . h                                            */
/*                                                                    */
/*       Routines in address.c                                        */
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
 *    $Id: address.h 1.9 1998/03/01 01:31:27 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: address.h $
 *    Revision 1.9  1998/03/01 01:31:27  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1997/12/13 18:11:33  ahd
 *    Add debuging information for HostAlias, macro to determine
 *    if address is only local part
 *
 *    Revision 1.7  1997/04/24 01:10:34  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1996/01/01 21:04:35  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1995/11/30 03:07:22  ahd
 *    Trap truly invalid addresses in tokenizer
 *
 *    Revision 1.4  1995/01/07 16:19:31  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.3  1994/12/22 00:20:00  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/03/07 06:12:42  ahd
 *    Add copyright information
 *
 */

KWBoolean
tokenizeAddress( const char *address,
                  char *hispath,
                  char *hisnode,
                  char *hisuser );


#define HostAlias( host ) hostAlias( host, __FILE__, __LINE__ )

char *hostAlias( const char *host, const char *fname, const size_t lineno);

char *HostPath( const char *host, const char *best);

typedef enum
{
      ADDRESSONLY = KWFalse,
      FULLNAMEONLY = KWTrue,
      FULLADDRESS
} FULLNAME;

char *ExtractAddress( char *result, const char *column, FULLNAME fullname );


#define isOnlyLocalAddress(address) (!strpbrk(address, "!@"))
