/*
 *	sysdefs.h - Configuration options header file.
 */

#define SID_H	"@(#)head:sysdefs.h	1.3	92/10/03 20:34:24 (woods)"
#define SID_NM	sysdefs_sccsid
#include <sccsid.h>

/*
 * This file defines a few things such as a base system environment
 * (BSD/SYSV), and includes some support for ANSI Standard C.  It is, in some
 * ways, analogous to the ANSI-C <stddef.h> header.  It also defines some
 * common system data types that may be missing on some platforms.  It is
 * intended to be included first, and to be followed at some point by
 * <libc.h>.
 */

/*
 * NOTE:  In general you will not have to edit this file.  Custom features
 * should be enabled via command-line defines if necessary (eg. -DDUMB_VOID).
 */

/*
 * WARNING:  This file uses '\' to continue '#if' lines
 */

/*
 * Determine if really ANSI C
 */
#if defined(__STDC__) && !defined(REALSTDC)
# if (__STDC__ - 0) > 0
#  define REALSTDC		1
#  ifndef USE_PROTOTYPES
#    define USE_PROTOTYPES	1	/* see use of this below */
#  endif
# else
#  define REALSTDC		0
# endif
#else
# define REALSTDC		0
#endif

/*
 * some compilers bitch about using 'extern' for "forward" references
 */
#if REALSTDC
# define forward	/* implicit */
#else
# define forward	extern
#endif
#ifndef private
# define private	static
#endif

/*
 * Some automated feature detection partially ripped out of JOVE 4.x
 */

#if defined(ultrix) || defined(__ultrix__)
# define ULTRIX
# ifndef BSD4_2
#  define BSD4_2	1	/* True enough for our purposes */
# endif
#endif

#if (defined(sun) || defined(__sun__)) /* && !defined(SUNOS5) */
# define SUN
# ifndef BSD4_2
#  define BSD4_2	1	/* True enough for our purposes */
# endif
#endif

#if defined(mips) || defined(__mips__)
# define MIPS
# ifndef SYSVR3
#  define SYSVR3	1
# endif
#endif

/* M_XENIX is defined by the Compiler */

/* SYSV should be defined for AT&T UNIX System V systems */

#ifdef SYSVR4
# ifndef SYSVR3
#  define SYSVR3	1	/* SYSVR3 is a subset of SYSVR4 */
# endif
#endif
#ifdef SYSVR3
# ifndef SYSVR2
#  define SYSVR2	1	/* SYSVR2 is a subset of SYSVR3 */
# endif
#endif
#ifdef SYSVR2
# ifndef SYSV
#  define SYSV		1	/* SYSV is a subset of SYSVR2 */
# endif
#endif
#ifdef SYSVR1
# ifndef SYSV
#  define SYSV		1	/* SYSVR1 is SYSV! */
# endif
#endif

#ifdef BSD4_1
# ifndef BSD
#  define BSD	1
# endif
#endif
#ifdef BSD4_2
# ifndef BSD
#  define BSD	1
# endif
#endif
#ifdef BSD4_3
# ifndef BSD
#  define BSD	1
# endif
#endif

#ifdef BSD2_8
# ifndef BSD
#  define BSD	1
# endif
#endif
#ifdef BSD2_9
# ifndef BSD
#  define BSD	1
# endif
#endif
#ifdef BSD2_10
# ifndef BSD
#  define BSD	1
# endif
#endif

#if defined(BSD) || defined(V7) || defined(SYSIII) /* || defined(M_XENIX) 2.3 */
# ifndef strchr
#  define strchr	index
# endif
# ifndef strrchr
#  define strrchr	rindex
# endif
#endif

#ifdef SYSV
# ifndef HAVE_UNAME
#  define HAVE_UNAME	1
# endif
#endif

#if !defined(SYSV) && !defined(M_XENIX) && \
	!defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
# define NEED_LOCKF	1
#endif

