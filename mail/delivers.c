/*--------------------------------------------------------------------*/
/*       d e l i v e r s . c                                          */
/*                                                                    */
/*       SMTP delivery module for UUPC/extended                       */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1996 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       This module performs simple SMTP delivery of mail of         */
/*       one or whitespace separated more addresses to the            */
/*       specified remote host.                                       */
/*                                                                    */
/*       How the remote name is host is determined is up the          */
/*       caller; specifically, the addresses are not checked for      */
/*       MX or other routing hints by this module.                    */
/*                                                                    */
/*       If the remote host cannot be contacted or otherwise not      */
/*       checked for addresses, the routine returns false and the     */
/*       caller must handle any retry (normally via our forte,        */
/*       UUCP); this routine does NOT queue mail for retry            */
/*                                                                    */
/*       If the remote host rejects a single address, the normal      */
/*       UUPC/extended bounce processing occurs.                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.36 1996/03/18 03:48:14 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 */

 deliverSMTP(
