/*--------------------------------------------------------------------*/
/*      l i b . h                                                     */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
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
 *    $Id: LIB.H 1.7 1993/05/30 15:27:22 ahd Exp $
 *
 *    Revision history:
 *    $Log: LIB.H $
 *     Revision 1.7  1993/05/30  15:27:22  ahd
 *     Drop PASSWD, SYSTEMS definitions
 *
 *     Revision 1.6  1993/05/30  00:11:03  ahd
 *     Drop free() macro
 *     Drop hardcoded PASSWD and SYSTEM file names
 *     Add RCSID macro
 *
 *     Revision 1.5  1993/04/04  21:51:00  ahd
 *     Update copyright
 *
 *     Revision 1.4  1993/03/06  23:09:50  ahd
 *     Break variable names out of master lib.h
 *
 *     Revision 1.3  1992/12/01  04:39:34  ahd
 *     Add SpeedOverMemory
 *
 * Revision 1.2  1992/11/22  21:31:22  ahd
 * Allow strpool() to allocate memory for const strings
 *
 * Revision 1.1  1992/11/16  05:00:26  ahd
 * Initial revision
 *
 */

#ifndef __LIB
#define __LIB

#include <confvars.h>

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

#define RCSID static const char UUFAR rcsid[] =

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
#define B_BATCH    0x00004000L // Used by news batching program - GMM
#define B_MAIL     (B_MUA | B_MTA | B_MUSH)
#define B_SPOOL    (B_MTA | B_NEWS | B_UUCICO | B_UUXQT | B_UUCP | B_UUSTAT)
#define B_ALL      (B_MAIL|B_SPOOL|B_NEWS|B_UUPOLL|B_UUSTAT|B_BATCH)

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
#define PATHS       "hostpath"
#define PERMISSIONS "permissn"
#define SYSLOG      "syslog"
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
#ifdef SAFEFREE
#define free(a)                  (safefree(a, cfnptr ,__LINE__))
#endif

#define nil(type)               ((type *)NULL)

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

#define msdos                 /* Required by dain bramaged compiler  */

#endif   /* ifndef __LIB */
