/*--------------------------------------------------------------------*/
/*       l i b . h                                                    */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*       Changes Copyright (c) 1989-1999 by Kendra Electronic         */
/*       Wonderworks.                                                 */
/*                                                                    */
/*       All rights reserved except those explicitly granted by       */
/*       the UUPC/extended license agreement.                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                          RCS Information                           */
/*--------------------------------------------------------------------*/

/*
 *       $Id: lib.h 1.47 1998/11/24 13:50:22 ahd v1-13f ahd $
 *
 *       Revision history:
 *       $Log: lib.h $
 *       Revision 1.47  1998/11/24 13:50:22  ahd
 *       Use BREAKPOINT values for IBM C++, VC++ 5.0
 *
 *       Revision 1.46  1998/11/05 03:30:26  ahd
 *       Allow determining NT vs. 95 on the fly
 *
 *       Revision 1.45  1998/11/04 02:01:17  ahd
 *       Add sanity check field to linked lists
 *       Add assertion macros
 *
 *     Revision 1.44  1998/04/27  01:57:29  ahd
 *     Support for setting selected boolean options to enabled
 *
 *       Revision 1.43  1998/04/20 02:48:54  ahd
 *       Windows 32 bit GUI environment/TAPI support
 *
 *    Revision 1.42  1998/03/01 01:26:54  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.41  1997/12/15 03:54:46  ahd
 *    Only define RCSID() to be useful (and use memory) in 32 bit environments
 *
 *    Revision 1.40  1997/06/03 03:26:38  ahd
 *    First compiling SMTP daemon
 *
 *    Revision 1.39  1997/05/11 04:28:53  ahd
 *    SMTP client support for RMAIL/UUXQT
 *
 *    Revision 1.38  1997/03/31 07:00:15  ahd
 *    Annual Copyright Update
 *
 *    Revision 1.37  1996/11/19 00:25:20  ahd
 *    Correct C++ scoping
 *
 *    Revision 1.36  1996/03/18 03:48:14  ahd
 *    Allow compilation under C++ compilers
 *
 *    Revision 1.35  1996/01/07 14:18:18  ahd
 *    Provide external references to configuration functions and routines
 *    needed by regsetup.
 *
 *    Revision 1.34  1996/01/04 04:01:44  ahd
 *    Use sorted table for boolean options with binary search
 *
 *    Revision 1.33  1996/01/02 00:02:08  ahd
 *    Use sorted confifugration file tables with predefined sizes
 *    to allow binary search, rather than previous linear search.
 *
 *    Revision 1.32  1995/07/21 13:28:20  ahd
 *    Correct compiler warnings
 *
 *    Revision 1.31  1995/02/12 23:39:45  ahd
 *    compiler cleanup, NNS C/news support, optimize dir processing
 *
 *    Revision 1.30  1995/01/30 04:05:39  ahd
 *    Additional compiler warning fixes, optimize path normalizing
 *
 *    Revision 1.29  1995/01/29 14:09:13  ahd
 *    IBM C/Set++ warnings cleanup
 *
 *    Revision 1.28  1995/01/28 23:13:37  ahd
 *    Delete GNU C reference
 *
 *    Revision 1.27  1995/01/07 16:16:05  ahd
 *    Change KWBoolean to KWBoolean to avoid VC++ 2.0 conflict
 *
 *    Revision 1.26  1994/12/31 03:51:25  ahd
 *    First pass of integrating Mike McLagan's news SYS file suuport
 *
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
/*       Use the NOTCPIP to suppress the TCP/IP when you don't        */
/*       have WINSOCK.H                                               */
/*--------------------------------------------------------------------*/

#if defined(WIN32) || defined(_Windows) || defined(__OS2__)
#ifndef NOTCPIP
#define TCPIP
#endif
#endif
/*--------------------------------------------------------------------*/
/*                 Macro for recording when UUPC dies                 */
/*--------------------------------------------------------------------*/

