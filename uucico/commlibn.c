/*--------------------------------------------------------------------*/
/*       c o m m l i b n . C                                          */
/*                                                                    */
/*       Generic communications library interface for UUPC/extended   */
/*       without TCP/IP support (OS/2) or TAPI support (Windows NT)   */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-2000 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: commlibn.c 1.9 2000/05/12 12:32:55 ahd v1-13g $
 *
 *    Revision history:
 *    $Log: commlibn.c $
 *    Revision 1.9  2000/05/12 12:32:55  ahd
 *    Annual copyright update
 *
 *    Revision 1.8  1999/01/08 02:20:56  ahd
 *    Convert currentfile() to RCSID()
 *
 *    Revision 1.7  1999/01/04 03:53:57  ahd
 *    Annual copyright change
 *
 *    Revision 1.6  1998/05/11 01:20:48  ahd
 *    Allow building UUCICO w/o TAPI support under NT
 *
 *    Revision 1.5  1998/03/01 01:39:07  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.4  1997/04/24 01:33:05  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.3  1996/01/01 21:19:40  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.2  1994/12/22 00:32:55  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.1  1994/05/01 21:56:50  ahd
 *    Initial revision
 *
 */

#if defined(__OS2__)
#define NOTCPIP            /* Don't include the TCP/IP suite      */
#endif

#if defined(WIN32)
#undef NOTAPI_SUPPORT      /* Don't include TAPI SUPPORT          */
#endif

#include "commlib.c"
