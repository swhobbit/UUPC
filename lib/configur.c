/*--------------------------------------------------------------------*/
/*    c o n f i g u r . c                                             */
/*                                                                    */
/*    Support routines for UUPC/extended                              */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989-1996 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: configur.c 1.73 1996/01/04 04:00:46 ahd Exp $
 *
 *    Revision history:
 *    $Log: configur.c $
 *    Revision 1.73  1996/01/04 04:00:46  ahd
 *    Use sorted list of boolean options with binary search and computed
 *    table size.
 *
 *    Revision 1.72  1996/01/02 00:00:24  ahd
 *    Break out search loop for configuration file keywords from
 *    processing of them.
 *    Use proper binary search for configuration file keywords rather
 *    than lineaer search.  Also includes pre-computing size of configuration
 *    tables.
 *
 *    Revision 1.71  1995/08/27 23:30:21  ahd
 *    Change variable case
 *
 *    Revision 1.70  1995/04/02 00:01:39  ahd
 *    Add option for Kai Uwe Rommel to suppress display of UUPC/extended
 *    version, system name, etc. at remote login time.
 *
 *    Revision 1.69  1995/03/23 01:30:34  ahd
 *    Load the mailserv variable for any news program which could generate
 *    a SYSTEMS file.
 *
 *    Revision 1.68  1995/03/08 02:58:08  ahd
 *    Delete redundant boolean options
 *
 *    Revision 1.67  1995/02/25 18:21:44  ahd
 *    Correct selected flags for config variables
 *
 *    Revision 1.66  1995/02/24 00:37:28  ahd
 *    Optimize which variables are included where
 *
 *    Revision 1.65  1995/02/21 03:30:52  ahd
 *    More compiler warning cleanup, drop selected messages at compile
 *    time if not debugging.
 *
 *    Revision 1.64  1995/02/20 18:54:08  ahd
 *    news panic support
 *
 *    Revision 1.63  1995/02/20 17:28:43  ahd
 *    Add rnewspanic option
 *    Various compiler warning clean up
 *
 *    Revision 1.62  1995/02/20 00:38:04  ahd
 *    news index caching support
 *
 *    Revision 1.61  1995/02/12 23:35:59  ahd
 *    'compiler
 *
 *    Revision 1.60  1995/01/29 16:43:03  ahd
 *    IBM C/Set compiler warnings
 *
 *    Revision 1.59  1995/01/29 14:07:59  ahd
 *    Clean up most IBM C/Set Compiler Warnings
 *
 *    Revision 1.58  1995/01/28 22:08:10  ahd
 *    Correct use of TZ variable from within UUPC configuration file
 *
 *    Revision 1.57  1995/01/15 19:48:35  ahd
 *    Allow active file to be optional
 *    Delete fullbatch global option
 *    Add "local" and "batch" flags to SYS structure for news
 *
 *    Revision 1.56  1995/01/07 16:12:03  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.55  1995/01/07 15:43:07  ahd
 *    Add in-memory file flag
 *
 *    Revision 1.54  1995/01/02 05:03:27  ahd
 *    Pass 2 of integrating SYS file support from Mike McLagan
 *
 *    Revision 1.53  1994/12/31 03:51:01  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.52  1994/12/31 03:41:08  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
 *    Revision 1.51  1994/12/27 20:47:55  ahd
 *    Smoother call grading'
 *
 *    Revision 1.50  1994/12/22 00:07:46  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.49  1994/12/09 03:42:09  ahd
 *    All suppressbeep support to allow NOT making any sound
 *
 *     Revision 1.48  1994/05/24  03:44:04  ahd
 *     Sort KWBoolean options
 *     Add suppressemptypassword option
 *
 *     Revision 1.47  1994/05/04  02:03:11  ahd
 *     Don't declare registry function if not under Windows NT
 *
 *     Revision 1.46  1994/04/26  23:49:06  dmwatt
 *     Windows NT registry support
 *
 *     Revision 1.45  1994/03/05  21:12:05  ahd
 *     Correct spelling of LONGNAME flag
 *
 *     Revision 1.44  1994/02/28  01:02:06  ahd
 *     Add 'HonorControl' KWBoolean option
 *
 *     Revision 1.43  1994/02/25  02:23:42  ahd
 *     Add Ignore, ReplyToList for Mail user agent (mail)
 *
 *     Revision 1.42  1994/02/20  19:05:02  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.41  1994/02/19  04:40:04  ahd
 *     Use standard first header
 *
 *     Revision 1.37  1994/01/01  19:00:55  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.36  1993/12/29  02:46:47  ahd
 *     Add Vmail queuing support
 *
 *     Revision 1.35  1993/12/23  03:11:17  rommel
 *     OS/2 32 bit support for additional compilers
 *
 *     Revision 1.34  1993/12/02  02:25:12  ahd
 *     Add max generated UUXQT command line length
 *
 *     Revision 1.33  1993/11/30  04:18:14  ahd
 *     Correct spelling error in message
 *
 *     Revision 1.32  1993/11/21  02:47:07  ahd
 *     Allow UUPCUSRRC to be set from environment variables
 *
 *     Revision 1.31  1993/11/14  20:51:37  ahd
 *     Add showspool
 *
 *     Revision 1.30  1993/11/13  17:37:02  ahd
 *     Add new options supporting mail formats and new windows under OS/2
 *
 *     Revision 1.29  1993/10/31  15:51:11  ahd
 *     Allow configuring permissions file name
 *     Restore period in names before printing error messages
 *
 *     Revision 1.28  1993/10/30  22:27:57  rommel
 *     News history support
 *
 *     Revision 1.27  1993/10/26  12:46:10  ahd
 *     Add include for setstdin.h
 *
 *     Revision 1.26  1993/10/25  02:39:44  ahd
 *     Delete OBSOLETE keyword from aliases, it breaks it
 *
 *     Revision 1.25  1993/10/25  01:21:22  ahd
 *     Rename Aliases to Nickname to make more unique for end users; allow
 *     Aliases as obsolete alias for now.
 *
 *     Revision 1.24  1993/10/12  00:47:04  ahd
 *     Normalize comments
 *
 *     Revision 1.23  1993/10/09  15:47:16  ahd
 *     Add run time IsDOS query function
 *
 *     Revision 1.22  1993/09/29  23:29:56  ahd
 *     Add xqtrootdir for UUXQT
 *
 *     Revision 1.21  1993/09/29  04:49:20  ahd
 *     Move priority variables to modem file
 *
 *     Revision 1.20  1993/09/24  03:43:27  ahd
 *     Use positive defaults for unsigned priority values
 *
 *     Revision 1.19  1993/09/20  04:38:11  ahd
 *     TCP/IP support from Dave Watt
 *     't' protocol support
 *     OS/2 2.x support
 *
 *     Revision 1.18  1993/07/31  16:22:16  ahd
 *     Changes in support of Robert Denny's Windows 3.x support
 *
 *     Revision 1.17  1993/07/22  23:19:50  ahd
 *     First pass for Robert Denny's Windows 3.x support changes
 *
 *     Revision 1.16  1993/07/05  14:45:29  ahd
 *     Correct message-of-the-day variable name
 *     Correct WIN32 prefix support
 *
 *     Revision 1.15  1993/06/16  04:03:25  ahd
 *     Special case root directories for UUPC/extended variable default
 *
 *     Revision 1.14  1993/06/15  12:18:06  ahd
 *     Saved changed directory name for debugging
 *
 *     Revision 1.13  1993/05/30  15:25:50  ahd
 *     Multiple driver support
 *
 *     Revision 1.12  1993/05/29  15:19:59  ahd
 *     Add systems file, passwd files
 *
 *     Revision 1.11  1993/05/09  03:49:21  ahd
 *     Support banner, motd strings
 *     Support longname, honordebug, senddebug options
 *
 *     Revision 1.10  1993/04/15  03:17:21  ahd
 *     Add bounce system option
 *
 *     Revision 1.9  1993/04/11  00:31:31  dmwatt
 *     Global edits for year, TEXT, etc.
 *
 *     Revision 1.8  1993/04/05  04:32:19  ahd
 *     Set timezone, windows input mode in common routine
 *
 *     Revision 1.7  1993/04/04  04:57:01  ahd
 *     Default configuration directory from UUPCSYSRC
 *     Default system directories from Configuration directory
 *
 *     Revision 1.6  1993/03/06  22:48:23  ahd
 *     Don't fall off end of shorter tables
 *
 *     Revision 1.5  1993/01/23  19:08:09  ahd
 *     Add Windows/NT to allowed environments
 *
 * Revision 1.4  1992/12/01  04:37:03  ahd
 * Add SpeedOverMemory
 *
 * Revision 1.3  1992/11/22  20:58:55  ahd
 * Normalize directories as read
 * Use strpool to allocate const strings
 *
 * Revision 1.2  1992/11/19  02:56:47  ahd
 * drop rcsid
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

