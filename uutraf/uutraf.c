/*
 *	uutraf.c - generate UUCP traffic reports and summaries
 */

#define SID	"@(#)uutraf:uutraf.c	1.2.2.8	92/10/23 10:23:03 (woods)"
#include <sccsid.h>

#if defined(USE_STDDEF) || defined(_POSIX_SOURCE) || \
	REALSTDC || (__STDC__ - 0) == 1
# include <stddef.h>	/* should do before sysdefs.h */
#endif
#include <sysdefs.h>	/* a local header for common portability */
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifdef BSD		/* really only pre-4.4BSD? */
# include <strings.h>
#else
# include <string.h>
# if !REALSTDC || defined(USE_MALLOC)
#  include <malloc.h>
# endif
# ifndef SYSVR2
#  include <limits.h>
# endif
#ifndef UUPC
# include <unistd.h>
#endif
#endif
#if REALSTDC || defined(USE_STDLIB)
# include <stdlib.h>
#endif
#include <str.h>	/* local header:  STR*() macros */
#include <libc.h>	/* local header:  for older libc's */
#include <dlst.h>	/* local header:  dynamic list library */
#include "defs.h"	/* uutraf's customization */
#include "uutraf.h"	/* uutraf's common header */
#include <extern.h>	/* local header:  setup for external declarations */
#include "version.h"	/* uutraf's version stuff */

#ifdef UUPC
#include "lib.h"
#include "getopt.h"
currentfile();
#endif

#ifndef lint
static const char *sccsold = "@(#) original uutraf.c : rel 1.2 : mod 2/2/90";
#endif

/*
 * History:
 *	woods@robohack.UUCP - Mon Oct 12 15:11:03 EDT 1992
 *	- added version and help message
 *
 *	rstory@shaboom.UUCP - Sat Sep 19 09:25:36 EDT 1992
 *	- added reporting of date range time stamping
 *
 *	woods@robohack.UUCP - Thu Aug 27 00:52:14 EDT 1992
 *	- finally an ULTRIX SYSLOG to work with!
 *
 *	woods@robohack.UUCP - Tue Aug 25 22:07:24 EDT 1992
 *	- changed some field widths to allow > CPS values
 *
 *	woods@robohack.UUCP - Sun Feb 16 14:30:38 EST 1992
 *	- added new report style for systems by port...
 *
 *	woods@eci386.UUCP - Mon Jul 16 10:46:06 EDT 1990
 *	- fixed compare math
 *
 *	woods@robohack.UUCP - Sun Jul 15 18:11:48 EDT 1990
 *	- major re-write
 *	- added list library modules
 *	- added print options (and name sort option)
 *
 * @(#) PROJ:		uutraf
 * @(#) FILE:		uutraf.c
 * @(#) Release:	1.2
 * @(#) Rel. Date:	2/2/90
 * @(#) Author:		Greg Hackney <hack@texbell.swbt.com>
 */

forward LINK	*getsys();
forward PORT	*getdev();
forward void	procfile(),
		tsfill(),
		zerosys(),
		zerodev(),
		sortrun(),
		printrun(),
		printsys(),
		printdev(),
		printsum(),
		printtmrng(),
		comma_fy(),
		sortlst(),
		usage_err();
forward cmp_t	cmpdev(),
		cmpsys();

char		*argv0 = NULL;

char		*statfile = STATFILE;

/*
 * globals for compare routines
 */
unsigned short	sort_sys_f = 0,
		sort_dev_f = 0;

#ifdef DEBUG
int	debug = 0;
#endif

