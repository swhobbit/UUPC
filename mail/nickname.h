/*--------------------------------------------------------------------*/
/*       a l i a s . h                                                */
/*                                                                    */
/*       Mail User Agent Address alias functions                      */
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

boolean InitRouter( void );
