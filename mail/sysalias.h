/*--------------------------------------------------------------------*/
/*    S y s A l i a s . H                                             */
/*                                                                    */
/*    System wide alias support for UUPC/extended                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: LIB.H 1.3 1992/12/01 04:39:34 ahd Exp $
 *
 *    $Log: LIB.H $
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
