/*
 *	libc.h - support for some older libc's
 */

#define SID_H	"@(#)head:libc.h	1.5	92/10/09 23:40:41 (woods)"
#define SID_NM	libc_sccsid
#include <sccsid.h>

/*
 * DESCRIPTION:  This file defines several things that are expected to be a
 * part of the compile-time environment for a normal UNIX program.  In
 * particular, this header attempts to bend a non-POSIX (1003.1) environment
 * into some semblance of conformance.  By no means am I advocating complete
 * compliance of POSIX is a good thing -- rather it is a necessary evil!
 */
/*
 * USAGE:  If your system is POSIX compliant, this header assumes that
 * _POSIX_SOURCE will be defined.
 */
/*
 * NOTE:  This file must always included *AFTER* <stdio.h>, <unistd.h>,
 * <sys/stat.h>, and/or <limits.h>, if you're using either, and in fact you
 * might want to include it after *all* other includes.
 */
/*
 * ALSO NOTE: This file should probably be preceded by "sysdefs.h"
 */
/*
 * HISTORY:  This header is modeled after one of the same name found in the C
 * News sources written at the University of Toronto by Geoff Collyer and
 * Henry Spencer.  Greg A. Woods <woods@robohack.UUCP> has adapted it to be
 * more complete and useful as a stand-alone header.  Some of the following
 * bits have been derived from headers in many other tools, including Jove.
 */

#ifndef NULL
# define NULL	0
#endif
#ifndef	EOF
# define EOF	(-1)
#endif
#ifndef EOS
# define EOS	'\0'
#endif

/*
 * Yes, it's ugly to define the following things in here, but the alternative
 * is to have several copies of each clause spread around many files.
 */

/*
 * this _should_ be in <limits.h>, but SYSVR2 & BSD don't have one
 */
#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX	MAXPATHLEN
# else
#  define PATH_MAX	1024
# endif
#endif
#ifndef NAME_MAX
# ifdef MAXNAMELEN
#  define NAME_MAX	MAXNAMELEN
# else
#  define NAME_MAX	14
# endif
#endif

/*
 * Symbolic constants for the "lseek" routine (should be in <unistd.h>).
 * Don't bother with the old BSD "L*" things.
 */
#ifndef SEEK_SET
# define SEEK_SET	0	/* Set file pointer to "offset" */
# define SEEK_CUR	1	/* Set file pointer to current plus "offset" */
# define SEEK_END	2	/* Set file pointer to EOF plus "offset" */
#endif

/*
 * these _should_ be in <sys/stat.h>!
 */
#ifndef S_IRWXU
# define S_IRWXU	00700		/* read, write, execute: owner */
# define S_IRWXG	00070		/* read, write, execute: group */
# define S_IRWXO	00007		/* read, write, execute: other */
#endif
#ifndef S_IRUSR
# define S_IRUSR	00400		/* read permission: owner */
# define S_IWUSR	00200		/* write permission: owner */
# define S_IXUSR	00100		/* execute permission: owner */
# define S_IRGRP	00040		/* read permission: group */
# define S_IWGRP	00020		/* write permission: group */
# define S_IROTH	00004		/* read permission: other */
# define S_IWOTH	00002		/* write permission: other */
# define S_IXOTH	00001		/* execute permission: other */
#endif

/*
 * signal types: tailor to suit local needs (usually 'VOID' is OK, as it will
 * be either 'void' or 'int', and most systems that support 'void' will have
 * "void (*signal)();", and 'VOID' will be 'int' otherwise.)
 */
typedef VOID (*sigret_t)();
typedef VOID (*sigarg_t)();

/*
 * The SCO UNIX, Solaris 2.0, and probably others have getopt(3c)'s that are not
 * POSIX compliant.  These should be in <unistd.h>!
 */
#if !defined(NO_GETOPT)		/* might force in getopt(3c) */
extern int		optind;
extern char		*optarg;
#endif

#if !REALSTDC && \
	!defined(_POSIX_SOURCE) && !defined(XOPEN_SOURCE)

