/*--------------------------------------------------------------------*/
/*       a l i a s . h                                                */
/*                                                                    */
/*       Mail User Agent Address alias functions                      */
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
 *    $Id: alias.h 1.4 1995/01/07 16:19:34 ahd v1-12q $
 *
 *    Revision history:
 *    $Log: alias.h $
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

static struct AliasTable {
      char *anick;
      char *anode;
      char *auser;
      char *afull;
   };

void ExtractName( char *result, const char *input );

void BuildAddress(char *result, const char *input );

char *AliasByNick(const char *input);

char *AliasByAddr(const char *node, const char *user);

KWBoolean InitRouter( void );