#include "uupcmoah.h"

#include <ctype.h>

#include <io.h>

/*--------------------------------------------------------------------*/
/*                    UUPC/extended include files                     */
/*--------------------------------------------------------------------*/

#include "timestmp.h"
#include "pushpop.h"

#ifdef WIN32
#include <windows.h>
#include "setstdin.h"
#include "pnterr.h"
#endif

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

currentfile();

KWBoolean bflag[F_LAST];       /* Initialized to zero by compiler      */

char **E_internal = NULL;
char *E_nickname = NULL;
char *E_altsignature = NULL;
char *E_anonymous = NULL;
char *E_archivedir = NULL;
char *E_backup = NULL;
char *E_banner = NULL;
char *E_charset = NULL;
char *E_compress= NULL;
char *E_passwd  = NULL;
char *E_systems = NULL;
char *E_confdir = NULL;
char *E_domain = NULL;
char *E_editor = NULL;
char *E_fdomain = NULL;
char *E_filesent = NULL;
char *E_homedir = NULL;
char *E_inmodem = NULL;
char *E_localdomain = NULL;
char *E_mailbox = NULL;
char *E_maildir = NULL;
char *E_mailext = NULL;
char *E_mailserv = NULL;
char *E_motd = NULL;
char *E_name = NULL;
char *E_newsdir = NULL;
char *E_newsserv = NULL;
char *E_nodename = NULL;
char *E_organization = NULL;
char *E_pager = NULL;
char *E_permissions = NULL;
char *E_postmaster = NULL;
char *E_pubdir = NULL;
char *E_replyto = NULL;
char *E_signature = NULL;
char *E_spooldir = NULL;
char *E_tempdir = NULL;
char *E_uncompress = NULL;
char *E_uuxqtpath = NULL;
char *E_version = NULL;
char *E_cwd = NULL;
char *E_xqtRootDir = NULL;
char *E_vmsQueueDir = NULL;
char *E_vmail = NULL;
char **E_ignoreList;           /* Headers not displayed by print      */
char **E_replyToList;          /* Primary Addr used to reply to mail  */
char E_firstGrade = 'C';       /* First class (and above) UUCICO
                                  searches for                        */

char E_mailGrade = 'C';        /* Class mail is transferred at        */
char E_newsGrade = 'n';        /* Class news is transferred at        */

KEWSHORT E_maxhops = 20;                                    /* ahd */
KEWSHORT E_maxuuxqt = 0;      /* Max length of command line for remote */

#ifdef BIT32ENV
KEWSHORT E_newsCache = 128;   /* Pages of news index to cache        */
#else
KEWSHORT E_newsCache = 4;     /* Pages of news index to cache        */
#endif

long     E_batchsize = 65536L;

char *E_tz = NULL;

/*--------------------------------------------------------------------*/
/*                       Local emumerated types                       */
/*--------------------------------------------------------------------*/

