/*
 *	list.c - generic list class methods
 */

#define SID	"@(#)dlst:list.c	1.5	92/10/03 23:37:50 (woods)"
#include <sccsid.h>

#if defined(USE_STDDEF) || REALSTDC || (__STDC__ - 0) == 1
# include <stddef.h>	/* should do before sysdefs.h */
#endif
#include <sysdefs.h>	/* localisation helpers */
#include <sys/types.h>	/* for size_t, if avail. */
#include <stdio.h>	/* only for "FILE" used in <libc.h> */
#include <assert.h>
#if defined(USG) || defined(SYSV)
# include <malloc.h>
# include <memory.h>
#ifndef UUPC
# include <unistd.h>
#endif
#endif
#include <libc.h>
#include "dlst.h"

/*
 * Sillyness to keep assigments of invalid_op from making noise
 */
typedef VOID	(*voidfn)();
typedef bool	(*boolfn)();
typedef cmp_t	(*cmp_tfn)();
typedef size_t	(*size_tfn)();
typedef UnivPtr	(*UnivPtrFn)();

static void	invalid_op();

/*
 * all must have the same types as in LIST_CLASS in list.h
 *
 * NOTE:  Not all of these are defined!
 */
static bool	attop(),
		atbottom(),
		isempty(),
		add(),
		prev(),
		next(),
		seek(),
		find();
static	cmp_t	cmp();
static	VOID	top(),
		bottom(),
		replace(),
		delete(),
		display();
static	UnivPtr	current(),
		cdr(),
		car();
static	size_t	total(),
		ltell();

/*
 *	safety feature
 */
static void
invalid_op()
{
	assert(0);
	return;		/* usually not reached! */
}

/*
 *	constructor
 */
lst_t	*
new_list()
{
	lst_t	*new;

	if ((new = (lst_t *) malloc(sizeof(lst_t))) == NULL)
		return(NULL);

	new->attop = (boolfn) invalid_op;
	new->atbottom = (boolfn) invalid_op;
	new->isempty = (boolfn) isempty;
	new->add = (boolfn) invalid_op;
	new->find = (boolfn) invalid_op;
	new->cmp = (cmp_tfn) invalid_op;
	new->prev = (boolfn) invalid_op;
	new->next = (boolfn) invalid_op;
	new->top = (voidfn) invalid_op;
	new->bottom = (voidfn) invalid_op;
	new->seek = seek;
	new->replace = (voidfn) invalid_op;
	new->delete = (voidfn) invalid_op;
	new->display = (voidfn) invalid_op;
	new->current = (UnivPtrFn) invalid_op;
	new->cdr = (UnivPtrFn) invalid_op;
	new->car = (UnivPtrFn) invalid_op;
	new->total = (size_tfn) total;
	new->ltell = (size_tfn) invalid_op;

	return(new);
}

/*
 *	destructor
 */
void
destroy_list(this)
	lst_t	*this;
{
	if (this)
		free(this);
	return;
}

/*
 *	methods -- these may not be optimum for all sub-classes
 */

static bool
isempty(this)
	lst_t	*this;
{
	return(lst_total(this) == 0);
}

static bool
seek(this, where, from)
	lst_t	*this;
	size_t	where;		/* destination element or offset */
	int	from;		/* arg. similar to lseek(2s) */
{
	size_t	count;

	if (!this)
		return(FALSE);
	/*
	 * NOTE:  upon failure, the current element is always set to the
	 * top.  This is because we don't want to be re-cursive in re-setting
	 * our position, in case the data is really corrupted.
	 */
	switch (from) {
	case SEEK_SET:
		lst_top(this);
		for (count = (long) 0; count < where; ++count) {
			if (lst_atbottom(this)) {
				lst_top(this);
				return(FALSE);
			}
			(void) lst_next(this);
		}
		break;
	case SEEK_CUR:
		if (where > 0) {
			for (count = (long) 0; count < where; ++count) {
				if (lst_atbottom(this)) {
					lst_top(this);
					return(FALSE);
				}
				(void) lst_next(this);
			}
		} else if (where < 0) {
			for (count = (long) 0; count < where; ++count) {
				if (lst_attop(this))
					return(FALSE);
				(void) lst_prev(this);
			}
		}
		break;
	case SEEK_END:
		lst_bottom(this);
		for (count = (long) 0; count < where; ++count) {
			if (lst_attop(this))
				return(FALSE);
			(void) lst_prev(this);
		}
		break;
	}
	return(TRUE);
}

static size_t
total(this)
	lst_t	*this;
{
	size_t	thisone,
		count = (long) 0;

	if (!this)
		return(0);
	thisone = lst_ltell(this);
	lst_top(this);
	do {
		if (!lst_atbottom(this)) {
			++count;
			lst_next(this);
		}
	} while (!lst_atbottom(this));
	lst_seek(this, thisone, SEEK_SET);
	return(count);
}
