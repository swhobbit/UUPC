/*--------------------------------------------------------------------*/
/*       n i c k n a m e . h                                          */
/*                                                                    */
/*       Mail User Agent Address nickname functions                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-2001 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: nickname.h 1.10 2000/05/12 12:42:31 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: nickname.h $
 *    Revision 1.10  2000/05/12 12:42:31  ahd
 *    Annual copyright update
 *
 *    Revision 1.9  1999/01/04 03:56:09  ahd
 *    Annual copyright change
 *
 *    Revision 1.8  1998/03/01 01:30:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1997/04/24 01:11:02  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.6  1996/01/01 23:52:34  ahd
 *    Rename former "user alias" functions to "nickname" prefixes, consistent
 *    with external references.
 *
 *    Revision 1.5  1996/01/01 21:04:25  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1995/01/07 16:19:34  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.3  1994/12/22 00:20:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/03/05 21:15:49  ahd
 *    Revamp header copyrights, line buffer lengths
 *
 */

typedef struct _NICKNAME
{
      char *anick;
      char *anode;
      char *auser;
      char *afull;
} NICKNAME;

void ExtractName( char *result, const char *input );

void BuildAddress(char *result, const char *input );

char *nicknameByNick(const char *input);

char *nicknameByAddr(const char *node, const char *user);

KWBoolean InitRouter( void );
