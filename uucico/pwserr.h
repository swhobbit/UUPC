/*--------------------------------------------------------------------*/
/*       p w s e r r . h                                              */
/*                                                                    */
/*       Report error message from Windows sockets error message      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1993 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: pwserr.h 1.1 1993/09/24 03:46:07 dmwatt Exp $
 *
 *    Revision history:
 *    $Log: pwserr.h $
 * Revision 1.1  1993/09/24  03:46:07  dmwatt
 * Initial revision
 *
 *
 */

void pWSErr(const size_t lineno,
             const char *fname,
             const char *prefix,
             int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printWSerror( x, rc )  pWSErr( __LINE__, cfnptr, x, rc)