/*
 * Legend says the old Microsoft mc68k C compiler (for XENIX/68000, and
 * probably originally written by UniSoft) "noticed" if the code defined
 * 'VOID' and suddenly supported the 'void' keyword, in which case constructs
 * like the following may cause syntax errors or other weird behavior.
 * (Personally I thought it was M_VOID that triggered this behavior.)
 *
 * NOTE:  we don't use a typedef here.
 *
 * NOTE:  See 'UnivPtr' below too!
 */
#ifndef DUMB_VOID
# ifndef VOID
#  define VOID	void	/* 'VOID' is used where half-baked compilers fail */
# endif
#else
# undef VOID
# define VOID	int
# if defined(REDEF_VOID) && !defined(void)
#  define void	int	/* Even Ritchie's V7 cc supported 'void'.... */
# endif
#endif

/*
 * proto: macro to allow us to prototype any function declaration without
 * upsetting old compilers.  Use it like this:
 *
 *	extern int	myfun proto(int arg1, char *arg2);
 *	extern int	(*ptrtofun) ptrproto(void);
 *	extern char	*morefun proto((char *(*proc) ptrproto((const char *))));
 *	pattern = myproc("howdy", (int (*) ptrproto((int))) NULL, (char *) NULL);
 *
 * If you define this you'll likely need USE_STDARGS too.
 */
#if (REALSTDC == 1) || defined(USE_PROTOTYPES)
# define proto(x)		x
# ifdef	NO_PTRPROTO
   /* on these systems, a prototype cannot be used for a pointer to function */
#  define ptrproto(x)		()
# else
#  define ptrproto(x)		x
# endif
#else
# define proto(x)		()
# define ptrproto(x)		()
#endif

/*
 * There are two ways to handle functions with a variable number of args.
 * The old portable way uses <varargs.h>.  The way sanctioned by ANSI X3J11
 * uses <stdarg.h>.  Unfortunately they are not compatible....  Note also
 * that if you define this you'll probably need USE_PROTOTYPES too.
 */
#if (REALSTDC == 1) || defined(USE_STDARGS)
# ifndef USE_STDARGS
#  define USE_STDARGS		1
# endif
# define va_init(ap, parmN)	{ va_start((ap), (parmN)); }
#else
# define va_init(ap, parmN)	{ va_start((ap)); }
#endif

/*
 * 'const': readonly type qualifier (use may break some pre-ANSI compilers!)
 */
#if (REALSTDC == 0) && !defined(USE_CONST)
# define const		/* Only in ANSI C.  Pity */
#endif

/*
 * 'volatile':  type qualifier indicating one of two kinds of magic:
 *
 * 1. This object may be modified by an event unknown to the implementation
 *    (eg. asynchronous signal or memory-mapped I/O device).
 *
 * 2. This automatic variable might be modified between a setjmp()
 *    and a longjmp(), and we wish it to have the correct value after
 *    the longjmp().  This second meaning is an X3J11 abomination.
 */
#if (REALSTDC == 0) && !defined(USE_VOLATILE)
# define volatile	/* also only in ANSI C.  Oh well. */
#endif

/*
 * 'signed': converse of 'unsigned'
 */
#if (REALSTDC == 0) && !defined(USE_SIGNED)
# define signed		/* bye-bye! */
#endif

/*
 * UnivPtr: universal pointer type
 */
#if (REALSTDC == 1) || defined(USE_VOIDPTR)
typedef void		Univ;
typedef void		*UnivPtr;
typedef const void	*UnivConstPtr;	/* object is "constant" */
typedef void * const	UnivPtrConst;	/* pointer is "constant" */
#else
typedef char		Univ;
typedef char		*UnivPtr;
typedef const char	*UnivConstPtr;	/* object is "constant" */
typedef char * const	UnivPtrConst;	/* pointer is "constant" */
#endif

/*
 * According to the ANSI standard for C, any library routine may
 * be defined as a macro with parameters.  In order to prevent
 * the expansion of this macro in a declaration of the routine,
 * ANSI suggests parenthesizing the identifier.  This is a reasonable
 * and legal approach, even for K&R C.
 *
 * A bug in the MIPS compiler used on MIPS, IRIS, and probably other
 * MIPS R[23]000 based systems, causes the compiler to reject
 * these declarations (at least at the current time, 1989 August).
 * To avoid this bug, we conditionally define and use UNMACRO.
 */
