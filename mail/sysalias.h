/*--------------------------------------------------------------------*/
/*    S y s A l i a s . H                                             */
/*                                                                    */
/*    System wide alias support for UUPC/extended                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1999 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: sysalias.h 1.10 1998/03/01 01:27:53 ahd v1-13f ahd $
 *
 *    $Log: sysalias.h $
 *    Revision 1.10  1998/03/01 01:27:53  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.9  1997/04/24 01:11:07  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.8  1996/01/01 21:04:58  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.7  1995/01/07 16:19:55  ahd
 *    Change boolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.6  1994/12/22 00:20:34  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.5  1994/12/09 03:54:15  ahd
 *    Support recursely processing system aliases file for same address
 *
 * Revision 1.4  1994/01/24  03:18:12  ahd
 * Annual Copyright Update
 *
 * Revision 1.3  1994/01/01  19:13:45  ahd
 * Annual Copyright Update
 *
 * Revision 1.2  1993/04/11  00:36:13  ahd
 * Global edits for year, TEXT, etc.
 *
 * Revision 1.1  1992/12/04  01:03:49  ahd
 * Initial revision
 *
 */

/*--------------------------------------------------------------------*/
/*    System alias table; includes name of alias, and start and       */
/*    end position of alias data in the system alias file             */
/*--------------------------------------------------------------------*/

typedef struct _ALIASTABLE
      {
         char *alias;
         long start;
         long end;
         KWBoolean recurse;
      } ALIASTABLE;

extern char *SysAliases;      /* Name of system alias file           */

ALIASTABLE *checkalias( const char *user );
