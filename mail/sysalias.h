/*--------------------------------------------------------------------*/
/*    S y s A l i a s . H                                             */
/*                                                                    */
/*    System wide alias support for UUPC/extended                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1993 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: SYSALIAS.H 1.1 1992/12/04 01:03:49 ahd Exp $
 *
 *    $Log: SYSALIAS.H $
 * Revision 1.1  1992/12/04  01:03:49  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*    System alias table; includes name of alias, and start and       */
/*    end position of alias data in the system alias file             */
/*--------------------------------------------------------------------*/

typedef struct _ALIASTABLE {
      char *alias;
      long start;
      long end;
      } ALIASTABLE;

extern char *SysAliases;      /* Name of system alias file           */

ALIASTABLE *checkalias( const char *user );
