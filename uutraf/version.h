/*
 *	version.h
 */

#define SID_H	"@(#)uutraf:version.h	1.2	92/10/12 16:06:30 (woods)"
#define SID_NM	version_sccsid
#include <sccsid.h>

/*
 * const: readonly type qualifier (use may break some pre-ANSI compilers!)
 */
#if (__STDC__ - 0) == 0 && !defined(const)
# define const	/* Only in ANSI C.  Pity */
#endif	/* !__STDC__ */

EXT const short	version		INIT(VER_ID);
EXT const short	subver		INIT(VER_SUB);
EXT const short	patchlevel	INIT(PATCHLEVEL);
