/*--------------------------------------------------------------------*/
/*       l i b . h                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1995 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *    $Id: lib.h 1.25 1994/12/27 20:50:28 ahd Exp $
 *
 *    Revision history:
 *    $Log: lib.h $
 *    Revision 1.25  1994/12/27 20:50:28  ahd
 *    Smoother call grading
 *
 *    Revision 1.24  1994/12/22 00:13:16  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.23  1994/11/27 17:34:39  ahd
 *    Ooops, B_GENERIC needs to be added to B_ALL to work.
 *
 *     Revision 1.22  1994/11/27  17:31:34  ahd
 *     Allow confguring for generic UUPC/extended utilities via B_GENERIC
 *
 *     Revision 1.21  1994/04/24  20:17:29  dmwatt
 *     Add Windows registry for configuration retrieval
 *
 *     Revision 1.20  1994/02/20  19:16:21  ahd
 *     IBM C/Set 2 Conversion, memory leak cleanup
 *
 *     Revision 1.19  1994/02/14  00:29:49  ahd
 *     Use quotes, not angle brackets, for local includes
 *
 *     Revision 1.18  1994/01/01  19:09:08  ahd
 *     Annual Copyright Update
 *
 *     Revision 1.17  1993/12/24  05:22:49  ahd
 *     Allow for more generic use of UUFAR type modifier
 *
 *     Revision 1.16  1993/11/06  17:57:46  rhg
 *     Drive Drew nuts by submitting cosmetic changes mixed in with bug fixes
 *
 *     Revision 1.15  1993/10/12  01:22:27  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.15  1993/10/12  01:22:27  ahd
 *     Normalize comments to PL/I style
 *
 *     Revision 1.14  1993/10/09  15:48:20  rhg
 *     ANSIfy the source
 *
 *     Revision 1.13  1993/09/29  04:56:11  ahd
 *     Revise B_KEWSHORT to B_SHORT
 *
 *     Revision 1.12  1993/09/20  04:51:31  ahd
 *     1.12OS/2 2.x support
 *
 *     Revision 1.11  1993/08/08  17:39:55  ahd
 *     Define denormalize() macro
 *
 *     Revision 1.10  1993/07/22  23:26:19  ahd
 *     First pass of changes for Robert Denny's Windows 3.1 support
 *
 *     Revision 1.9  1993/07/19  02:53:32  ahd
 *     Update copyright year'
 *
 *     Revision 1.8  1993/06/13  14:12:29  ahd
 *     Changes per Mike McLagan for outbound batched news support
 *
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

/*--------------------------------------------------------------------*/
/*     Environment variables set by UUXQT before invoking modules     */
/*--------------------------------------------------------------------*/

#define UU_USER "UU_USER"
#define UU_MACHINE "UU_MACHINE"
#define LOGNAME "LOGNAME"
#define RMAIL   "rmail"
#define RNEWS   "rnews"

/*--------------------------------------------------------------------*/
/*                   Global configuration variables                   */
/*--------------------------------------------------------------------*/

#define SYSRCSYM "UUPCSYSRC"
#define USRRCSYM  "UUPCUSRRC"
#define SYSDEBUG  "UUPCDEBUG"    /* Initialize debug level ahd        */

#if defined(WIN32)
#define REGISTRYHIVE  "Software\\Kendra Electronic Wonderworks\\UUPC/extended"
#endif

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
#define B_BATCH    0x00004000L /* Used by news batching program - GMM */
#define B_GENERIC  0x00008000L /* Generic utilties with no spec vars  */
#define B_MAIL     (B_MUA | B_MTA | B_MUSH)
#define B_SPOOL    (B_MTA | B_NEWS | B_UUCICO | B_UUXQT | B_UUCP | B_UUSTAT)
#define B_ALL      (B_MAIL | B_SPOOL | B_NEWS | B_UUPOLL | B_BATCH | B_GENERIC)

#define B_SHORT    0x80000000L /* Pointer is to short int, not string */
#define B_TOKEN    0x40000000L /* Pointer is one word, ignore blanks  */
#define B_BOOLEAN  0x20000000L /* Pointer is to boolean keywords      */
#define B_LIST     0x10000000L /* Pointer to array of char pointers   */
#define B_CLIST    0x08000000L /* Pointer to array of char pointers,
                                  using colon delimiter               */
#define B_CHAR     0x04000000L /* Single character                    */
#define B_STRING   0x02000000L /* String value (same as original UUPC
                                  configuration processor             */
#define B_NORMAL   0x01000000L /* Normalize backslashes to slashes in
                                  in this variable                    */
#define B_OBSOLETE 0x00800000L /* Option is obsolete, should be
                                  deleted                             */
#define B_MALLOC   0x00400000L  /* Use malloc(), not newstr()         */
#define B_LONG     0x00200000L  /* Pointer is to long, not string     */
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
#define checkref(a)              { if (!a) checkptr(cfnptr,__LINE__); }

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

extern void checkptr(const char *file,
                     const int line);

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

#ifdef UDEBUG
void dump_pool( void );          /* Dump our string pool          */
#endif

void safefree( void *input , const char *file, size_t line);

char *normalize( const char *path );

#define denormalize( path ) { char *xxp = path; \
   while ((xxp = strchr(xxp,'/')) != NULL)  \
      *xxp++ = '\\';  }

boolean IsDOS( void );

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

/*--------------------------------------------------------------------*/
/*     This triggers a "nice" break into the debugger under BC++      */
/*--------------------------------------------------------------------*/

#ifdef __TURBOC__
#define BREAKPOINT   _asm INT 3
#endif

#else
#error "lib.h" was already included once!
#endif   /* ifndef __LIB */