typedef enum {
      ENV_UNKNOWN    = 0x0001,
      ENV_DOS        = 0x0002,
      ENV_BIT32      = 0x0004,
      ENV_BIT16      = 0x0008,
      ENV_OS2        = 0x0010,
      ENV_OS2_32BIT  = 0x0020,
      ENV_OS2_16BIT  = 0x0040,
      ENV_WIN        = 0x0080,
      ENV_WIN_32BIT  = 0x0100,
      ENV_WIN_16BIT  = 0x0200
      } ENV_TYPE;

#ifdef WIN32
static ENV_TYPE active_env = ENV_WIN_32BIT | ENV_WIN | ENV_BIT32;
#elif defined(_Windows)
static ENV_TYPE active_env = ENV_WIN_16BIT | ENV_WIN | ENV_BIT16;
#elif defined(__OS2__)
static ENV_TYPE active_env = ENV_OS2_32BIT | ENV_OS2 | ENV_BIT32;
#elif defined(FAMILYAPI)
static ENV_TYPE active_env = ENV_OS2_16BIT | ENV_OS2 | ENV_BIT16;
#else
static ENV_TYPE active_env = ENV_DOS | ENV_BIT16;
#endif

KWBoolean getrcnames(char **sysp,char **usrp);

#ifdef WIN32
static char *getregistry(char *envName, char **value);
#endif

/*--------------------------------------------------------------------*/
/*       UUPC.RC/personal.RC configuration variables.  This table     */
/*       MUST be kept sorted to allow a binary search.                */
/*--------------------------------------------------------------------*/

CONFIGTABLE rcTable[] = {
   {"aliases",      &E_nickname,     B_MUA,     B_TOKEN },
   {"altsignature", &E_altsignature, B_MUA,     B_TOKEN },
   {"anonymouslogin", &E_anonymous,  (B_ALL & ~ B_MAIL), B_GLOBAL|B_TOKEN },
   {"archivedir",   &E_archivedir,   B_OBSOLETE,B_GLOBAL|B_PATH },
   {"backupext",    &E_backup,       B_MUA | B_NEWS,     B_TOKEN },
   {"banner",       &E_banner,       B_UUCICO,  B_GLOBAL|B_PATH },
   {"batchsize",    &E_batchsize,    B_SENDBATS,B_GLOBAL|B_LONG },
   {"charset",      &E_charset,      B_SPOOL,   B_TOKEN|B_GLOBAL },
   {"compress",     &E_compress,     B_SENDBATS,B_GLOBAL|B_STRING },
   {"confdir",      &E_confdir,      B_ALL,     B_GLOBAL|B_PATH },
   {"domain",       &E_domain,       B_ALL,     B_REQUIRED|B_GLOBAL|B_TOKEN },
   {"editor",       &E_editor,       B_MUA,     B_STRING },
   {"filesent",     &E_filesent,     B_MUA,     B_TOKEN },
   {"firstGrade",   &E_firstGrade,   B_UUCICO,  B_CHAR },
   {"folders",      0     ,          B_MUSH,    B_PATH  },
   {"fromdomain",   &E_fdomain,      B_ALL,     B_GLOBAL|B_TOKEN },
   {"home",         &E_homedir,      B_ALL,     B_PATH|B_REQUIRED },
   {"ignore",       &E_ignoreList,   B_MUA,     B_LIST },
   {"inmodem",      &E_inmodem,      B_UUCICO,  B_GLOBAL|B_TOKEN },
   {"internalcommands", &E_internal, B_ALL,     B_GLOBAL|B_LIST },
   {"localdomain",  &E_localdomain,  B_MAIL,    B_GLOBAL|B_TOKEN   },
   {"mailbox",      &E_mailbox,      B_ALL,     B_REQUIRED|B_TOKEN },
   {"maildir",      &E_maildir,      B_MAIL,    B_GLOBAL|B_PATH },
   {"mailext",      &E_mailext,      B_MAIL,    B_TOKEN },
   {"mailgrade",    &E_mailGrade,    B_MTA,     B_CHAR },
   {"mailserv",     &E_mailserv,     B_MAIL|B_NEWS,    B_REQUIRED|B_GLOBAL|B_TOKEN },
   {"maximumhops",  &E_maxhops,      B_MTA,     B_SHORT | B_GLOBAL },
   {"maximumuuxqt", &E_maxuuxqt,     B_MTA,     B_SHORT | B_GLOBAL },
   {"motd",         &E_motd,         B_UUCICO,  B_GLOBAL|B_PATH },
   {"mushdir",      0,               B_MUSH,    B_GLOBAL|B_PATH },
   {"name",         &E_name,         B_INEWS|B_MAIL, B_REQUIRED|B_STRING },
   {"newscache",    &E_newsCache,    B_NEWS,    B_SHORT },
   {"newsdir",      &E_newsdir,      B_NEWS,    B_GLOBAL|B_PATH },
   {"newsgrade",    &E_newsGrade,    B_NEWS,    B_CHAR },
   {"newsserv",     &E_newsserv,     B_NEWS,    B_GLOBAL|B_TOKEN },
   {"nickname",     &E_nickname,     B_MUA,     B_TOKEN },
   {"nodename",     &E_nodename,     B_ALL,     B_REQUIRED|B_GLOBAL|B_TOKEN },
   {"options",      bflag,           B_ALL,     B_BOOLEAN },
   {"organization", &E_organization, B_INEWS|B_MAIL, B_STRING },
   {"pager",        &E_pager,        B_MUA, B_STRING },
   {"passwd",       &E_passwd,       B_ALL,     B_GLOBAL|B_PATH },
   {"path",         &E_uuxqtpath,    B_UUXQT,   B_STRING|B_GLOBAL },
   {"permissions",  &E_permissions,  B_UUCICO|B_UUXQT,   B_GLOBAL|B_PATH },
   {"postmaster",   &E_postmaster,   B_ALL,     B_REQUIRED|B_GLOBAL|B_TOKEN },
   {"priority",     0,               B_OBSOLETE  },
   {"prioritydelta",0,               B_OBSOLETE  },
   {"pubdir",       &E_pubdir,       B_UUCICO|B_UUXQT,   B_GLOBAL|B_PATH },
   {"replyto",      &E_replyto,      B_NEWS|B_MAIL, B_TOKEN },
   {"replytolist",  &E_replyToList,  B_MUA,     B_LIST },
   {"rmail",        0,               B_OBSOLETE  },
   {"rnews",        0,               B_OBSOLETE  },
   {"signature",    &E_signature,    B_NEWS|B_MUA, B_TOKEN },
   {"spooldir",     &E_spooldir,     B_ALL,     B_GLOBAL|B_PATH },
   {"systems",      &E_systems,      B_ALL,     B_GLOBAL|B_PATH },
   {"tempdir",      &E_tempdir,      B_ALL,     B_GLOBAL|B_PATH },
   {"tz",           &E_tz,           B_ALL,     B_TOKEN },
   {"uncompress",   &E_uncompress,   B_RNEWS,   B_GLOBAL|B_STRING  },
   {"version",      &E_version,      B_INSTALL, B_TOKEN },
   {"vmail",        &E_vmail,        B_MTA,     B_PATH },
   {"vmsqueuedir",  &E_vmsQueueDir,  B_MTA,     B_PATH },
   {"xqtrootdir",   &E_xqtRootDir,   B_UUXQT,   B_PATH }
}; /* table */

