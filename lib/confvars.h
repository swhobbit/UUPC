#ifndef CONFVARS_H
#define CONFVARS_H

/*--------------------------------------------------------------------*/
/*    c o n f v a r s . h                                             */
/*                                                                    */
/*    COnfiguration variable information for UUPC/extended            */
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
 *    $Id: confvars.h 1.32 1999/01/04 03:55:03 ahd Exp $
 *
 *    Revision history:
 *    $Log: confvars.h $
 *    Revision 1.32  1999/01/04 03:55:03  ahd
 *    Annual copyright change
 *
 *    Revision 1.31  1998/05/11 01:25:40  ahd
 *    Add log directory
 *
 *    Revision 1.30  1998/03/01 01:26:14  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.29  1997/05/11 04:28:53  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.28  1997/03/31 06:58:34  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.27  1996/03/18 03:48:14  ahd
 *    Allow compilation under C++ compilers
 *
 *    Revision 1.26  1996/01/07 14:18:18  ahd
 *    Provide external references to configuration functions and routines
 *    needed by regsetup.
 *
 *    Revision 1.25  1996/01/01 20:56:44  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.24  1995/02/20 00:40:44  ahd
 *    News index caching support
 *
 *    Revision 1.23  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.22  1994/12/27 20:50:28  ahd
 *    Smoother call grading
 *
 *    Revision 1.21  1994/12/22 00:12:24  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.20  1994/02/25 02:31:10  ahd
 *    Add E_ignoreList, and E_replytoList for MAIL
 *
 *     Revision 1.19  1994/02/20  19:16:21  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.18  1994/02/14  00:29:49  ahd
 *     Use quotes, not angle brackets, for local includes
 *
 *     Revision 1.17  1994/01/24  03:09:53  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.16  1994/01/01  19:07:41  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.15  1993/12/29  02:48:21  ahd
 *     Add V-mail queuing support
 *
 *     Revision 1.14  1993/12/02  01:37:33  ahd
 *     Add maximum length of UUXQT command lines
 *
 *     Revision 1.13  1993/10/31  15:53:26  ahd
 *     Allow configurating permissions file name
 *
 *     Revision 1.12  1993/10/25  01:23:25  ahd
 *     Rename E_aliases to E_nickname
 *
 *     Revision 1.11  1993/10/12  01:21:25  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.10  1993/09/29  23:30:28  ahd
 *     Add xqtRootDir for UUXQT
 *
 *     Revision 1.9  1993/09/29  04:56:11  ahd
 *     Move priority variables to modem file
 *
 *     Revision 1.8  1993/09/20  04:51:31  ahd
 *     TCP Support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support (BC++ 1.0 for OS/2 support)
 *
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

/*--------------------------------------------------------------------*/
/*      Configuration file strings                                    */
/*--------------------------------------------------------------------*/

#ifdef __cplusplus
   extern "C" {
#endif

extern char **E_internal;
extern char *E_altsignature;                                   /* pdm  */
extern char *E_anonymous;
extern char *E_archivedir, *E_newsserv;                        /* news */
extern char *E_banner;
extern char *E_charset;
extern char *E_compress;         /* GMM - Outbound batched news support  */
extern char *E_cwd;              /* Never set by user, only program   */
extern char *E_editor, *E_pager;
extern char *E_fdomain;       /* Domain used in From: lines            */
extern char *E_filesent, *E_signature;
extern char *E_inmodem;
extern char *E_localdomain;                                    /* ahd  */
extern char *E_mailbox, *E_homedir;
extern char *E_maildir, *E_newsdir, *E_spooldir, *E_confdir;
extern char *E_mailext, *E_backup;
extern char *E_motd;
extern char *E_name, *E_mailbox, *E_homedir;
extern char *E_nickname;      /* Nickname (user alias) file          */
extern char *E_nodename, *E_domain, *E_mailserv;
extern char *E_organization, *E_replyto;
extern char *E_passwd;
extern char *E_permissions;         /* Name of permissions file   */
extern char *E_postmaster;
extern char *E_pubdir;
extern char *E_tempdir;
extern char *E_systems;
extern char *E_uncompress;
extern char *E_uuxqtpath;
extern char *E_version;
extern char *E_xqtRootDir;
extern char *E_vmsQueueDir;
extern char *E_vmail;
extern char **E_ignoreList;      /* Headers not displayed by print   */
extern char **E_replyToList;     /* Primary Addr to reply to mail    */
extern char E_firstGrade;        /* First class searched by UUCICO   */
extern char E_mailGrade;         /* Default grade for mail           */
extern char E_newsGrade;         /* Default grade for news           */
extern char *E_tz;               /* Time zone                        */
extern char *E_logdir;           /* Log directory name               */

extern KEWSHORT E_timeoutSMTP;   /* SMTP read timeout for rmail      */
extern KEWSHORT E_maxhops;
extern KEWSHORT E_maxuuxqt;      /* Max length of remote cmd line    */
extern long E_batchsize;         /* Size of remote batches           */
extern KEWSHORT E_newsCache;     /* Pages of news index to cache     */

#ifdef __cplusplus
   }
#endif

#endif