/*
 * these _should_ be in other system header files....  <stdio.h>, <stdlib.h>,
 * <unistd.h>, etc.
 *
 * NOTE:  There are some in here you may not have.
 *
 * WARNING:  These are the "standard" declarations, and some "broken" systems
 * may define things a bit differently.  Hopefully a re-declaration clash
 * will make this obvious!
 */

extern int		errno;		/* always(?) in <errno.h> */

#ifndef UUPC
extern SPRINTF_T	sprintf();
#endif

extern sigret_t		signal();

extern void		exit();
extern void		_exit();

extern void		abort();
extern void		_assert();
extern void		longjmp();

extern char		*crypt();
extern void		setkey();
extern void		encrypt();

extern void		monitor();
extern void		profil();

#ifndef M_UNIX	/* on SCO, it's a macro! */
extern char		*strerror();
#endif /* M_UNIX */

extern unsigned int	sleep();

#ifndef UUPC
extern time_t		time();
extern struct tm	*gmtime();
extern char		*ctime();
#endif

extern struct passwd	*getpwent();
extern struct passwd	*getpwuid();
extern struct passwd	*getpwnam();
extern void		setpwent();
extern void		endpwent();
extern struct passwd	*fgetpwent();

extern struct group	*getgrent();
extern struct group	*getgrgid();
extern struct group	*getgrnam();
extern void		setgrent();
extern void		endgrent();
extern struct group	*fgetgrent();

#ifndef UUPC
extern uid_t		getuid();
extern uid_t		geteuid();
extern gid_t		getgid();
extern gid_t		getegid();
#endif

extern FILE		*fopen();
extern FILE		*freopen();
extern FILE		*popen();
extern FILE		*tmpfile();
extern char		*mktemp();
extern char		*tempnam();
extern char		*fgets();
extern char		*gets();
extern long		ftell();
extern void		setbuf();
#if 0
extern void		clearerr();	/* a macro in most <stdio.h>! */
#endif
extern void		rewind();
extern void		perror();

extern char		*getenv();
extern char		*cuserid();

#ifndef UUPC
#ifndef M_UNIX	/* SCO's compiler (i.e. Microsoft's) barfs on some of these */
extern char		*strcpy();
extern char		*strncpy();
extern char		*strcat();
extern char		*strncat();
extern char		*strchr();
extern char		*strrchr();
extern char		*strpbrk();
extern char		*strtok();
extern char		*strdup();
extern char		*strstr();
#endif /* M_UNIX */
extern int		strcmp();
extern int		strncmp();
extern int		strlen();
extern int		strspn();
extern int		strcspn();

#ifndef M_UNIX	/* more SCO inconsitency */
extern UnivPtr		memccpy();
extern UnivPtr		memchr();
extern UnivPtr		memcpy();
extern UnivPtr		memmove();
extern UnivPtr		memset();
#endif /* M_UNIX */
extern int		memcmp();
#endif

extern UnivPtr		bsearch();
extern void		qsort();
extern void		twalk();

extern void		srand();

extern void		swab();

#ifndef UUPC
#ifndef USE_MALLOC
extern void		free();
extern UnivPtr		calloc();
extern UnivPtr		malloc();
extern UnivPtr		realloc();
#endif /* USE_MALLOC */
#endif

extern double		atof();
extern long		atol();
extern long		labs();

extern double		strtod();
extern long		strtol();
# if 0
extern unsigned long	strtoul();	/* 'unsigned long' not portable */
# endif

#endif

/*
 * The following prototypes are esp. useful since they are stdargs-like.
 * If they clash, please let me know!
 */
#if !REALSTDC && defined(USE_PROTOTYPES)
extern int	printf(const char *fmt, ...);
extern int	fprintf(FILE *fp, const char *fmt, ...);
extern int	sprintf(char *buf, const char *fmt, ...);
extern int	scanf(const char *fmt, ...);
extern int	fscanf(FILE *fp, const char *fmt, ...);
extern int	sscanf(char *s, const char *fmt, ...);
#endif