size_t rcTableSize = (sizeof rcTable / sizeof (CONFIGTABLE));

/*--------------------------------------------------------------------*/
/*               Boolean options shared by all programs               */
/*--------------------------------------------------------------------*/

FLAGTABLE configFlags[] = {
 { "askcc",                   F_ASKCC,                 B_LOCAL},
 { "autoedit",                F_AUTOEDIT,              B_LOCAL},
 { "autoinclude",             F_AUTOINCLUDE,           B_LOCAL},
 { "autoprint",               F_AUTOPRINT,             B_LOCAL},
 { "autosign",                F_AUTOSIGN,              B_LOCAL},
 { "backup",                  F_BACKUP,                B_LOCAL},
 { "bang",                    F_BANG,                  B_GLOBAL},
 { "bounce",                  F_BOUNCE,                B_GLOBAL},
 { "collect",                 F_COLLECTSTATS,          B_GLOBAL},
 { "directory",               F_DIRECT,                B_GLOBAL},
 { "doskey",                  F_DOSKEY,                B_LOCAL},
 { "dot",                     F_DOT,                   B_LOCAL},
 { "escape",                  F_ESCAPE,                B_GLOBAL},
 { "expert",                  F_EXPERT,                B_LOCAL},
 { "fastnews",                F_NEWSRUN,               B_GLOBAL},
 { "forwardsave",             F_SAVERESENT,            B_LOCAL},
 { "fromsep",                 F_FROMSEP,               B_LOCAL},
 { "honorcontrol",            F_HONORCTRL,             B_GLOBAL},
 { "honordebug",              F_HONORDEBUG,            B_GLOBAL},
 { "imfile",                  F_IMFILE,                B_LOCAL},
 { "kanji",                   F_KANJI,                 B_GLOBAL},
 { "longname",                F_LONGNAME,              B_GLOBAL},
 { "monocase",                F_ONECASE,               B_GLOBAL},
 { "multiqueue",              F_MULTI,                 B_GLOBAL},
 { "multitask",               F_MULTITASK,             B_GLOBAL},
 { "newspanic",               F_NEWSPANIC,             B_GLOBAL},
 { "nns",                     F_NNS,                   B_GLOBAL},
 { "pager",                   F_PAGER,                 B_LOCAL},
 { "purge",                   F_PURGE,                 B_LOCAL},
 { "save",                    F_SAVE,                  B_LOCAL},
 { "senddebug",               F_SENDDEBUG,             B_GLOBAL},
 { "shortfrom",               F_SHORTFROM,             B_GLOBAL},
 { "showspool",               F_SHOWSPOOL,             B_GLOBAL},
 { "snews",                   F_SNEWS,                 B_GLOBAL},
 { "speedovermemory",         F_SPEEDOVERMEMORY,       B_LOCAL},
 { "suppressbeep",            F_SUPPRESSBEEP,          B_LOCAL},
 { "suppresscopyright",       F_SUPPRESSCOPYRIGHT,     B_LOCAL},
 { "suppressemptypassword",   F_SUPPRESSEMPTYPASSWORD, B_GLOBAL},
 { "suppressfrom",            F_SUPPRESSFROM,          B_GLOBAL},
 { "suppresslogininfo",       F_SUPPRESSLOGININFO,     B_GLOBAL},
 { "symmetricgrades",         F_SYMMETRICGRADES,       B_GLOBAL},
 { "syslog",                  F_SYSLOG,                B_GLOBAL},
 { "undelete",                F_UNDELETE,              B_LOCAL},
 { "verbose",                 F_VERBOSE,               B_LOCAL},
 { "windows",                 F_WINDOWS,               B_LOCAL}
};


size_t configFlagsSize = (sizeof configFlags / sizeof (FLAGTABLE));

/*--------------------------------------------------------------------*/
/*             Operating environment configuration table              */
/*--------------------------------------------------------------------*/

typedef struct _ENVLIST
{
   char *name;
   ENV_TYPE value;
} ENVLIST;

static ENVLIST osEnvTable[] =
{
   { "dos",      ENV_DOS      },
   { "16bit",    ENV_BIT16    },
   { "32bit",    ENV_BIT32    },
   { "32bitos2", ENV_OS2_16BIT},
   { "16bitos2", ENV_OS2_32BIT},
   { "os2",      ENV_OS2      },
   { "win32",    ENV_WIN_32BIT},
   { "win16",    ENV_WIN_16BIT},
   { "32bitwin", ENV_WIN_32BIT},
   { "16bitwin", ENV_WIN_16BIT},
   { "win",      ENV_WIN      },
   { NULL,       ENV_UNKNOWN  }
};

/*--------------------------------------------------------------------*/
/*       Configuration variables extracted from environment, and      */
/*       their names                                                  */
/*--------------------------------------------------------------------*/

