/*
 *	str.h
 *
 * some stuff borrowed from C News and elsewhere...
 */

#define SID_H	"@(#)head:str.h	1.2	92/04/20 09:53:14 (woods)"
#define SID_NM	str_sccsid
#include "sccsid.h"

/*
 *	simple in-lining for older compilers...
 */
#ifndef FASTSTRCHR
# define STRCHR(src, chr, dest)		(dest) = strchr(src, chr)
#else
# define STRCHR(src, chr, dest) \
	for ((dest) = (src); *(dest) != '\0' && *(dest) != (chr); ++(dest)) \
		; \
	if (*(dest) == '\0') \
		(dest) = NULL		/* N.B.: missing semi-colon */
#endif

/*
 *	more useful things...
 */

/*
 * STREQ is an optimised strcmp(a,b)==0
 */
#define STREQ(s1, s2)	((s1)[0] == (s2)[0] && strcmp(s1, s2) == 0)
#define STRNE(s1, s2)	(!((s1)[0] == (s2)[0] && strcmp(s1, s2) == 0))

#define STRLT(s1, s2)	(strcmp(s1, s2) < 0)
#define STRGT(s1, s2)	(strcmp(s1, s2) > 0)
#define STRLE(s1, s2)	(strcmp(s1, s2) <= 0)
#define STRGE(s1, s2)	(strcmp(s1, s2) >= 0)

/*
 * STREQN is an optimised strncmp(a,b,n)==0; assumes n > 0
 */
#define STREQN(s1, s2, len)	((s1)[0] == (s2)[0] && strncmp(s1, s2, len) == 0)
#define STRNEN(s1, s2, len)	(!((s1)[0] == (s2)[0] && strncmp(s1, s2, len) == 0))

#define STRLTN(s1, s2, len)	(strncmp(s1, s2, len) < 0)
#define STRGTN(s1, s2, len)	(strncmp(s1, s2, len) > 0)
#define STRLEN(s1, s2, len)	(strncmp(s1, s2, len) <= 0)
#define STRGEN(s1, s2, len)	(strncmp(s1, s2, len) >= 0)

/* see case.h for case-independent stuff */