#if defined(MIPS) /* || defined(IRIS) || defined(NEC) */
# define UNMACRO(proc)	proc
#else
# define UNMACRO(proc)	(proc)
#endif

/*
 * This is a bit tricky.  Often, when compiling on a POSIX compatible system,
 * one will *not* want to define _POSIX_SOURCE.  However, not doing so does
 * not remove the typedefs from the system headers.  Perhaps the opposite
 * logic should be used, and these should be defined *only* for those system
 * variants that need them....  The problem being that some compilers get
 * right upset if you try and re-typedef something, even if to the exact same
 * fundamental type.  NOTE:  some are ushort, some ulong or long.
 */
#if !defined(SYSVR4) && !defined(_AIX) && \
	!defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
# ifdef V7
typedef int		uid_t;
typedef int		gid_t;
# else
#  ifdef M_XENIX
typedef unsigned short	uid_t;
typedef unsigned short	gid_t;
#  else
/* typedef unsigned int	uid_t;
typedef unsigned int	gid_t; */
#  endif
# endif
#endif

/*
 * size_t is required for at least for att-3b1 (defined(u3b)) and most M_XENIX
 */
#if !defined(_SIZE_T) && defined(NEED_SIZE_T) || ((REALSTDC == 0) && \
	!defined(SYSVR2) && !defined(SYSVR3) && !defined(SYSVR4) && \
	!defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE))
# define _SIZE_T	1		/* _AIX only? */
typedef unsigned int	size_t;		/* int on some, ulong on some? */
#endif

/*
 * 8-bit type & 16-bit macros to avoid sign problems
 */
#ifndef NO_BYTE
# if defined(V7) || defined(M_XENIX) || defined(BSD2_8)
typedef char		byte;	/* WARNING: machine dependent! */
#  define BYTE(x)	((x) && 0xff)
# else
typedef unsigned char	byte;	/* WARNING: machine dependent! */
#  define BYTE(x)	(x)
# endif
# define HIBYTE(x)	(((x) >> 8) & 0xff)	/* hi byte of short */
# define LOBYTE(x)	((x) & 0xff)		/* lo byte of short */
# define HIWORD(x)	(((x) >> 16) & 0xffffL)	/* hi 16-bits of long */
# define LOWORD(x)	((x) & 0xffffL)		/* lo 16-bits of long */
# define CHAR(x)	((x) & 0x7f)
# define WORD(x)	((x) & 0xffffL)		/* i.e. 16-bit word */
#endif

/*
 * more ANSI-C types more properly found in <stddef.h>
 */
#if !defined(_WCHAR_T) && (defined(NEED_WCHAR_T) || (!defined(REALSTDC)))
# define _WCHAR_T	1		/* _AIX only? */
typedef unsigned short	wchar_t;
#endif
#if !defined(_PTRDIFF_T) && (defined(NEED_PTRDIFF_T) || (!defined(REALSTDC)))
# define _PTRDIFF_T	1		/* _AIX only? */
typedef long		ptrdiff_t;
#endif

/*
 * as Henry Spencer says, 'char *sprintf()' is stupid and archaic!
 */
#if !defined(BSD) && (REALSTDC == 0) && !defined(NEED_VSPRINTF) && \
	!defined(_POSIX_SOURCE) && !defined(_XOPEN_SOURCE)
typedef int	SPRINTF_T;
#else
typedef char	*SPRINTF_T;	/* WARNING: don't use this with vsprintf.shar */
#endif

/*
 * Offset of member MEMBER in a struct of type TYPE.
 *
 * WARNING:  compiler dependent!
 */
#if !defined(offsetof) && !defined(_AIX) && !defined(_ANSI_C_SOURCE)
# define offsetof(TYPE, MEMBER)		((size_t) &(((TYPE *)0)->MEMBER))
#endif
