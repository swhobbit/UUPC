/*--------------------------------------------------------------------*/
/*      l i b . h                                                     */
/*                                                                    */
/*      Update log:                                                   */
/*                                                                    */
/*      13 May 89    Added PCMAILVER                        ahd       */
/*      Summer 89    Added equali, equalni, compiled,                 */
/*                         compilet                         ahd       */
/*      22 Sep 89    Add boolean typedef                    ahd       */
/*      01 Oct 89    Make logecho boolean                   ahd       */
/*      19 Mar 90    Move FOPEN prototype to here           ahd       */
/*      02 May 1990  Allow set of booleans options via options=       */
/*  8 May  90  Add 'pager' option                                     */
/* 10 May  90  Add 'purge' option                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*    Changes Copyright (c) 1989 by Andrew H. Derbyshire.             */
/*                                                                    */
/*    Changes Copyright (c) 1990-1992 by Kendra Electronic            */
/*    Wonderworks.                                                    */
/*                                                                    */
/*    All rights reserved except those explicitly granted by the      */
/*    UUPC/extended license agreement.                                */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.2 1992/11/22 21:31:22 ahd Exp ahd $
 *
 *    Revision history:
 *    $Log: lib.h $
 * Revision 1.2  1992/11/22  21:31:22  ahd
 * Allow strpool() to allocate memory for const strings
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

#ifndef __LIB
#define __LIB

/*--------------------------------------------------------------------*/
/*     Environment variables set by UUXQT before invoking modules     */
/*--------------------------------------------------------------------*/

#define UU_USER "UU_USER"
#define UU_MACHINE "UU_MACHINE"
#define LOGNAME "LOGNAME"
#define RMAIL   "rmail"

/*--------------------------------------------------------------------*/
/*                   Global configuration variables                   */
/*--------------------------------------------------------------------*/

#define SYSRCSYM "UUPCSYSRC"
#define USRRCSYM  "UUPCUSRRC"
#define SYSDEBUG  "UUPCDEBUG"    /* Initialize debug level ahd       */

/*--------------------------------------------------------------------*/
/*                 Macro for recording when UUPC dies                 */
/*--------------------------------------------------------------------*/

#define panic()  bugout( __LINE__, cfnptr)

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printerr( x )  prterror( __LINE__, cfnptr, x)

/*--------------------------------------------------------------------*/
/*                  Convert hours/minutes to seconds                  */
/*--------------------------------------------------------------------*/

#define hhmm2sec(HHMM)    ((time_t)(((HHMM / 100) * 60L) + \
                           (time_t)(HHMM % 100)) * 60L)

/*--------------------------------------------------------------------*/
/*                     Configuration file defines                     */
/*--------------------------------------------------------------------*/

#define B_REQUIRED 0x00000001L /* Line must appear in configuration   */
#define B_FOUND    0x00000002L /* We found the token                  */

#define B_GLOBAL   0x00000004L /* Must not appear in PERSONAL.RC      */
#define B_LOCAL    0x00000008L /* The opposite of B_GLOBAL, sort of   */

#define B_MTA      0x00000010L /* Used by Mail Delivery (RMAIL)       */
#define B_MUA      0x00000020L /* Used by Mail User Agent (MAIL)      */
#define B_MUSH     0x00000040L /* Used by MUSH - Not used by UUPC     */
#define B_NEWS     0x00000080L /* Used by NEWS software               */
#define B_UUCICO   0x00000100L /* Used by transport program UUCICO    */
#define B_UUCP     0x00000200L /* Used by UUCP command                */
#define B_UUPOLL   0x00000400L /* UUPOLL program                      */
#define B_UUSTAT   0x00000800L /* UUSTAT, UUSUB, UUNAME programs      */
#define B_UUXQT    0x00001000L /* Used by queue processor UUXQT       */
#define B_INSTALL  0x00002000L /* Used by install program only        */
#define B_MAIL     (B_MUA | B_MTA | B_MUSH)
#define B_SPOOL    (B_MTA | B_UUCICO | B_UUXQT | B_UUCP | B_UUSTAT)
#define B_ALL      (B_MAIL|B_SPOOL|B_NEWS|B_UUPOLL|B_UUSTAT)

