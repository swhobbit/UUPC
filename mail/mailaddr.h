#ifndef _MAILADDR_H
#define _MAILADDR_H

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1998 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1998 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: mailaddr.h 1.1 1997/12/13 18:11:33 ahd v1-12u $
 *
 *    $Log: mailaddr.h $
 *    Revision 1.1  1997/12/13 18:11:33  ahd
 *    Initial revision
 *
 */

typedef struct _MAIL_ADDR
{
   char *address;
   char *relay;
   char *host;
   char *user;
}  MAIL_ADDR;

#endif /* _MAILADDR_H */
