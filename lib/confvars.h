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
 *    $Id: confvars.h 1.7 1993/06/15 12:19:57 ahd Exp ahd $
 *
 *    Revision history:
 *    $Log: confvars.h $
 *     Revision 1.7  1993/06/15  12:19:57  ahd
 *     Save changed/pushed directory name for debugging
 *
 *     Revision 1.6  1993/06/13  14:12:29  ahd
 *     Changes per Mike McLagan for outbound batched news support
 *
 *     Revision 1.5  1993/05/30  00:11:03  ahd
 *     Add E_passwd and E_systems
 *
 *     Revision 1.4  1993/05/09  03:50:05  ahd
 *     Support motd, banner strings in configuration
 *
 * Revision 1.3  1993/04/11  00:36:59  dmwatt
 * Global edits for year, TEXT, etc.
 *
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
extern char *E_compress;         // GMM - Outbound batched news support
extern char *E_banner;
extern char *E_motd;
extern char *E_systems;
extern char *E_passwd;
extern char *E_cwd;              // Never set by user, only program
extern KEWSHORT E_priority;
extern KEWSHORT E_prioritydelta;
extern KEWSHORT E_maxhops;

#endif