/* ARGSUSED */
int
main(argc, argv, envp)
	int	argc;
	char	*argv[];
	char	*envp[];
{
	unsigned short	sortopt,	/* default sort method, see defs.h */
			sortnames,
			printopt;
	static SUMM	trec = {0};	/* summary record */
	llst_t		*syslst,	/* linked list of per-system totals */
			*devlst;	/* linked list of per-device totals */
	int		c;
	FILE		*logfile = NULL;

#ifdef UUPC
	if (!configure( B_UUCICO ))
	  panic();
	statfile = malloc(512);
	mkfilename(statfile, E_spooldir, SYSLOG);
#endif

	sortnames = 0;
	sortopt = 0;
	printopt = 0;
	devlst = syslst = (llst_t *) NULL;

	argv0 = (argv0 = strrchr(argv[0], '/')) ? argv0 + 1 : argv[0];

	while ((c = getopt(argc, argv, "HMNPSVab:c:f:hnt:sx:?")) != EOF) {
		switch (c) {
		case 'H':
			printopt |= HEADERS;
			break;
		case 'N':
			printopt |= SYS_RPT;
			break;
		case 'S':
			printopt |= SUMM_RPT;
			break;
#ifdef HAVE_HDBUUCP
		case 'P':
			printopt |= PORT_RPT;
			break;
		case 'M':
			printopt |= (SYSBYPORT_OPT | SYS_RPT);
			break;
#endif
		case 'a':	/* sort in ascending vs. descending order */
			sortopt |= REVERSE;
			break;
		case 'b':	/* sort by bytes xmitted and/or received */
			if (*(optarg + 1) != EOS)
				usage_err(argv0, "-b requires a one-letter argument");
			switch (*optarg) {
			case 'b':
				sortopt |= (RECEIVED | XMIT);
				break;
			case 'r':
				sortopt |= RECEIVED;
				break;
			case 'x':
				sortopt |= XMIT;
				break;
			default:
				usage_err(argv0, "-b's argument must be one of [brx]");
				break;
			}
			break;
		case 'c':	/* sort by xfer rate xmitted and/or received */
			if (*(optarg + 1) != EOS)
				usage_err(argv0, "-c requires a one-letter argument");
			switch (*optarg) {
			case 'b':
				sortopt |= (R_CPS | X_CPS);
				break;
			case 'r':
				sortopt |= R_CPS;
				break;
			case 'x':
				sortopt |= X_CPS;
				break;
			default:
				usage_err(argv0, "-c's argument must be one of [brx]");
				break;
			}
			break;
		case 'n':
			sortnames = 1;
			break;
		case 'f':	/* number of files */
			if (*(optarg + 1) != EOS)
				usage_err(argv0, "-f requires a one-letter argument");
			switch (*optarg) {
			case 'b':
				sortopt |= (R_NUMB | X_NUMB);
				break;
			case 'r':
				sortopt |= R_NUMB;
				break;
			case 'x':
				sortopt |= X_NUMB;
				break;
			default:
				usage_err(argv0, "-f's argument must be one of [brx]");
				break;
			}
			break;
		case 't':	/* sort by accumulated time */
			if (*(optarg + 1) != EOS)
				usage_err(argv0, "-t requires a one-letter argument");
			switch (*optarg) {
			case 'b':
				sortopt |= (R_TIME | X_TIME);
				break;
			case 'r':
				sortopt |= R_TIME;
				break;
			case 'x':
				sortopt |= X_TIME;
				break;
			default:
				usage_err(argv0, "-t's argument must be one of [brx]");
				break;
			}
			break;
		case 's':	/* Use stdin instead of normal xferstat file */
			logfile = stdin;
			break;
		case 'x':
#ifdef DEBUG
			debug = atoi(optarg);
			break;
#else
			(void) fprintf(stderr, "%s: debugging not compiled in.\n", argv0);
			break;
#endif
		case 'V':
			(void) fprintf(stderr, "%s: Version %d.%d, Patchlevel %d.\n", argv0, version, subver, patchlevel);
			break;
		default:
			(void) fprintf(stderr, "\n%s: Version %d.%d, Patchlevel %d.\n", argv0, version, subver, patchlevel);
			usage_err(argv0, usage);
			break;
		}
	}
	/*
	 * Use default sorting method if none or names was chosen
	 */
	if (((sortopt == REVERSE) || !sortopt) && !sortnames)
		sortopt |= DEFAULT_SORT;
	if ((printopt == HEADERS) || !printopt)
		printopt |= DEFAULT_RPT;
	if (optind == argc || logfile)			/* fake up the default file */
		argv[--optind] = statfile;
	for ( ; optind < argc; optind++) {
		if (STREQ(argv[optind], "-")) {
#ifdef DEBUG
			if (debug)
				(void) fprintf(stderr, "%s: processing STDIN.\n", argv0);
#endif
			logfile = stdin;
		} else {
			if (! (logfile = fopen(argv[optind], "r"))) {
				(void) fprintf(stderr, "%s: Can't open %s for reading -- ", argv0, argv[optind]);
				perror(optarg);
			}
#ifdef DEBUG
			if (debug)
				(void) fprintf(stderr, "%s: processing %s.\n", argv0, argv[optind]);
#endif
		}
		procfile(logfile, &syslst, &devlst, &trec);
		(void) fclose(logfile);		/* will possibly close stdin */
	}
	trec.syscnt = llst_total(syslst);
#ifdef HAVE_HDBUUCP
	trec.devcnt = llst_total(devlst);
#endif
	sortrun(syslst, devlst, sortopt, sortopt);	/* sort it */
	printrun(syslst, devlst, &trec, printopt);	/* print it out */
	exit(0);
	/* NOTREACHED */
}

