/*--------------------------------------------------------------------*/
/*       n i c k n a m e . h                                          */
/*                                                                    */
/*       Mail User Agent Address nickname functions                   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: alias.h 1.5 1996/01/01 21:04:25 ahd Exp $
 *
 *    Revision history:
 *    $Log: nickname.h $
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