typedef struct _ENV_VARS
{
   char *envName;
   char *confName;
} ENV_VARS;

static ENV_VARS envVarList[] =
{
   { "EDITOR",   "EDITOR"   },
   { "HOME",     "HOME"     },
   { "NAME",     "NAME"     },
   { "HOSTNAME", "NODENAME" },   /* Useful for OS/2 TCP/IP        */
   { "MAILBOX",  "MAILBOX"  },
   { "LOGNAME",  "MAILBOX"  },   /* Same as RCS                   */
   { "USER",     "MAILBOX"  },   /* Useful for OS/2 TCP/IP        */
   { "USERNAME", "MAILBOX"  },   /* Useful for NT                 */
   { "TEMP",     "TEMPDIR"  },
   { "TMP",      "TEMPDIR"  },
   { "TZ",       "TZ"       },
   { NULL,       NULL       }
} ;

typedef struct _DEFAULTS
{
   char **value;
   char *literal;
   KWBoolean path;
} DEFAULTS;

/*--------------------------------------------------------------------*/
/*       List of directories automatically initialized to be off      */
/*       root configuration directory, if needed.                     */
/*--------------------------------------------------------------------*/

static DEFAULTS directoryList[] =
{
   {&E_maildir,      "mail"    , KWTrue },
   {&E_newsdir,      "news"    , KWTrue },
   {&E_pubdir,       "public"  , KWTrue },
   {&E_spooldir,     "spool"   , KWTrue },
   {&E_tempdir,      "tmp"     , KWTrue },
   {&E_systems,      "systems" , KWTrue },
   {&E_passwd,       "passwd"  , KWTrue },
   {&E_permissions,  "permissn", KWTrue },
   { NULL  }
} ;

/*--------------------------------------------------------------------*/
/*    p r o c e s s K e y w o r d                                     */
/*                                                                    */
/*    Given a keyword in the configuration, process it                */
/*--------------------------------------------------------------------*/