#define panic()  bugout( cfnptr, __LINE__)


#ifdef UDEBUG
#define kwassert(condition) if (condition) \
   { printmsg(0,"Assertion failed:" #condition); panic(); }
#else
#define kwassert(condition)
#endif

/*--------------------------------------------------------------------*/
/*                 Macro for generic error messages from DOS          */
/*--------------------------------------------------------------------*/

#define printerr( x )  prterror( __LINE__, cfnptr, x)

/*--------------------------------------------------------------------*/
/*                  Convert hours/minutes to seconds                  */
/*--------------------------------------------------------------------*/

#define hhmm2sec(HHMM)    ((time_t)(((HHMM / 100) * 60L) + \
                           (time_t)(HHMM % 100)) * 60L)

#ifdef BIT32ENV
#define RCSID(x) static const char UUFAR _rcsId[] = x
#else
#define RCSID(x)
#endif

/*--------------------------------------------------------------------*/
/*                     Configuration file defines                     */
/*--------------------------------------------------------------------*/


#define B_EXPIRE   0x00000001L
#define B_GENERIC  0x00000002L /* Generic utilties with no spec vars  */
#define B_GENHIST  B_EXPIRE
#define B_INEWS    0x00000004L
#define B_INSTALL  0x00000008L /* Used by install program only        */
#define B_RMAIL    0x00000010L /* Used by Mail Delivery (RMAIL)       */
#define B_MUA      0x00000020L /* Used by Mail User Agent (MAIL)      */
#define B_MUSH     0x00000040L /* Used by MUSH - Not used by UUPC     */
#define B_NEWSRUN  0x00000080L
#define B_RNEWS    0x00000100L
#define B_SENDBATS 0x00000200L /* Used by news batching program - GMM */
#define B_UUCICO   0x00000400L /* Used by transport program UUCICO    */
#define B_UUCP     0x00000800L /* Used by UUCP command                */
#define B_UUPOLL   0x00001000L /* UUPOLL program                      */
#define B_UUSTAT   0x00002000L /* UUSTAT, UUSUB, UUNAME programs      */
#define B_UUNAME   B_UUSTAT
#define B_UUSUB    B_UUSTAT
#define B_UUXQT    0x00004000L /* Used by queue processor UUXQT       */
#define B_UUSMTPD  0x00008000L /* SMTP mail processor                 */

#define B_MTA      (B_UUSMTPD | B_RMAIL )
#define B_NEWS     (B_RNEWS | B_SENDBATS | B_NEWSRUN | B_EXPIRE | B_INEWS )
#define B_MAIL     (B_MUA | B_MTA | B_MUSH)
#define B_SPOOL    (B_MTA | B_UUCICO | B_UUXQT | B_UUCP | B_UUSTAT)
#define B_ALL      (B_MAIL | B_SPOOL | B_NEWS | B_UUPOLL | B_GENERIC | B_INSTALL)

/*--------------------------------------------------------------------*/
/*               Flags in configuration table flag word               */
/*--------------------------------------------------------------------*/

#define B_REQUIRED 0x00000001L /* Line must appear in configuration   */
#define B_FOUND    0x00000002L /* We found the token                  */

#define B_GLOBAL   0x00000004L /* Must not appear in PERSONAL.RC      */
#define B_LOCAL    0x00000008L /* The opposite of B_GLOBAL, sort of   */
#define B_SHORT    0x00000010L /* Pointer is to short int, not string */
#define B_TOKEN    0x00000020L /* Pointer is one word, ignore blanks  */
#define B_BOOLEAN  0x00000040L /* Pointer is to KWBoolean keywords     */
#define B_LIST     0x00000080L /* Pointer to array of char pointers   */
#define B_CLIST    0x00000100L /* Pointer to array of char pointers,
                                  using colon delimiter               */
#define B_CHAR     0x00000200L /* Single character                    */
#define B_STRING   0x00000400L /* String value (same as original UUPC
                                  configuration processor)            */