void
procfile(logfile, syslstp, devlstp, trec)
	FILE	*logfile;
	llst_t	**syslstp;
	llst_t	**devlstp;
	SUMM	*trec;
{
	LINK	*sysp;
	PORT	*devp;
	PORT	*portp;
	char	*sys,		/* points at the previous remote node name */
		*user,		/* points at the "user-id" */
		flow[10],	/* holds the direction of uucp flow */
		buf[BUFSIZ],	/* general purpose */
		*string;	/* general purpose */
	long	bytes;		/* number of bytes in the uucp xfer */
	float	seconds;	/* number of seconds the xfer took */
	int	mon, day, hh, mm;
#ifdef HAVE_HDBUUCP
	char	*dev;		/* points at the device name used */
	char	mode, pchar = ' ';
	int	ss, pid = 0, seq = 0;
#else
	long	epoch;
#endif
#ifdef DEBUG
	int	i = 0;
#endif

	/*
	 * process each line in the logfile
	 */
#ifdef HAVE_HDBUUCP
	/*	From an AT&T 3b2/400 SysVr3.2 /usr/spool/uucp/.Admin/xferstats:
	 * fastlane!woods M (2/16-19:43:37) (C,5711,2) [tty33] -> 152 / 1.420 secs, 107 bytes/sec
	 * fastlane!rhl M (2/16-19:43:50) (C,5711,3) [tty33] <- 471 / 6.090 secs, 77 bytes/sec
	 *
	 *	From an AIX 3.2 /var/uucp/.Admin/xfererstats:
	 * ecirs6k!uucp M (2/17-19:44:54) (C,8560,0) [notty] <- 365531 / 7.633 secs, 47888 bytes/sec 
	 * ecirs6k!uucp M (2/17-22:10:19) (C,321,0) [notty] <- 32178 / 0.366 secs, 87918 bytes/sec 
	 */
#else
	/*	From an ULTRIX 4.2 /var/spool/uucp/SYSLOG:
	 * daemon becker (8/26-0:14) (714802480) sent 348 b 0 secs, Pk: 7, Rxmt: 0
	 * daemon becker (8/26-0:14) (714802487) sent 89 b 0 secs, Pk: 3, Rxmt: 0
	 * news censor (8/25-6:00) (714736824) received 43842 b 47 secs
	 * news censor (8/25-6:00) (714736827) received 108 b 1 secs
	 */
#endif
	while (fgets(buf, BUFSIZ, logfile)) {
#ifdef DEBUG
		if (debug > 8)
			(void) fprintf(stderr, "%d: %s", i, buf);
#endif
		user = buf;
		/*
		 * look for the node name
		 */
#ifdef HAVE_HDBUUCP
		if (!(sys = strtok(buf, "!")))
#else
		if (!(sys = strtok(buf, " ")) ||
		    !(sys = strtok((char *) NULL, " ")))
#endif
			continue;
#ifdef DEBUG
		if (debug >  7)
			(void) fprintf(stderr, "System: %s, ", sys);
#endif
		/*
		 * look for the junk information
		 */
#ifdef HAVE_HDBUUCP
		if (!(string = strtok((char *) NULL, "[")))
			continue;
#ifdef UUPC
		if (sscanf(string, "%*s %c %*s (%d/%d-%d:%d:%d) (%c,%d,%d)",
#else
		if (sscanf(string, "%*s %c (%d/%d-%d:%d:%d) (%c,%d,%d)",
#endif
			   &mode, &mon, &day, &hh, &mm, &ss, &pchar, &pid, &seq) == EOF)
			continue;
# ifdef DEBUG
		if (debug > 6)
			(void) fprintf(stderr, "mode: %c, time&pid: (%d/%d-%d:%d:%d) (%c,%d,%d)\n",
				       mode, mon, day, hh, mm, ss, pchar, pid, seq);
# endif
#else
		if (!(string = strtok((char *) NULL, " ")))
			continue;
# ifdef DEBUG
		if (debug > 8)
			(void) fprintf(stderr, "date&time ");
# endif
		if (sscanf(string, "(%d/%d-%d:%d)", &mon, &day, &hh, &mm) == EOF)
			continue;
# ifdef DEBUG
		if (debug > 6)
			(void) fprintf(stderr, "(%d/%d-%d:%d)", mon, day, hh, mm);
# endif
		if (!(string = strtok((char *) NULL, " ")))
			continue;
# ifdef DEBUG
		if (debug > 8)
			(void) fprintf(stderr, " epoch");
# endif
		if (sscanf(string, "(%ld)", &epoch) == EOF)
			continue;
# ifdef DEBUG
		if (debug > 6)
			(void) fprintf(stderr, " (%ld)\n", mon, day, hh, mm, epoch);
# endif
#endif
                tsfill(trec, mon, day, hh, mm);	/* should be inlined for efficiency */
#ifdef HAVE_HDBUUCP
		/*
		 * look for the device name
		 */
		if (!(dev = strtok((char *) NULL, "]")))
			continue;
#endif
		/*
		 * parse the remainder of the data string
		 */
		if (!(string = strtok((char *) NULL, "\n")))
			continue;
#ifdef HAVE_HDBUUCP
		if (sscanf(string, "%s %ld / %f", flow, &bytes, &seconds) == EOF)
#else
# if defined(ultrix) || defined(__ultrix__)	/* ???? */
		if (sscanf(string, "%s %ld b %f secs", flow, &bytes, &seconds) == EOF)
# else
		if (sscanf(string, "%s data %ld bytes %f secs", flow, &bytes, &seconds) == EOF)
# endif
#endif
			continue;
		if (!(sysp = getsys(syslstp, sys)))
			exit(1);
#ifdef HAVE_HDBUUCP
		if (!(devp = getdev(devlstp, dev)))
			exit(1);
		if (sysp->l_lastpid != pid) {
			sysp->l_lastpid = pid;
			if (mode == 'M')
				sysp->l_out_conn++;
			else
				sysp->l_in_conn++;
		}
		if (devp->p_lastpid != pid) {
			devp->p_lastpid = pid;
			if (mode == 'M') 
				devp->p_out_conn++;
			else
				devp->p_in_conn++;
		}
#endif
		/* outgoing uucp */
		if (STREQ(flow, OUT)
#ifdef pyr
		     || STREQ(flow, T_OUT) || STREQ(flow, F_OUT)
#endif
		   ) {
#ifdef DEBUG
			if (debug > 5)
# ifdef HAVE_HDBUUCP
				(void) fprintf(stderr, "%s OUTGOING %s\n", sys, dev);
# else
				(void) fprintf(stderr, "%s OUTGOING\n", sys);
# endif
#endif
			sysp->l_xmit += bytes;
			sysp->l_xmit_time += seconds;
			sysp->l_xmit_cnt++;
#ifdef HAVE_HDBUUCP
			devp->p_xmit += bytes;
			devp->p_xmit_time += seconds;
			devp->p_xmit_cnt++;
			if (!(portp = getdev(&(sysp->l_portlst), dev)))
				exit(1);
			portp->p_xmit += bytes;
			portp->p_xmit_time += seconds;
			portp->p_xmit_cnt++;
			if (portp->p_lastpid != pid) {
				portp->p_lastpid = pid;
				if (mode == 'M') {
					portp->p_out_conn++;
					trec->num_out_conn++;	/* only once! */
				} else {
					portp->p_in_conn++;
					trec->num_in_conn++;
				}
			}
#endif
			trec->bytes_xmit += bytes;	/* summary totals */
			trec->sec_xmit += seconds;
			trec->num_xmit++;
		} else if (STREQ(flow, IN)		/* incoming uucp */

#ifdef pyr
			   || STREQ(flow, T_IN) || STREQ(flow, F_IN)
#endif
			  ) {
#ifdef DEBUG
			if (debug > 5)
# ifdef HAVE_HDBUUC
				(void) fprintf(stderr, "%s INCOMING %s\n", sys, dev);
# else
				(void) fprintf(stderr, "%s INCOMING\n", sys);
# endif
#endif
			sysp->l_recv += bytes;
			sysp->l_recv_time += seconds;
			sysp->l_recv_cnt++;
#ifdef HAVE_HDBUUCP
			devp->p_recv += bytes;
			devp->p_recv_time += seconds;
			devp->p_recv_cnt++;
			if (!(portp = getdev(&(sysp->l_portlst), dev)))
				exit(1);
			portp->p_recv += bytes;
			portp->p_recv_time += seconds;
			portp->p_recv_cnt++;
			if (portp->p_lastpid != pid) {
				portp->p_lastpid = pid;
				if (mode == 'M') {
					portp->p_out_conn++;
					trec->num_out_conn++;
				} else {
					portp->p_in_conn++;
					trec->num_in_conn++;
				}
			}
#endif
			trec->bytes_recv += bytes;	/* summary totals */
			trec->sec_recv += seconds;
			trec->num_recv++;
		}
#ifdef DEBUG
		i++;
#endif
	}
	return;
}

#ifdef HAVE_HDBUUCP
PORT *
getdev(devlstp, d)	/* returns an link struct to the system name specified. */
	llst_t	**devlstp;
	char	*d;
{
	llst_t	*work;
	PORT	*nd;
	PORT	sd;

	extern unsigned short	sort_dev_f;

	sort_dev_f = 0;		/* set default comparison of name only */
	work = *devlstp;
	if (! work) {		/* must be the first time thru */
#ifdef DEBUG
		if (debug > 2)
			(void) fprintf(stderr, "initialising devlst (%s)\n", d);
#endif
		if (!(work = llst_new())) {
			perror("getdev(): llst_new() failed");
			return(NULL);
		}
		if (!(nd = (PORT *) malloc(sizeof(PORT)))) {
			perror("getdev(): malloc(nd) failed");
			llst_destroy(work);
			return(NULL);
		}
		if (!llst_add(work, (char *) nd)) {
			perror("getdev(): llst_add(nd) failed");
			llst_destroy(work);
			free(nd);
			return(NULL);
		}
		*devlstp = work;
		zerodev(nd);
		(void) strncpy(nd->p_dname, d, sizeof(nd->p_dname) - 1);
		return(nd);
	}
	zerodev(&sd);
	(void) strncpy(sd.p_dname, d, sizeof(sd.p_dname) - 1);
	if (!llst_find(work, (char *) &sd, cmpdev)) {
		/*
		 * create a new link
		 */
#ifdef DEBUG
		if (debug > 1)
			(void) fprintf(stderr, "adding new device - %s\n", d);
#endif
		if (!(nd = (PORT *) malloc(sizeof(PORT)))) {
			perror("getdev(): malloc(nd2) failed");
			return(NULL);
		}
		if (!llst_add(work, (char *) nd)) {
			perror("getdev(): llst_add(nd2) failed");
			free(nd);
			return(NULL);
		}
		zerodev(nd);
		(void) strncpy(nd->p_dname, d, sizeof(nd->p_dname) - 1);
	}
	return((PORT *) llst_current(work));
}
#endif

LINK *
getsys(syslstp, s)	/* returns an link struct to the system name specified. */
	llst_t	**syslstp;
	char	*s;
{
	llst_t	*work;
	LINK	*nd;
	char	buf[BUFSIZ];
	LINK	*sd = (LINK *) buf;

	extern unsigned short	sort_sys_f;

	sort_sys_f = 0;		/* set default comparison of name only */
	work = *syslstp;
	if (! work) {		/* must be the first time thru */
#ifdef DEBUG
		if (debug > 2)
			(void) fprintf(stderr, "initialising syslst (%s)\n", s);
#endif
		if (!(work = llst_new())) {
			perror("getsys(): llst_new() failed");
			return(NULL);
		}
		if (!(nd = (LINK *) malloc(sizeof(LINK) + strlen(s)))) {
			perror("getsys(): malloc() failed");
			llst_destroy(work);
			return(NULL);
		}
		if (!llst_add(work, (char *) nd)) {
			perror("getsys(): llst_add(nd) failed");
			llst_destroy(work);
			free(nd);
			return(NULL);
		}
		*syslstp = work;
		zerosys(nd);
		(void) strcpy(nd->l_sysname, s);
		return(nd);
	}
	zerosys(sd);
	(void) strcpy(sd->l_sysname, s);
	if (!llst_find(work, (char *) sd, cmpsys)) {
	    /*
	     * create a new link
	     */
#ifdef DEBUG
		if (debug > 1)
			(void) fprintf(stderr, "adding new system - %s\n", s);
#endif
		if (!(nd = (LINK *) malloc(sizeof(LINK) + strlen(s)))) {
#ifdef DEBUG
			(void) fprintf(stderr, "malloc(%d + %d)\n", sizeof(LINK), strlen(s));
#endif
			perror("getsys(): malloc(nd2) failed");
			return((LINK *) NULL);
		}
		if (!llst_add(work, (char *) nd)) {
			perror("getsys(): llst_add(nd2) failed");
			free(nd);
			return(NULL);
		}
		zerosys(nd);
		(void) strcpy(nd->l_sysname, s);
	}
	return((LINK *) llst_current(work));
}

void
zerosys(l)
	LINK	*l;
{
	/*
	 * added for portability vs. memset/bzero
	 */
	l->l_recv	= 0.0;
	l->l_recv_time	= 0.0;
	l->l_recv_cnt	= 0L;
	l->l_xmit	= 0.0;
	l->l_xmit_time	= 0.0;
	l->l_xmit_cnt	= 0L;
	l->l_in_conn	= 0;
	l->l_out_conn	= 0;
	l->l_lastpid	= 0;
	l->l_portlst	= NULL;
	l->l_sysname[0]	= '\0';
}

void
zerodev(p)
	PORT	*p;
{
	/*
	 * added for portability vs. memset/bzero
	 */
	p->p_recv	= 0.0;
	p->p_recv_time	= 0.0;
	p->p_recv_cnt	= 0L;
	p->p_xmit	= 0.0;
	p->p_xmit_time	= 0.0;
	p->p_xmit_cnt	= 0L;
	p->p_in_conn	= 0;
	p->p_out_conn	= 0;
	p->p_lastpid	= 0;
	p->p_syslst	= NULL;
	p->p_dname[0]	= '\0';
}

void 
tsfill(trec, mon, day, hh, mm) 
	SUMM	*trec;
	int	mon;
        int	day;
        int	hh;
        int	mm;
{
	static int	AlreadySetFirst = FALSE;

	if (!AlreadySetFirst) {
#ifdef DEBUG
		if (debug > 2)
			(void) fprintf(stderr, "initialising trec->first_rec (%d/%d-%02d:%02d)\n", mon, day, hh, mm);
#endif
		trec->first_rec.tm_mon = mon;
		trec->first_rec.tm_mday = day;
		trec->first_rec.tm_hour = hh;
		trec->first_rec.tm_min = mm;
		AlreadySetFirst = TRUE;
	} else {
#ifdef DEBUG
		if (debug > 2)
			(void) fprintf(stderr, "initialising trec->last_rec (%d/%d-%02d:%02d)\n", mon, day, hh, mm);
#endif
		trec->last_rec.tm_mon = mon;
		trec->last_rec.tm_mday = day;
		trec->last_rec.tm_hour = hh;
		trec->last_rec.tm_min = mm;
	}
	return;
}

void
sortrun(syslst, devlst, howsys, howdev)
	llst_t		*syslst;
	llst_t		*devlst;
	unsigned short	howsys;
	unsigned short	howdev;
{
	extern cmp_t		cmpdev();
	extern cmp_t		cmpsys();
	extern unsigned short	sort_sys_f;
	extern unsigned short	sort_dev_f;

#ifdef DEBUG
	if (debug)
		(void) fprintf(stderr, "%s: sorting system and device lists.\n", argv0);
#endif
	sort_sys_f = howsys;		/* set global used by cmpsys() */
	llst_sort(syslst, cmpsys);
	llst_top(devlst);
	do {
		PORT	*devp;

		if (! (devp = (PORT *) llst_current(devlst)))
			continue;
		llst_sort(devp->p_syslst, cmpsys);
	} while (llst_next(devlst));
	sort_dev_f = howdev;		/* set global used by cmpdev() */
	llst_sort(devlst, cmpdev);
	llst_top(syslst);
	do {
		LINK	*sysp;

		if (! (sysp = (LINK *) llst_current(syslst)))
			continue;
		llst_sort(sysp->l_portlst, cmpdev);
	} while (llst_next(syslst));
	return;
}

void
printrun(syslst, devlst, trec, printopt)
	llst_t		*syslst;
	llst_t		*devlst;
	SUMM		*trec;
	unsigned short	printopt;
{
#ifdef DEBUG
	if (debug)
		(void) fprintf(stderr, "%s: printing report.\n", argv0);
#endif
	if (printopt & SYS_RPT)
		printsys(syslst, trec, printopt);
#ifdef HAVE_HDBUUCP
	if (printopt & PORT_RPT)
		printdev(devlst, trec, printopt);
#endif
	if (printopt & SUMM_RPT)
		printsum(trec, printopt);
	return;
}

/* ARGSUSED */
void
printsys(syslst, trec, printopt)
	llst_t		*syslst;
	SUMM		*trec;
	unsigned short	printopt;
{
	LINK	*sysp;

	if (printopt & HEADERS) {
		printtmrng(trec, printopt);
		(void) printf("%-8s ", "Remote");
		(void) printf("%9s ", "K-Bytes");
		(void) printf("%9s ", "K-Bytes");
		(void) printf("%9s ", "K-Bytes");
		(void) printf("%5s ", "Hours");
		(void) printf("%5s ", "Hours");
		(void) printf("%6s ", "AvCPS");
		(void) printf("%6s ", "AvCPS");
		(void) printf("%4s ", "#");
		(void) printf("%4s ", "#");
#ifdef HAVE_HDBUUCP
		(void) printf("%3s\n", "#");
#else
		(void) putchar('\n');
#endif

		(void) printf("%-8s ", "SiteName");
		(void) printf("%9s ", "Recv");
		(void) printf("%9s ", "Xmit");
		(void) printf("%9s ", "Total");
		(void) printf("%5s ", "Recv");
		(void) printf("%5s ", "Xmit");
		(void) printf("%6s ", "Recv");
		(void) printf("%6s ", "Xmit");
		(void) printf("%4s ", "Recv");
		(void) printf("%4s ", "Xmit");
#ifdef HAVE_HDBUUCP
		(void) printf("%3s\n", "Con");
#else
		(void) putchar('\n');
#endif

		(void) printf("%s ", "--------");
		(void) printf("%s ", "---------");
		(void) printf("%s ", "---------");
		(void) printf("%s ", "---------");
		(void) printf("%s ", "-----");
		(void) printf("%s ", "-----");
		(void) printf("%s ", "------");
		(void) printf("%s ", "------");
		(void) printf("%s ", "----");
		(void) printf("%s ", "----");
#ifdef HAVE_HDBUUCP
		(void) printf("%s\n", "---");
#else
		(void) putchar('\n');
#endif
	}

	llst_top(syslst);
	do {
		if (! (sysp = (LINK *) llst_current(syslst)))
			continue;
		(void) printf("%-9.9s", sysp->l_sysname);
		(void) printf("%9.3f ", (double) sysp->l_recv / 1024.0);
		(void) printf("%9.3f ", (double) sysp->l_xmit / 1024.0);
		(void) printf("%9.3f ", (double) (sysp->l_xmit + sysp->l_recv) / 1024.0);
		(void) printf("%5.2f ", (double) sysp->l_recv_time / 3600.0);
		(void) printf("%5.2f ", (double) sysp->l_xmit_time / 3600.0);
		if (sysp->l_recv_time != 0.0)	/* divide by zero ? */
			(void) printf("%6.0f ", (double) sysp->l_recv / sysp->l_recv_time);
		else
			(void) printf("%6.0f ", (double) 0);
		if (sysp->l_xmit_time != 0.0)	/* divide by zero ? */
			(void) printf("%6.0f ", (double) sysp->l_xmit / sysp->l_xmit_time);
		else
			(void) printf("%6.0f ", (double) 0);
		(void) printf("%4ld ", sysp->l_recv_cnt);
		(void) printf("%4ld ", sysp->l_xmit_cnt);
#ifdef HAVE_HDBUUCP
		(void) printf("%3d\n", sysp->l_in_conn + sysp->l_out_conn);
		if (printopt & SYSBYPORT_OPT) {
			llst_top(sysp->l_portlst);
			do {
				PORT	*devp;

				if (! (devp = (PORT *) llst_current(sysp->l_portlst)))
					continue;
				(void) printf("%8.8s ", devp->p_dname);
				(void) printf("%9.3f ", (double) devp->p_recv / 1024.0);
				(void) printf("%9.3f ", (double) devp->p_xmit / 1024.0);
				(void) printf("%9.3f ", (double) (devp->p_xmit + devp->p_recv) / 1024);
				(void) printf("%5.2f ", (double) devp->p_recv_time / 3600.0);
				(void) printf("%5.2f ", (double) devp->p_xmit_time / 3600.0);
				if (devp->p_recv_time != 0.0)	/* divide by zero ? */
					(void) printf("%6.0f ", (double) devp->p_recv / devp->p_recv_time);
				else
					(void) printf("%6.0f ", (double) 0);
				if (devp->p_xmit_time != 0.0)	/* divide by zero ? */
					(void) printf("%6.0f ", (double) devp->p_xmit / devp->p_xmit_time);
				else
					(void) printf("%6.0f ", (double) 0);
				(void) printf("%4ld ", devp->p_recv_cnt);
				(void) printf("%4ld ", devp->p_xmit_cnt);
				(void) printf("%3d\n", devp->p_in_conn + devp->p_out_conn);
			} while (llst_next(sysp->l_portlst));
			putchar('\n');
		}
#else
		putchar('\n');
#endif
	} while (llst_next(syslst));
	return;
}

#ifdef HAVE_HDBUUCP
/* ARGSUSED */
void
printdev(devlst, trec, printopt)
	llst_t		*devlst;
	SUMM		*trec;
	unsigned short	printopt;
{
	PORT	*devp;

	if (printopt & HEADERS) {
		printtmrng(trec, printopt);
		(void) printf("%-8s ", "Port");
		(void) printf("%9s ", "K-Bytes");
		(void) printf("%9s ", "K-Bytes");
		(void) printf("%9s ", "K-Bytes");
		(void) printf("%5s ", "Hours");
		(void) printf("%5s ", "Hours");
		(void) printf("%6s ", "AvCPS");
		(void) printf("%6s ", "AvCPS");
		(void) printf("%4s ", "#");
		(void) printf("%4s ", "#");
		(void) printf("%3s\n", "#");

		(void) printf("%-8s ", "Name");
		(void) printf("%9s ", "Recv");
		(void) printf("%9s ", "Xmit");
		(void) printf("%9s ", "Total");
		(void) printf("%5s ", "Recv");
		(void) printf("%5s ", "Xmit");
		(void) printf("%6s ", "Recv");
		(void) printf("%6s ", "Xmit");
		(void) printf("%4s ", "Recv");
		(void) printf("%4s ", "Xmit");
		(void) printf("%3s\n", "Con");

		(void) printf("%s ", "--------");
		(void) printf("%s ", "---------");
		(void) printf("%s ", "---------");
		(void) printf("%s ", "---------");
		(void) printf("%s ", "-----");
		(void) printf("%s ", "-----");
		(void) printf("%s ", "------");
		(void) printf("%s ", "------");
		(void) printf("%s ", "----");
		(void) printf("%s ", "----");
		(void) printf("%s\n", "---");
	}

	llst_top(devlst);
	do {
		if (! (devp = (PORT *) llst_current(devlst)))
			continue;
		(void) printf("%-9.9s", devp->p_dname);
		(void) printf("%9.3f ", (double) devp->p_recv / 1024.0);
		(void) printf("%9.3f ", (double) devp->p_xmit / 1024.0);
		(void) printf("%9.3f ", (double) (devp->p_xmit + devp->p_recv) / 1024.0);
		(void) printf("%5.2f ", (double) devp->p_recv_time / 3600.0);
		(void) printf("%5.2f ", (double) devp->p_xmit_time / 3600.0);
		if (devp->p_recv_time != 0.0)	/* divide by zero ? */
			(void) printf("%6.0f ", (double) devp->p_recv / devp->p_recv_time);
		else
			(void) printf("%6.0f ", (double) 0);
		if (devp->p_xmit_time != 0.0)	/* divide by zero ? */
			(void) printf("%6.0f ", (double) devp->p_xmit / devp->p_xmit_time);
		else
			(void) printf("%6.0f ", (double) 0);
		(void) printf("%4ld ", devp->p_recv_cnt);
		(void) printf("%4ld ", devp->p_xmit_cnt);
		(void) printf("%3d\n", devp->p_in_conn + devp->p_out_conn);
	} while (llst_next(devlst));
	return;
}
#endif

/* ARGSUSED */
void
printsum(trec, printopt)
	SUMM		*trec;
	unsigned short	printopt;
{
	long	total_secs;
	char	buf[BUFSIZ];

	if (printopt & HEADERS)
		printtmrng(trec);
	(void) printf("Active UUCP sites:\t");
	(void) sprintf(buf, "%d", trec->syscnt);
	comma_fy(buf);
	(void) printf("%10s", buf);
	(void) printf("\tTotal time recv:\t%4d:%02d:%02d\n",
		      (int) (trec->sec_recv / 3600.0),
		      (int) (((int) trec->sec_recv % 3600) / 60),
		      (int) ((int) trec->sec_recv % 60));
#ifdef HAVE_HDBUUCP
	(void) printf("Active UUCP ports:\t");
	(void) sprintf(buf, "%d", trec->devcnt);
	comma_fy(buf);
	(void) printf("%10s", buf);
#else
	(void) printf("\t\t\t\t");
#endif
	(void) printf("\tTotal time xmit:\t%4d:%02d:%02d\n",
		      (int) (trec->sec_xmit / 3600.0),
		      (int) (((int) trec->sec_xmit % 3600) / 60),
		      (int) ((int) trec->sec_xmit % 60));
#ifdef HAVE_HDBUUCP
	(void) printf("Connections with work:\t");
	(void) sprintf(buf, "%ld", trec->num_in_conn + trec->num_out_conn);
	comma_fy(buf);
	(void) printf("%10s", buf);
#else
	(void) printf("\t\t\t\t");
#endif
	total_secs = trec->sec_recv + trec->sec_xmit;
	(void) printf("\tTotal UUCP time:\t%4d:%02d:%02d\n",
		      (int) (total_secs / 3600.0),
		      (int) (((long) total_secs % 3600) / 60),
		      (int) ((long) total_secs % 60));
	(void) printf("\nTotal files rec'd:\t");
	(void) sprintf(buf, "%ld", trec->num_recv);
	comma_fy(buf);
	(void) printf("%10s", buf);
	(void) printf("\tTotal bytes rec'd:\t");
	(void) sprintf(buf, "%ld", trec->bytes_recv);
	comma_fy(buf);
	(void) printf("%10s\n", buf);
	(void) printf("Total files xmit:\t");
	(void) sprintf(buf, "%ld", trec->num_xmit);
	comma_fy(buf);
	(void) printf("%10s", buf);
	(void) printf("\tTotal bytes xmit:\t");
	(void) sprintf(buf, "%ld", trec->bytes_xmit);
	comma_fy(buf);
	(void) printf("%10s\n", buf);
	(void) printf("Total files:\t\t");
	(void) sprintf(buf, "%ld", trec->num_xmit + trec->num_recv);
	comma_fy(buf);
	(void) printf("%10s", buf);
	(void) printf("\tTotal bytes:\t\t");
	(void) sprintf(buf, "%ld", trec->bytes_xmit + trec->bytes_recv);
	comma_fy(buf);
	(void) printf("%10s\n", buf);
	return;
}

/*
 * print report header & report time range
 */
void 
printtmrng(trec, printopt)
	SUMM	*trec;
	int	printopt;
{
	static char	*months[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static int	IsAlreadyPrinted = FALSE;

	if (IsAlreadyPrinted) {
		putchar('\n');
		return;
	}
	(void) printf("%*sUUCP Traffic Report\n", 31, " ");
	if (trec->first_rec.tm_mon && trec->last_rec.tm_mon) {
		(void) printf("%*sFrom: %s %2d %02d:%02d   To: %s %2d %02d:%02d\n\n",
			      22, " ",
			      months[trec->first_rec.tm_mon-1],
			      trec->first_rec.tm_mday,
			      trec->first_rec.tm_hour,
			      trec->first_rec.tm_min,
			      months[trec->last_rec.tm_mon-1],
			      trec->last_rec.tm_mday,
			      trec->last_rec.tm_hour,
			      trec->last_rec.tm_min);
	}
	IsAlreadyPrinted = TRUE;
	return;
}

/*
 * put commas in long numeric strings contained in buf[]
 */
void
comma_fy(buf)
	char	buf[];
{
	int	i, ii, cnt;
	char	backw[BUFSIZ];

	/*
	 * put string backwards inserting commas
	 */
	for (ii = 0, cnt = 0, i = strlen(buf) - 1; i >= 0 ; i--) {
		backw[ii++] = buf[i];
		if (buf[i] == '.')
			cnt = 0;
		else if (++cnt == 3) {
			if (i && buf[i - 1] != '\0' && isdigit(buf[i - 1]))
				backw[ii++] = ',';
			cnt = 0;
		}
	}
	backw[ii] = '\0';
	/*
	 * put string forward
	 */
	for (ii = 0, cnt = 0, i = strlen(backw) - 1; i >= 0 ; i--)
		buf[ii++] = backw[i];
	buf[ii]='\0';
}

cmp_t
cmpdev(a, b)
	PORT	*a,
		*b;		/* we rely on alignment of char * */
{
	float			aw = 0, bw = 0;
	extern unsigned short	sort_dev_f;

	if (sort_dev_f & RECEIVED) {
		aw += a->p_recv;
		bw += b->p_recv;
	}
	if (sort_dev_f & XMIT) {
		aw += a->p_xmit;
		bw += b->p_xmit;
	}
	if (sort_dev_f & R_TIME) {
		aw += a->p_recv_time;
		bw += b->p_recv_time;
	}
	if (sort_dev_f & X_TIME) {
		aw += a->p_xmit_time;
		bw += b->p_xmit_time;
	}
	if (sort_dev_f & R_CPS) {
		if (a->p_recv_time != 0)
			aw += (float) a->p_recv / (float) a->p_recv_time;
		if (b->p_recv_time != 0)
			bw += (float) b->p_recv / (float) b->p_recv_time;
	}
	if (sort_dev_f & X_CPS) {
		if (a->p_xmit_time != 0)
			aw += (float) a->p_xmit / (float) a->p_xmit_time;
		if (b->p_xmit_time != 0)
			bw += (float) b->p_xmit / (float) b->p_xmit_time;
	}
	if (sort_dev_f & R_NUMB) {
		aw += a->p_recv_cnt;
		bw += b->p_recv_cnt;
	}
	if (sort_dev_f & X_NUMB) {
		aw += a->p_xmit_cnt;
		bw += b->p_xmit_cnt;
	}
	if (!sort_dev_f || (sort_sys_f == REVERSE) || ((aw == 0) && (bw == 0)))
		aw = strcmp(b->p_dname, a->p_dname);
	if (sort_sys_f & REVERSE)
		return((cmp_t) (aw - bw));
	else
		return((cmp_t) (bw - aw));
}

cmp_t
cmpsys(a, b)
	LINK	*a,
		*b;
{
	float			aw = 0, bw = 0;
	extern unsigned short	sort_sys_f;

	if (sort_sys_f & RECEIVED) {
		aw += a->l_recv;
		bw += b->l_recv;
	}
	if (sort_sys_f & XMIT) {
		aw += a->l_xmit;
		bw += b->l_xmit;
	}
	if (sort_sys_f & R_TIME) {
		aw += a->l_recv_time;
		bw += b->l_recv_time;
	}
	if (sort_sys_f & X_TIME) {
		aw += a->l_xmit_time;
		bw += b->l_xmit_time;
	}
	if (sort_sys_f & R_CPS) {
		if (a->l_recv_time != 0)
			aw += (float) a->l_recv / (float) a->l_recv_time;
		if (b->l_recv_time != 0)
			bw += (float) b->l_recv / (float) b->l_recv_time;
	}
	if (sort_sys_f & X_CPS) {
		if (a->l_xmit_time != 0)
			aw += (float) a->l_xmit / (float) a->l_xmit_time;
		if (b->l_xmit_time != 0)
			bw += (float) b->l_xmit / (float) b->l_xmit_time;
	}
	if (sort_sys_f & R_NUMB) {
		aw += a->l_recv_cnt;
		bw += b->l_recv_cnt;
	}
	if (sort_sys_f & X_NUMB) {
		aw += a->l_xmit_cnt;
		bw += b->l_xmit_cnt;
	}
	if (!sort_sys_f || (sort_sys_f == REVERSE) || ((aw == 0) && (bw == 0)))
		aw = strcmp(b->l_sysname, a->l_sysname);
	if (sort_sys_f & REVERSE)
		return((cmp_t) (aw - bw));
	else
		return((cmp_t) (bw - aw));
}


void
usage_err(arg0, msg)
	char	*arg0;
	char	*msg;
{
	(void) fprintf(stderr, "\nUsage: %s [options] [xferstat_file | -]\n", arg0);
	if (msg && *msg)
		(void) fprintf(stderr, "%s\n", msg);
	exit(2);
	/* NOTREACHED */
}

