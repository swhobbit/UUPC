/*
 *	sccsid.h - handle various incantations for SID's
 *
 * #ident "@(#)head:sccsid.h	1.3	92/10/09 23:39:52 (woods)"
 *
 *
 * USE (as the very first statements in a C source module w/out '\'s):
 *
 *	#define SID	"%Z\%%Y\%:%M\%	%I\%	%E\% %U\% (%Q\%)"
 *	#include "sccsid.h"
 *
 * USE (as the very first statement in a C header module w/out '\'s):
 *
 *	#define SID_NM	module_sccsid	* the full name of the static array *
 *	#define SID_H	"%Z\%%Y\%:%M\%	%I\%	%E\% %U\% (%Q\%)"
 *	#include "sccsid.h"
 *
 * [Use tab characters around the '%I\%'.]
 *
 * NOTE:  assumes SYSVR3 or SYSVR4 or SMART_CPP will be defined on the
 *	  compiler command line if appropriate.
 *
 * WARNING:  as a side-effect, this file define's and undef's
 *	     both SMART_CPP and MYconst
 */

#if !defined(_AIX) && (defined(SYSVR3) || defined(SYSVR4))
# define SMART_CPP
# define SMART_IDENT
#endif

#if defined(SYSVR2)	/* You might want this, but it's usually a no-op. */
/*# define SMART_CPP		/* use this if your compiler has #sccs */
/*# define SMART_SCCS		/* you must also uncomment '#sccs' lines below */
#endif

#if defined(__STDC__) && (__STDC__ > 0)
# define MYconst	const
#else
# define MYconst	/* no_op */
#endif

#ifdef SMART_CPP

# ifdef SID
#  ifdef SMART_IDENT
/*#   ident	SID	/* comment this out if your cpp blows up */
#  endif
#  ifdef SMART_SCCS
#   include "You must uncomment the #sccs lines for SMART_SCCS to work!"
/*#   sccs	SID	/* comment the above line if you uncomment this one */
#  endif
#  undef SID
# else
#  ifdef SID_H
#   ifdef SID_NM
#    ifdef SMART_IDENT
/*#     ident	SID_H	/* unset SMART_CPP if your compiler blows up */
#    endif
#    ifdef SMART_SCCS
#     include "You must uncomment the #sccs lines for SMART_SCCS to work!"
/*#     sccs	SID_H	/* comment the above line if you uncomment this one */
#    endif
#    undef SID_NM
#    undef SID_H
#   else
#    include	"ERROR: you must define SID_NM to use SID_H"
#   endif
#  else
#   include	"ERROR: you must define either SID or SID_H to use sccsid.h"
#  endif
# endif

#else /* !SMART_CPP */

# if !(defined(lint) || defined(M_LINT)) || defined(DEBUG)
#  ifdef SID
static MYconst char	sccsid[] = SID;
#   undef SID
#  else
#   ifdef SID_H
#    ifdef SID_NM
static MYconst char	SID_NM[] = SID_H;
#     undef SID_NM
#     undef SID_H
#    else
#     include	"ERROR: you must define SID_NM to use SID_H"
#    endif
#   else
#    include	"ERROR: you must define either SID or SID_H to use sccsid.h"
#   endif
#  endif
# endif

#endif

#undef MYconst
#undef SMART_CPP