#define B_INTEGER  0x00010000L /* Pointer is to short int, not string */
#define B_TOKEN    0x00020000L /* Pointer is one word, ignore blanks  */
#define B_BOOLEAN  0x00040000L /* Pointer is to boolean keywords      */
#define B_LIST     0x00080000L /* Pointer to array of char pointers   */
#define B_CLIST    0x00100000L /* Pointer to array of char pointers,
                                  input is separated by colons, not
                                  spaces                              */
#define B_STRING   0x00200000L /* String value (same as original UUPC
                                  configuration processor             */
#define B_NORMAL   0x00400000L /* Normalize backslashes to slashes in
                                  in this variable                    */
#define B_OBSOLETE 0x00800000L /* Option is obsolete, should be
                                  deleted                             */
#define B_MALLOC   0x01000000L  /* Use malloc(), not newstr()         */
#define B_PATH     (B_TOKEN | B_NORMAL)
                               /* DOS Path name                       */

/*--------------------------------------------------------------------*/
/*                     General defines and macros                     */
/*--------------------------------------------------------------------*/

#define  MESSAGESEP "\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\n"

#define DCSTATUS    "hostatus"
#define PASSWD      "passwd"
#define PATHS       "hostpath"
#define PERMISSIONS "permissn"
#define SYSLOG      "syslog"
#define SYSTEMS     "systems"
#define ACTIVE      "active"

#define WHITESPACE " \t\n\r"

/*--------------------------------------------------------------------*/
/*    Equality macros                                                 */
/*--------------------------------------------------------------------*/

#define equal(a,b)               (!strcmp(a,b))
#define equali(a,b)              (!stricmp(a,b))                     /*ahd */
#define equalni(a,b,n)           (!strnicmp(a,b,n))                  /*ahd */
#define equaln(a,b,n)            (!strncmp(a,b,n))

#ifndef __FILE__
#error __FILE__ must be defined!!!
#endif

#define currentfile()            static char *cfnptr = __FILE__
#define checkref(a)              (checkptr(a, cfnptr ,__LINE__))     /*ahd */
#define newstr(a)                (strpool(a, cfnptr ,__LINE__))
#ifndef NOSAFEFREE
#define free(a)                  (safefree(a, cfnptr ,__LINE__))
#endif

#define nil(type)               ((type *)NULL)

/*--------------------------------------------------------------------*/
/*    Enumerated list used by the options= configuration line         */
/*--------------------------------------------------------------------*/

typedef enum {

/*--------------------------------------------------------------------*/
/*                          Per user options                          */
/*--------------------------------------------------------------------*/

               F_ASKCC,       /* TRUE = ask for Carbon copies              */
               F_DOSKEY,      /* TRUE = Use DOSKEY under DOS 5 if available*/
               F_BACKUP,      /* TRUE = Backup mailbox before rewriting    */
               F_DOT,         /* TRUE = period ends a message              */
               F_AUTOPRINT,   /* TRUE = print next message automatically   */
               F_AUTOEDIT,    /* TRUE = no line prompt, always edit        */
               F_AUTOINCLUDE, /* TRUE = Perform automatic INCLUDE command  */
               F_AUTOSIGN,    /* TRUE = append the signature file          */
               F_EXPERT,      /* TRUE = Let user shoot self in foot w/o
                                        a message.                         */
               F_FROMSEP,     /* TRUE = Allow From to split messages       */
               F_PAGER,       /* TRUE = Invert meaning of P/p T/t commands */
               F_PURGE,       /* TRUE = Delete mailbox, if empty           */
               F_SAVE,        /* TRUE = Save read messages in =mbox when
                                        reading new mail.                  */
               F_SAVERESENT,  /* TRUE = Save forwarded mail a second time  */
               F_SUPPRESSCOPYRIGHT,
                              /* Skip copyright message                    */
               F_SPEEDOVERMEMORY,
                              /* TRUE = Be lazy in strpool()               */
               F_VERBOSE,     /* TRUE = Verbose RMAIL output               */

/*--------------------------------------------------------------------*/
/*                    Per system (GLOBAL) options                     */
/*--------------------------------------------------------------------*/

               F_BANG,        /* TRUE = re-write addresses in bang (!) form*/
               F_COLLECTSTATS,/* TRUE = Report additional information in
                                        various logs                       */
               F_DIRECT,      /* TRUE = Deliver to subdirectories, not
                                        files                              */
               F_ESCAPE,      /* TRUE = Allow ESCAPE to act as Ctrl-Break  */
               F_HPFS,        /* TRUE = Exploit HPFS names under OS/2      */
               F_HISTORY,     /* TRUE = Maintain history of of articles
                                        read and posted                    */
               F_KANJI,       /* TRUE = enable Kanji (Japanese) support    */
               F_MULTI,       /* TRUE = Deliver to multiple addresses on
                                        remote host at once                */
               F_MULTITASK,   /* TRUE = System is multitasking, watch
                                        for race conditions                */
               F_ONECASE,     /* TRUE = Remote host is case insensitive    */
               F_SNEWS,       /* TRUE = Use Simple news delivery           */
               F_SYMMETRICGRADES,
                              /* TRUE = Use send grade as receive grade    */
               F_SYSLOG,      /* TRUE = Write syslog with name and time of
                                        each file transferred              */
               F_UNDELETE,    /* TRUE = Do not override OS/2 undelete
                                 support                                   */
               F_LAST }       /* Dummy - Must be last - defines array size */
               B_FLAG;        /* Boolean flag array - ahd                  */