static KWBoolean
processKeyword( CONFIGTABLE *tptr,
                char *cp )
{
   char **varPtr = tptr->loc;

/*--------------------------------------------------------------------*/
/*                      Handle obsolete options                       */
/*--------------------------------------------------------------------*/

   if (tptr->flag & B_OBSOLETE)
   {
      printmsg(2,"Obsolete keyword \"%s\" ignored.", tptr->sym );
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*          Handle options for which no storage is allocated          */
/*--------------------------------------------------------------------*/

   if ( tptr->loc == 0 )
   {

#ifdef UDEBUG
      printmsg(10,"Dummy entry %s ignored.", tptr->sym );
#endif
      return KWFalse;

   }

/*--------------------------------------------------------------------*/
/*                       Handle integer values                        */
/*--------------------------------------------------------------------*/

   if (tptr->flag & (B_SHORT|B_LONG))
   {
      long foo;
      cp = strtok(cp,WHITESPACE);

      if ( equal(cp,"0"))
         foo = 0;
      else {

         foo = atol(cp);

         if ( foo == 0)
         {
            printmsg(0,
               "Unable to convert \"%s\" value \"%s\" to integer",
               tptr->sym, cp);

            return KWFalse;

         } /* if */

      } /* else */

#ifdef UDEBUG
      printmsg(10,"Assigning keyword %s numeric value %ld",
                   tptr->sym,
                   foo );
#endif

      if (tptr->flag & B_LONG)
         *((long *) tptr->loc) = foo;
      else
         *((KEWSHORT *) tptr->loc) = (KEWSHORT) foo;

      return KWTrue;

   } /* if (tptr->flag & (B_SHORT|B_LONG)) */

/*--------------------------------------------------------------------*/
/*       Begin processing character types:  lists, strings,           */
/*       tokens, and single characters                                */
/*--------------------------------------------------------------------*/

#ifdef UDEBUG
      printmsg(10,"Assigning keyword %s string value \"%s\"",
                   tptr->sym,
                   cp );
#endif

/*--------------------------------------------------------------------*/
/*                       Handle lists of tokens                       */
/*--------------------------------------------------------------------*/

   if (tptr->flag & (B_LIST | B_CLIST))
   {
      char **list = malloc( (MAXLIST+1) * sizeof (*list));
      char *colon;
      int words;

      checkref( list );

      if (tptr->flag & B_CLIST)  /* Use colon as delimiter?  */
      {
         while ( (colon = strchr( cp , ':')) != NULL)
            *colon = ' ';     /* Make colons spaces ...      */
      }

      words = getargs(cp, list);

      if( words > MAXLIST)
         panic();

      if (words > 0)
      {
         if ( *(varPtr) )
            free( *(varPtr) );

         list = realloc( list, (size_t) (words+1) * sizeof(*list));
         checkref( list );
         *(varPtr) = (char *) list;
         list[words] = NULL;

         while( *list != NULL)
         {

            if (strlen(*list))
            {
               *list = newstr(*list);
               checkref( *list++ );
            }
            else
               *list++ = "";

         } /* while */

      } /* if (words > 0) */
      else {

         printmsg(0,"No parameters given for keyword \"%s\"",
                  tptr->sym);

         return KWFalse;

      } /* else */

      return KWTrue;

   } /* if */

/*--------------------------------------------------------------------*/
/*                  Handle single tokens and strings                  */
/*--------------------------------------------------------------------*/

   while( *cp == ' ' )     /* Trim leading whitespace     */
      cp++;

   if (*cp == '\0')
   {
      printmsg(0,"No parameter given for keyword \"%s\""
               ", ignored.",
               tptr->sym);

      return KWFalse;
   } /* if */

   if (tptr->flag & (B_TOKEN|B_CHAR))  /* One word value?   */
      cp = strtok(cp,WHITESPACE); /* Yes --> Tokenize */

   if (tptr->flag & B_NORMAL)  /* Normalize path?     */
      cp = normalize( cp );

   if (tptr->flag & B_CHAR )   /* Simple character?   */
   {
      if ( strlen( cp ) > 1 )
      {
         printmsg(0,"Keyword %s value \"%s\" "
                    "length exceeds one character",
                    tptr->sym,
                    cp );
         return KWFalse;
      }
      else
         *((char *) tptr->loc) = *cp;

   } /* if (tptr->flag & B_CHAR ) */
   else if (tptr->flag & B_MALLOC)  /* Allocate normally?  */
   {
      *(varPtr) = strdup(cp);    /* Save string           */

      checkref( *(varPtr) );     /* Verify malloc()       */
   }
   else
      *(varPtr) = newstr(cp);    /* Save string           */

   return KWTrue;

}  /* processKeyword */

/*--------------------------------------------------------------------*/
/*    p r o c e s s c o n f i g                                       */
/*                                                                    */
/*    Handle a single line of a configuration file                    */
/*--------------------------------------------------------------------*/

KWBoolean
processconfig( char *buff,
               SYSMODE sysmode,
               CONFIGBITS program,
               CONFIGTABLE *table,
               const size_t tableSize,
               FLAGTABLE *bTable,
               const size_t bTableSize)
{
   CONFIGTABLE *tptr = NULL;
   char *cp;
   char *period;
   char *keyword;
   ENV_TYPE target_env;

   int lower = 0;
   int upper = (int) tableSize - 1;

/*--------------------------------------------------------------------*/
/*                break out the keyword from its value                */
/*--------------------------------------------------------------------*/

   if ((cp = strchr(buff, '=')) == nil(char))
   {
      printmsg(0,"Missing equals sign after keyword \"%s\", ignored",
                  buff);
      return KWFalse;
   }

   *cp++ = '\0';

   strlwr(buff);

/*--------------------------------------------------------------------*/
/*    Determine if the keyword should processed in this environment   */
/*--------------------------------------------------------------------*/

   period = keyword = strchr( buff, '.' );
                              /* Look for environment prefix          */

   if ( keyword == NULL )     /* No environment?                      */
   {
      keyword = buff;         /* Then buffer starts with keyword      */
      target_env = active_env;
   }
   else {

      short subscript = 0;

      *keyword++ = '\0';      /* Terminate environment string         */
      target_env = ENV_UNKNOWN;

      while( osEnvTable[subscript].name != NULL)
      {

         if (equal( osEnvTable[subscript].name, buff ))
         {
            target_env = osEnvTable[subscript].value;
            break;
         }
         else
            subscript ++;

      } /* while */

      if ( target_env == ENV_UNKNOWN )
      {
         printmsg(0,"Unknown environment \"%s\", keyword \"%s\" ignored",
               buff, keyword );

         return KWTrue;
      }

   } /* else */

   if ( period != NULL )
      *period = '.';

/*--------------------------------------------------------------------*/
/*            Binary search to locate keyword in the table            */
/*--------------------------------------------------------------------*/

   while( (tptr == NULL) && (lower <= upper) )
   {
      int midpoint = ( lower + upper ) / 2;
      int hit = strcmp( keyword, table[midpoint].sym );

      if (hit > 0)
         lower = midpoint + 1;
      else if (hit < 0)
         upper = midpoint - 1;
      else
         tptr = &table[midpoint];
   }

   if ( tptr == NULL )              /* Search fail?                  */
      return KWFalse;               /* Yes, report same to caller    */

/*--------------------------------------------------------------------*/
/*            Skip the keyword because of the environment?            */
/*--------------------------------------------------------------------*/

   if (!(((unsigned long) active_env) & (unsigned long) target_env) )
   {
       printmsg(2,"%s-environment keyword \"%s\" skipped.",
                   strupr(buff), keyword);
       return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*                  Handle mis-placed system options                  */
/*--------------------------------------------------------------------*/

   if ((tptr->flag & B_GLOBAL) && (sysmode != SYSTEM_CONFIG))
   {
       printmsg(0,
          "User specified system keyword \"%s\" ignored.",
          keyword);
       return KWTrue;
   }

/*--------------------------------------------------------------------*/
/*       Skip options we don't need for this particular program       */
/*--------------------------------------------------------------------*/

   if (program && !(tptr->program & program))
   {
      tptr->flag |= B_FOUND;
      return KWTrue;                /* No operation if we don't need
                                       the keyword                   */
   }

/*--------------------------------------------------------------------*/
/*                       Handle Boolean options                       */
/*--------------------------------------------------------------------*/

   if (tptr->flag & B_BOOLEAN )
      options(cp,
              sysmode,
              bTable,
              (KWBoolean *) tptr->loc,
              bTableSize);

/*--------------------------------------------------------------------*/
/*                        Process the keyword                         */
/*--------------------------------------------------------------------*/

  else if (processKeyword( tptr, cp ))
     tptr->flag |= B_FOUND;

  return KWTrue;

} /* processconfig */

/*--------------------------------------------------------------------*/
/*    g e t c o n f i g                                               */
/*                                                                    */
/*    Process a single configuration file                             */
/*--------------------------------------------------------------------*/

KWBoolean getconfig(FILE *fp,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  const size_t tableSize,
                  FLAGTABLE *bTable,
                  const size_t bTableSize)
{

   char buff[BUFSIZ];
   char *cp;

   while(!(fgets(buff, sizeof buff, fp) == nil(char)))
   {

/*--------------------------------------------------------------------*/
/*                        Ingore comment lines                        */
/*--------------------------------------------------------------------*/

      if (*buff == '#')
         continue;            /* comment line   */

/*--------------------------------------------------------------------*/
/*                       Drop trailing new line                       */
/*--------------------------------------------------------------------*/

      if (*(cp = buff + strlen(buff) - 1) == '\n')
         *cp = '\0';

/*--------------------------------------------------------------------*/
/*                 Drop leading blanks on input line                  */
/*--------------------------------------------------------------------*/

      cp = buff;

      while( isspace( *cp ) )
         cp ++ ;

/*--------------------------------------------------------------------*/
/*                 If line was not blank, process it.                 */
/*--------------------------------------------------------------------*/

     if (*cp != '\0')
     {
        if ( ! processconfig( cp,
                              sysmode,
                              program,
                              table,
                              tableSize,
                              bTable,
                              bTableSize ) )
            printmsg(0,
               "Unknown keyword \"%s\" in %s configuration file ignored",
               buff, sysmode ? "system" : "user");
     }

   } /* while */

   return KWTrue;

} /* getconfig */

/*--------------------------------------------------------------------*/
/*    o p t i o n s                                                   */
/*                                                                    */
/*    Process a line of KWBoolean option flags.                       */
/*--------------------------------------------------------------------*/

void
options(char *s,
        SYSMODE sysmode,
        FLAGTABLE *flags,
        KWBoolean *barray,
        const size_t flagSize )
{
   char *token;

   strlwr(s);
   token = strtok(s,WHITESPACE);

   while (token != NULL)
   {
      KWBoolean newValue;
      char *compare;
      int upper, lower;

      if ( equaln(token,"no",2) && (strlen(token) > 2) )
      {
         newValue = KWFalse;
         compare = token + 2;
      }
      else {
         newValue = KWTrue;
         compare = token;
      }

      upper = (int) flagSize - 1;
      lower = 0;

/*--------------------------------------------------------------------*/
/*            Binary search to locate keyword in the table            */
/*--------------------------------------------------------------------*/

      while( lower <= upper)
      {
         int midpoint = ( lower + upper ) / 2;
         int hit = strcmp( compare, flags[midpoint].sym );

         if (hit > 0)
            lower = midpoint + 1;
         else if (hit < 0)
            upper = midpoint - 1;
         else {
            if ((flags[midpoint].bits & B_GLOBAL) &&
                (sysmode != SYSTEM_CONFIG))
            {
               printmsg(0,"options: System boolean option %s "
                          "only be specified in system configuration file.",
                          token );
            }
            else {
               barray[ flags[midpoint].position ] = newValue;
            }

            break;

         } /* else */

      } /* while( lower <= upper) */

      if ( lower > upper )
      {
         strupr( token );
         printf("options: Invalid boolean option %s specified\n", token);
      }

      token = strtok(NULL,WHITESPACE);  /* Step to next token on line */

   } /* while */

} /* options */

/*--------------------------------------------------------------------*/
/*    c o n f i g u r e                                               */
/*                                                                    */
/*    Define the global parameters of UUPC/extended                   */
/*--------------------------------------------------------------------*/

KWBoolean configure( CONFIGBITS program)
{
   char *sysrc, *usrrc;
   FILE *fp;
   KWBoolean success;
   char buf[BUFSIZ];
   size_t subscript = 0;
   char *s;

/*--------------------------------------------------------------------*/
/*     In Windows/NT, set the console input mode to non-linebased     */
/*--------------------------------------------------------------------*/

#ifdef WIN32
   setstdinmode();
#endif

#ifdef __IBMC__
   logfile = stdout;
#endif

/*--------------------------------------------------------------------*/
/*                  Determine the active environment                  */
/*--------------------------------------------------------------------*/

   if (!getrcnames(&sysrc, &usrrc))
      return KWFalse;

/*--------------------------------------------------------------------*/
/*          Extract selected variables from our environment           */
/*--------------------------------------------------------------------*/

   for ( subscript = 0; envVarList[subscript].envName != NULL; subscript++ )
   {
      s = getenv( envVarList[subscript].envName );

      if (s != NULL )
      {
         sprintf(buf,"%s=%s", envVarList[subscript].confName, s );
         printmsg(5,"Inserting environment variable %s as %s",
                     s, buf );

         if ( !processconfig( buf,
                              SYSTEM_CONFIG,
                              program,
                              rcTable,
                              rcTableSize,
                              configFlags,
                              configFlagsSize ))
         {
            printmsg(0,"Internal error: Invalid keyword %s",
                       envVarList[subscript].confName  );
            panic();
         }

      } /* if (sysrc != NULL ) */

   } /* for ( subscript = 0; ... ) */

/*--------------------------------------------------------------------*/
/*       If the user specified a user name in the environment but     */
/*       not a UUPCUSRRC variable, build a UUPCUSRRC variable         */
/*--------------------------------------------------------------------*/

   if ((usrrc == nil(char)) && ( E_mailbox != NULL ))
   {                             /* Can build from the environment?  */
      strcpy( buf, E_mailbox );
      strcat( buf, ".rc" );
      usrrc = newstr( buf );
      printmsg(2,"Using UUPCUSRRC=%s", usrrc );
   }

/*--------------------------------------------------------------------*/
/*          Determine configuration directory from UUPCSYSRC          */
/*--------------------------------------------------------------------*/

   E_confdir = normalize( sysrc );     /* Make 'em all slashes        */

   s = strrchr( E_confdir, '/' );      /* Get end of path component   */
   if ( s == NULL )                    /* There WAS one, right?       */
   {                                   /* Er, no, sorry.              */
      printmsg(0,"No path name in UUPCSYSRC: %s", sysrc);
      panic();
   }

   *(s+1) = '\0';                      /* Terminate for Config Directory */
   E_confdir = newstr(normalize( E_confdir ));
                                       /* Drop trailing slash unless  */
                                       /* root directory and save     */

/*--------------------------------------------------------------------*/
/*               Process the system configuration file                */
/*--------------------------------------------------------------------*/

   if ((fp = FOPEN(sysrc, "r",TEXT_MODE)) == nil(FILE))
   {
      printmsg(0, "Cannot open system configuration file \"%s\"", sysrc);
      printerr(sysrc);
      return KWFalse;
   }

   PushDir( E_confdir );

   success = getconfig(fp,
                       SYSTEM_CONFIG,
                       program,
                       rcTable,
                       rcTableSize,
                       configFlags,
                       configFlagsSize );

   fclose(fp);

   if (!success)
   {
      PopDir();
      return KWFalse;
   }

/*--------------------------------------------------------------------*/
/*                Process the user configuration value                */
/*--------------------------------------------------------------------*/

   if (usrrc != nil(char))
   {
      usrrc = normalize( usrrc );

      if ((fp = FOPEN(usrrc, "r",TEXT_MODE)) == nil(FILE))
      {
         printmsg(0, "Cannot open user configuration file \"%s\"", usrrc);
         PopDir();
         return KWFalse;
      }

      success = getconfig(fp,
                           USER_CONFIG,
                           program,
                           rcTable,
                           rcTableSize,
                           configFlags,
                           configFlagsSize );

      fclose(fp);

      if (!success)
      {
         PopDir();
         return KWFalse;
      }

   } /* if (usrrc != nil(char)) */

/*--------------------------------------------------------------------*/
/*                       Display our copyright                        */
/*--------------------------------------------------------------------*/

   if (! bflag[F_SUPPRESSCOPYRIGHT] &&
        (program != B_MTA) &&
        isatty(fileno(stdout)))
      fprintf(stdout,
"Changes and Compilation Copyright (c) 1989-1996 by Kendra Electronic\n"
"Wonderworks.  May be freely distributed for reasonable copying fee\n"
"if original documentation and source is included.  See license for\n"
"details and restrictions.\n");

/*--------------------------------------------------------------------*/
/*          Validate that all required parameters were given          */
/*--------------------------------------------------------------------*/

   for (subscript = 0; subscript < rcTableSize; subscript++ )
   {

      if ((rcTable[subscript].flag & (B_REQUIRED | B_FOUND)) == B_REQUIRED)
      {
         printmsg(0, "%s configuration parameter \"%s\" must be set.",
            (rcTable[subscript].flag & B_GLOBAL) ? "System" : "User",
            rcTable[subscript].sym);
         success = KWFalse;
      } /* if */

   } /* for */

/*--------------------------------------------------------------------*/
/*                     Fill in derived parameters                     */
/*--------------------------------------------------------------------*/

   subscript = 0;

   while( directoryList[subscript].value != NULL )
   {
      if ( *(directoryList[subscript].value) == NULL )
         *(directoryList[subscript].value) = directoryList[subscript].path ?
                     newstr( normalize(directoryList[subscript].literal) ) :
                     directoryList[subscript].literal;
      subscript++;
   }

/*--------------------------------------------------------------------*/
/*                         Set our time zone                          */
/*--------------------------------------------------------------------*/

   if (E_tz != NULL)
   {
      sprintf( buf, "TZ=%s", E_tz );
      E_tz = newstr( buf );
      putenv( E_tz );
   }

   tzset();                      /* Set up time zone information  */

   PopDir();

   return success;

} /*configure*/

/*--------------------------------------------------------------------*/
/*    g e t r c n a m e s                                             */
/*                                                                    */
/*    Return the name of the configuration files                      */
/*--------------------------------------------------------------------*/

KWBoolean getrcnames(char **sysp,char **usrp)
{
   char *debugp = NULL;      /* Pointer to debug environment variable */

   if ((*sysp = getenv(SYSRCSYM)) == nil(char)
#if defined(WIN32)
      && (getregistry(SYSRCSYM, sysp) == nil(char))
#endif
   )
   {
      printf("Environment variable %s must be specified\n", SYSRCSYM);
      return KWFalse;
   }

   *usrp = getenv(USRRCSYM);

#if defined(WIN32)
   if (*usrp == nil(char))
      getregistry(USRRCSYM, usrp);
#endif

   debugp = getenv(SYSDEBUG);

   if ( debugp != nil(char))        /* Debug specified in environment? */
      debuglevel = atoi(debugp);    /* Yes --> preset debuglevel for user */

   return KWTrue;

} /*getrcnames*/

/*--------------------------------------------------------------------*/
/*       I s D O S                                                    */
/*                                                                    */
/*       Reports if current enviroment is DOS                         */
/*--------------------------------------------------------------------*/

KWBoolean IsDOS( void )
{
   if ( ((unsigned long) active_env) & ENV_DOS )
      return KWTrue;
   else
      return KWFalse;

} /* IsDOS */

#if defined(WIN32)
static HKEY uupcMachineKey = INVALID_HANDLE_VALUE;
static HKEY uupcUserKey = INVALID_HANDLE_VALUE;

char *getregistry(char *envName, char **value)
{
   LONG result;
   DWORD dwType;
   char bData[BUFSIZ];
   DWORD cbData = BUFSIZ;

   *value = NULL;

   if (uupcMachineKey == INVALID_HANDLE_VALUE)
   {
      result = RegOpenKey(HKEY_LOCAL_MACHINE, REGISTRYHIVE, &uupcMachineKey);

      if (result != ERROR_SUCCESS)
      {
         printmsg(2, "getregistry: could not open LOCAL_MACHINE hive");
         if (debuglevel > 1)
            printNTerror("RegOpenKey", result);
      }
   }

   if (uupcUserKey == INVALID_HANDLE_VALUE)
   {
      result = RegOpenKey(HKEY_CURRENT_USER, REGISTRYHIVE, &uupcUserKey);
      if (result != ERROR_SUCCESS)
      {
         printmsg(2, "getregistry: could not open CURRENT_USER hive");
         if (debuglevel > 1)
            printNTerror("RegOpenKey", result);
      }
   }

/*--------------------------------------------------------------------*/
/*   Check the user's hive first, then the machine's hive             */
/*--------------------------------------------------------------------*/

   result = RegQueryValueEx(uupcUserKey, envName, NULL, &dwType, bData, &cbData);
   if (result == ERROR_SUCCESS && dwType == REG_SZ)
   {
      printmsg(2, "Found user registry entry %s, value %s", envName, bData);
      *value = newstr(bData);
      return *value;
   }

   result = RegQueryValueEx(uupcMachineKey, envName, NULL, &dwType, bData, &cbData);

   if (result == ERROR_SUCCESS && dwType == REG_SZ)
   {
      printmsg(2, "Found machine registry entry %s, value %s", envName, bData);
      *value = newstr(bData);
      return *value;
   }

   *value = NULL;
   return *value;
}

#endif
