/*
 *	alist.c - array list sub-class methods
 */

#define SID	"@(#)dlst:alist.c	1.5	92/10/03 23:37:12 (woods)"
#include <sccsid.h>

/*
 * This is a mid-level class.  The basic structure, as well as simple
 * movement methods have been defined.  However, a further sub-class is
 * required to define the actual array elements, their addition, deletion,
 * and other data-related functions.
 */

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
 * all must have the same types as in LIST_CLASS in list.h
 *
 * NOTE:  Not all of these are defined below!
 */
static bool	attop(),
		atbottom(),
		isempty(),
		add(),
		prev(),
		next(),
		seek(),
		find();
static cmp_t	cmp();
static VOID	top(),
		bottom(),
		replace(),
		delete(),
		display(),
		sort();
static UnivPtr	current(),
		cdr(),
		car();
static size_t	total(),
		ltell();

/*
 *	constructor
 */
alst_t	*
new_alist()
{
	lst_t	*lst;
	alst_t	*new;

	if ((lst = lst_new()) == NULL)
		return(NULL);

	if ((new = (alst_t *) malloc(sizeof(alst_t))) == NULL) {
		lst_destroy(lst);
		return(NULL);
	}
	(void) memcpy(new, lst, sizeof(*lst));	/* mucho assumptions! */
	lst_destroy(lst);

	new->attop = attop;
	new->atbottom = atbottom;
	new->next = next;
	new->prev = prev;
	new->seek = seek;
	new->top = top;
	new->bottom = bottom;
	new->total = total;
	return(new);
}

/*
 * destructor
 */
void
destroy_alist(this)
	alst_t	*this;
{
	/*
	 * NOTE:  the super (or parent) object has already been
	 * destroyed in the constructor for this object.
	 */
	if (this)
		free(this);
	return;
}

static size_t
total(this)
	alst_t	*this;
{
	if (!this)
		return(0);
	return(this->num_el);
}

static bool
attop(this)
	alst_t	*this;
{
	assert(this);
	return(this->curr_el == 0);
}

static bool
atbottom(this)
	alst_t	*this;
{
	assert(this);
	return(this->curr_el == this->num_el);
}

static bool
next(this)
	alst_t	*this;
{
	if (!this)
		return(FALSE);
	assert(this->curr_el >= 0 && this->curr_el <= this->num_el);
	if (this->curr_el >= this->num_el)	/* >= in case NDEBUG set */
		return(FALSE);
	this->curr_el++;
	return(TRUE);
}

static bool
prev(this)
	alst_t	*this;
{
	if (!this)
		return(FALSE);
	assert(this->curr_el >= 0 && this->curr_el <= this->num_el);
	if (!this->curr_el)
		return(FALSE);
	this->curr_el--;
	return(TRUE);
}

static VOID
top(this)
	alst_t	*this;
{
	if (!this)
		return;
	this->curr_el = 0;
	return;
}

static VOID
bottom(this)
	alst_t	*this;
{
	if (!this)
		return;
	this->curr_el = this->num_el - 1;
	return;
}

static bool
seek(this, where, from)
	alst_t	*this;
	size_t	where;		/* destination element or offset */
	int	from;		/* arg. similar to lseek(2s) */
{
	if (!this)
		return(FALSE);
	/*
	 * NOTE:  There is still no major amount of checking in here.  The
	 * seek operation does not change your current element, as the linked
	 * list implementation does.
	 */
	switch (from) {
	case SEEK_SET:
		if (where < this->num_el)
			this->curr_el = where;
		else
			return(FALSE);
		break;
	case SEEK_CUR:
		if (where > 0) {
			if ((this->curr_el + where) < this->num_el)
				this->curr_el += where;
			else
				return(FALSE);
		} else {
			if ((this->curr_el - where) > 0)
				this->curr_el -= where;
			else
				return(FALSE);
		}
		break;
	case SEEK_END:
		if (where <= this->num_el)
			this->curr_el = this->num_el - where;
		else
			return(FALSE);
		break;
	}
	return(TRUE);
}
