#ifndef CONFVARS_H
#define CONFVARS_H

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
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
 *    $Id: CONFVARS.H 1.2 1993/04/04 05:01:08 ahd Exp $
 *
 *    Revision history:
 *    $Log: CONFVARS.H $
 * Revision 1.2  1993/04/04  05:01:08  ahd
 * Add configurable OS/2 priority values
 *
 * Revision 1.1  1993/03/06  23:09:50  ahd
 * Initial revision
 *
 */

#include <uutypes.h>

/*--------------------------------------------------------------------*/
/*      Configuration file strings                                    */
/*--------------------------------------------------------------------*/

extern char *E_name, *E_mailbox, *E_homedir;
extern char *E_archivedir, *E_newsserv;                        /* news */
extern char *E_mailbox, *E_homedir;
extern char *E_maildir, *E_newsdir, *E_spooldir, *E_confdir;
extern char *E_pubdir, *E_tempdir;
extern char *E_nodename, *E_domain, *E_mailserv;
extern char *E_fdomain;       /* Domain used in From: lines             */
extern char *E_inmodem;
extern char *E_editor, *E_pager;
extern char *E_mailext, *E_backup;
extern char *E_filesent, *E_signature;
extern char *E_altsignature;                                   /* pdm   */
extern char *E_aliases;                                        /* ahd   */
extern char *E_organization, *E_replyto;                       /* ahd   */
extern char *E_localdomain;                                    /* ahd   */
extern char *E_postmaster;                                     /* ahd   */
extern char *E_uuxqtpath;
extern char **E_internal;
extern char *E_charset;
extern char *E_anonymous;
extern char *E_version;
extern char *E_uncompress;
extern INTEGER E_priority;
extern INTEGER E_prioritydelta;
extern INTEGER E_maxhops;

#endif
