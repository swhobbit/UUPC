/*--------------------------------------------------------------------*/
/*       p n t e r r . h                                              */
/*                                                                    */
/*       Report error message from OS/2 error message                 */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) David M. Watt 1993, All Rights Reserved   */
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
 *    $Id: pnterr.h 1.1 1993/09/25 03:02:55 ahd Exp $
 *
 *    Revision history:
 *    $Log: pnterr.h $
 * Revision 1.1  1993/09/25  03:02:55  ahd
 * Initial revision
 *
 * Revision 1.1  1993/09/24  03:46:07  ahd
 * Initial revision
 */

void pNTErr(const size_t lineno,
             const char *fname,
             const char *prefix,
             DWORD rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printNTerror( x, rc )  pNTErr( __LINE__, cfnptr, x, rc)
