/*
 *	defs.h - configuration for uutraf
 */

#define SID_H	"@(#)uutraf:defs.h-dist	1.2.2.5	92/10/23 10:27:14 (woods)"
#define SID_NM	defs_sccsid
#include <sccsid.h>

/*
 * This file contains user defined parameters
 */

#define HAVE_HDBUUCP	1	/* HoneyDanBer-UUCP xferstats? */

#ifdef HAVE_HDBUUCP
# if defined(sun4) || defined(SYSVR4)
#  define STATFILE	"/var/uucp/.Admin/xferstats"
# else
#  if defined(_AIX) && defined(_IBMR2)	/* really should be AIX 3.2.x */
#   define STATFILE	"/var/spool/uucp/.Admin/xferstats"
#  else
#   define STATFILE	"/usr/spool/uucp/.Admin/xferstats"
#  endif
# endif
#else
# if defined(ultrix) || defined(__ultrix__) || defined(BSD4_3)
#  define STATFILE	"/var/spool/uucp/SYSLOG"	/* all the same? */
# else
#  define STATFILE	"/usr/spool/uucp/SYSLOG"	/* all the same? */
# endif
#endif

#if defined(_AIX) && !defined(_IBMR2)	/* really should be AIX 3.1.x ??? */
# define NAME_MAX	PATH_MAX	/* they say you should use pathconf() */
#endif

/*
 * default report to print (you might want to add in SYSBYPORT_OPT (with
 * SYS_RPT))
 */
#define DEFAULT_RPT	(HEADERS | SYS_RPT | PORT_RPT | SUMM_RPT)

/*
 * default way to sort (both node and port sections)
 */
#define DEFAULT_SORT	(RECEIVED | XMIT)	/* Total number of bytes */

/* you can instead pick from one of these for the default sorting method.
 *
 * Sort by number of bytes
 *	#define DEFAULT_SORT (RECEIVED)
 *	#define DEFAULT_SORT (XMIT)
 *	#define DEFAULT_SORT (RECEIVED | XMIT)
 *	#define DEFAULT_SORT (RECEIVED | REVERSE)
 *	#define DEFAULT_SORT (XMIT | REVERSE)
 *	#define DEFAULT_SORT (RECEIVED | XMIT | REVERSE)
 *
 * Sort by transfer rate
 *	#define DEFAULT_SORT (R_CPS)
 *	#define DEFAULT_SORT (X_CPS)
 *	#define DEFAULT_SORT (R_CPS | X_CPS)
 *	#define DEFAULT_SORT (R_CPS | REVERSE)
 *	#define DEFAULT_SORT (X_CPS | REVERSE)
 *	#define DEFAULT_SORT (R_CPS | X_CPS | REVERSE)
 *
 * Sort by number of file transactions
 *	#define DEFAULT_SORT (R_NUMB)
 *	#define DEFAULT_SORT (X_NUMB)
 *	#define DEFAULT_SORT (R_NUMB | X_NUMB)
 *	#define DEFAULT_SORT (R_NUMB | REVERSE)
 *	#define DEFAULT_SORT (X_NUMB | REVERSE)
 *	#define DEFAULT_SORT (R_NUMB | X_NUMB | REVERSE)
 *
 * Sort by modem clock time
 *	#define DEFAULT_SORT (R_TIME)
 *	#define DEFAULT_SORT (X_TIME)
 *	#define DEFAULT_SORT (X_TIME | R_TIME)
 *	#define DEFAULT_SORT (R_TIME | REVERSE)
 *	#define DEFAULT_SORT (X_TIME | REVERSE)
 *	#define DEFAULT_SORT (X_TIME | R_TIME | REVERSE)
 */