/*--------------------------------------------------------------------*/
/*                  Your basic Boolean logic values                   */
/*--------------------------------------------------------------------*/

#undef FALSE
#undef TRUE
typedef enum { FALSE = 0, TRUE = 1 } boolean;

/*--------------------------------------------------------------------*/
/*                      Configuration mode flags                      */
/*--------------------------------------------------------------------*/

typedef enum {
      USER_CONFIG,
      SYSTEM_CONFIG,
      MODEM_CONFIG
      } SYSMODE;

/*--------------------------------------------------------------------*/
/*               enumerated type for configuration bits               */
/*--------------------------------------------------------------------*/

typedef unsigned long CONFIGBITS;

typedef unsigned short INTEGER;  /* Integers in the config file      */

typedef unsigned int BPS;

/*--------------------------------------------------------------------*/
/*                     Configuration table layout                     */
/*--------------------------------------------------------------------*/

typedef struct ConfigTable {
   char *sym;
   char **loc;
   CONFIGBITS bits;
} CONFIGTABLE;

/*--------------------------------------------------------------------*/
/*                    Boolean options table layout                    */
/*--------------------------------------------------------------------*/

typedef struct FlagTable {
   char *sym;
   int position;
   CONFIGBITS bits;
} FLAGTABLE;

/*--------------------------------------------------------------------*/
/*                            linked list                             */
/*--------------------------------------------------------------------*/

struct file_queue {
   char name[FILENAME_MAX];
   struct file_queue *next_link;
} ;

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

extern int debuglevel;
extern FILE *logfile;
extern boolean bflag[F_LAST];

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
extern INTEGER E_maxhops;

extern FLAGTABLE configFlags[];

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

void prterror(const size_t lineno, const char *fname, const char *prefix);

extern void checkptr(const void *block, const char *file, const int line);

extern int MKDIR(const char *path);
                              /* Make a directory              ahd */

extern int CHDIR(const char *path);
                              /* Change to a directory          ahd */

int RENAME(const char *oldname, const char *newname );

extern FILE *FSOPEN(const char *name,
                   const char *mode );

#define FOPEN( name, mode, ftype ) FSOPEN( name, mode ftype )
                              /* Since the last two are literals,
                                 we just concat them together for
                                 the call                         */

extern void options(char *s,
                    SYSMODE sysmode ,
                    FLAGTABLE *flags,
                    boolean *xflag);

int getargs(char *line,
            char **flds);                                   /* ahd */

void printmsg(int level, char *fmt, ...);

boolean configure( CONFIGBITS program );

boolean getconfig(FILE *fp,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  FLAGTABLE *btable);

boolean processconfig(char *buff,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  FLAGTABLE *btable);

void bugout( const size_t lineno, const char *fname);

char *strpool( const char *input , const char *file, size_t line);

void safefree( void *input , const char *file, size_t line);

char *normalize( const char *path );

#ifdef __GNUC__
char *strlwr( char *s );
#endif

/*--------------------------------------------------------------------*/
/*                   Compiler specific information                    */
/*--------------------------------------------------------------------*/

#if defined(__CORE__)
extern   char *copyright;
extern   char *copywrong;
extern   long *lowcore;
#endif   /* defined(__CORE__)   */
#endif   /* ifndef __LIB */

#define msdos                 /* Required by dain bramaged compiler  */
