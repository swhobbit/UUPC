/*--------------------------------------------------------------------*/
/*       u l i b n t . c                                              */
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
 *    $Id: ulibntn.c 1.1 1998/05/11 01:20:48 ahd v1-13f ahd $
 *
 *    Revision history:
 *    $Log: ulibntn.c $
 *    Revision 1.1  1998/05/11 01:20:48  ahd
 *    Initial revision
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

#include "ulibnt.c"