#define B_NORMAL   0x00000800L /* Normalize backslashes to slashes in
                                  in this variable                    */
#define B_OBSOLETE 0x00001000L /* Option is obsolete, should be
                                  deleted                             */
#define B_MALLOC   0x00002000L  /* Use malloc(), not newstr()         */
#define B_LONG     0x00004000L  /* Pointer is to long, not string     */
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

#define equal(a,b)               ((KWBoolean) !strcmp(a,b))
#define equali(a,b)              ((KWBoolean) !stricmp(a,b))
#define equalni(a,b,n)           ((KWBoolean) !strnicmp(a,b,n))
#define equaln(a,b,n)            ((KWBoolean) !strncmp(a,b,n))

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

typedef struct ConfigTable
   {
      char *sym;
      void *loc;
      CONFIGBITS program;
      CONFIGBITS flag;
   } CONFIGTABLE;

/*--------------------------------------------------------------------*/
/*                    Boolean options table layout                    */
/*--------------------------------------------------------------------*/

typedef struct FlagTable
   {
      char *sym;
      int position;
      CONFIGBITS bits;
      KWBoolean initial;
   } FLAGTABLE;

/*--------------------------------------------------------------------*/
/*                          Global variables                          */
/*--------------------------------------------------------------------*/

#ifdef __cplusplus
   extern "C" {
#endif

#ifdef SAFEFREE
void safefree( void *input , const char *file, size_t line);
#endif

extern int debuglevel;
extern FILE *logfile;
extern KWBoolean bflag[F_LAST];

extern FLAGTABLE configFlags[];
extern size_t configFlagsSize;

extern CONFIGTABLE rcTable[];
extern size_t rcTableSize;

/*--------------------------------------------------------------------*/
/*                        Function prototypes                         */
/*--------------------------------------------------------------------*/

#ifdef SAFEFREE
void safefree( void *input , const char *file, size_t line);
#endif

void prterror(const size_t lineno, const char *fname, const char *prefix);

extern void checkptr(const char *file,
                     const size_t line);

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
                    KWBoolean *xflag,
                    const size_t flagsSize );

int getargs(char *line,
            char **flds);                                   /* ahd */

void printmsg(int level, char *fmt, ...);

/*--------------------------------------------------------------------*/
/*                      Configuration functions                       */
/*--------------------------------------------------------------------*/

KWBoolean getrcnames(char **sysp,char **usrp);

KWBoolean configure( CONFIGBITS program );

KWBoolean getconfig(FILE *fp,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  const size_t TableSize,
                  FLAGTABLE *btable,
                  const size_t btableSize);

KWBoolean processconfig(char *buff,
                  SYSMODE sysmode,
                  CONFIGBITS program,
                  CONFIGTABLE *table,
                  const size_t TableSize,
                  FLAGTABLE *bTable,
                  const size_t bTableSize );

void
resetOptions(FLAGTABLE *flags,
             KWBoolean *barray,
             const size_t flagSize);

/*--------------------------------------------------------------------*/
/*                           Abort function                           */
/*--------------------------------------------------------------------*/

void bugout(  const char *fname, const size_t lineno);

/*--------------------------------------------------------------------*/
/*                Constant String allocation function                 */
/*--------------------------------------------------------------------*/

char *strpool( const char *input , const char *file, size_t line);

#ifdef UDEBUG
void dump_pool( void );          /* Dump our string pool          */
#endif


KWBoolean IsDOS( void );

#ifdef WIN32
KWBoolean isWinNT( void );
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

#if    defined(__IBMC__)
#define BREAKPOINT   { int *p = NULL; *(p) = 5; }
#elif defined(WIN32)
#define BREAKPOINT   DebugBreak();
#else
#define BREAKPOINT   _asm INT 3
#endif

#ifdef __cplusplus
   }
#endif

#else
#error "lib.h" was already included once!
#endif   /* ifndef __LIB */
