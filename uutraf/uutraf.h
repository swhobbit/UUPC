/*
 *	uutraf.h - common header for uutraf
 */

#define SID_H	"@(#)uutraf:uutraf.h	1.2.2.7	92/10/23 10:26:35 (woods)"
#define SID_NM	uutraf_sccsid
#include <sccsid.h>

/*
 * these defines are part of the sort routine to figure out how to sort the
 * linked list
 */
#define REVERSE  00001  /* reverse the order, least to most */
#define RECEIVED 00002  /* sort by received */
#define XMIT     00004  /* sort by xmit */
#define R_TIME   00010  /* sort by received time */
#define X_TIME   00020  /* sort by xmit time */
#define R_CPS    00100  /* sort by characters RECEIVED per second */
#define X_CPS    00200  /* sort by characters TRANSMITTED per second */
#define R_NUMB   01000  /* sort by number of RECEIVED transactions */
#define X_NUMB   02000  /* sort by number of TRANSMITTED transactions */

/*
 * print options
 */
#define HEADERS		001
#define SYS_RPT		002
#define PORT_RPT	004
#define SUMM_RPT	010
#define SYSBYPORT_OPT	020

/*
 * other various things
 */
#ifdef HAVE_HDBUUCP		/* Most BNU (aka. HDB) systems */
# define OUT	"->"  /*uucp flow direction indicator*/
# define IN	"<-"  /*uucp flow direction indicator*/
#else				/* Most "old" UUCP's (i.e. with SYSLOG) */
# define OUT	"sent"
# define IN	"received"
#endif

#ifdef pyr	/* UUCP via TCP/IP on an ATT Pyramid */
#define T_OUT	"t->"  /* t protocol outgoing */
#define T_IN	"t<-"  /* t protocol incoming */
#define F_OUT	"f->"  /* f protocol outgoing */
#define F_IN	"f<-"  /* f protocol incoming */
#endif

/*
 * structures for storing totals and summary results
 */
typedef struct port {
	long		p_xmit;		/* # of bytes sent on this device */
	float		p_xmit_time;	/* total seconds xmit'ing on this device */
	long		p_xmit_cnt;	/* total number of files sent */
	long		p_recv;		/* # of bytes rec'd on this device */
	float		p_recv_time;	/* total seconds rec'ing on this device */
	long		p_recv_cnt;	/* total number of files received */
	int		p_in_conn;	/* total number of slave connections */
	int		p_out_conn;	/* total number of master connections */
	int		p_lastpid;	/* pid found in previous data line */
	llst_t		*p_syslst;	/* per-system summary */
	char		p_dname[NAME_MAX + 1];	/* device name (not path) */
} PORT;

typedef struct link {
	long		l_xmit;		/* # of bytes sent to remote system */
	float		l_xmit_time; 	/* total seconds spent in transmit */
	long		l_xmit_cnt;	/* total number of files sent */
	long		l_recv;		/* # of bytes rec'd from remote system */
	float		l_recv_time; 	/* total seconds spent in receive */
	long		l_recv_cnt;	/* total number of files received */
	int		l_in_conn;	/* total number of slave connections */
	int		l_out_conn;	/* total number of master connections */
	int		l_lastpid;	/* pid found in previous data line */
	llst_t		*l_portlst;	/* per-port summary */
	char		l_sysname[1];	/* name of remote system */
} LINK;

typedef struct summ {
	int		syscnt;		/* total active sites */
	int		devcnt;		/* total active ports */
	long		bytes_recv;	/* total bytes received */
	long		bytes_xmit;	/* total bytes transmitted */
	float		sec_recv;	/* total seconds spent in receive */
	float		sec_xmit;	/* total seconds spent in transmit */
	long		num_recv;	/* total number of files received */
	long		num_xmit;	/* total number of files transmitted */
	long		num_in_conn;	/* total number of slave connections */
	long		num_out_conn;	/* total number of master connections */
	struct tm	first_rec;	/* timestamp in first record seen */
	struct tm	last_rec;	/* timestamp in last record seen */
} SUMM;

static
char usage[]="\n\
\t-H\tinclude headers in non-default report\n\
\t-M\tprint merged port summary by system (implies -N)\n\
\t-N\tprint nodes summary\n\
\t-P\tprint ports summary\n\
\t-S\tprint total summary\n\
\t-V\tprint version identification\n\
\t-a\tsort in ascending vs. descending order\n\
\t-b[rxb]\tsort by total bytes rec'd, xmit'ed, or both\n\
\t-c[rxb]\tsort by chars per second rec'd, xmit'ed, or both\n\
\t-f[rxb]\tsort by # of files rec'd, xmit'ed, or both\n\
\t-h\tprint this help message\n\
\t-n\tsort by {system,port} name (must be used alone)\n\
\t-s\tuse standard input instead of default xferstats file\n\
\t-t[rxb]\tsort by total clock time in rec, xmit, or both";
