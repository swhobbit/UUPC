/*--------------------------------------------------------------------*/
/*       p o s 2 e r r . h                                            */
/*                                                                    */
/*       Report error message from OS/2 error message                 */
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
 *    $Id: pOS2Err.c 1.5 1993/09/20 04:38:11 ahd Exp $
 *
 *    Revision history:
 *    $Log: pOS2Err.c $
 *
 */

void pOS2Err(const size_t lineno,
             const char *fname,
             const char *prefix,
             unsigned int rc);

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printOS2error( x, rc )  pOS2Err( __LINE__, cfnptr, x, rc)
