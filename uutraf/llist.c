/*
 *	llist.c - linked list sub-class methods
 */

#define SID	"@(#)dlst:llist.c	1.5	92/10/03 23:38:06 (woods)"
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

static void	invalid_op();
static llel_t	*sort_list();
static llel_t	*merge_list();

/*
 * Sillyness to keep assigments of invalid_op from making noise
 */
typedef VOID    (*voidfn)();

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
 *	safety feature
 */
static void
invalid_op()
{
	assert(0);
}

/*
 *	constructor
 */
llst_t	*
new_llist()
{
	lst_t	*lst;
	llst_t	*new;

	if ((lst = lst_new()) == NULL)
		return(NULL);

	if ((new = (llst_t *) malloc(sizeof(llst_t))) == NULL) {
		lst_destroy(lst);
		return(NULL);
	}
	(void) memcpy(new, lst, sizeof(*lst));	/* WARNING:  assumptions! */

	lst_destroy(lst);	/* throw away original super-object */

	new->attop = attop;
	new->atbottom = atbottom;
	new->isempty = isempty;
	new->top = top;
	new->bottom = bottom;
	new->next = next;
	new->prev = prev;
	new->add = add;
	new->find = find;
	new->current = current;
	new->replace = replace;
	new->delete = delete;
	new->display = (voidfn) invalid_op;
	new->ltell = ltell;
	new->total = total;	/* over-ride super-class definition */
	new->sort = (voidfn) sort;

	new->first_el = NULL;
	new->last_el = NULL;
	new->curr_el = NULL;
	new->num_el = 0;

	return(new);
}

/*
 *	destructor
 */
void
destroy_llist(this)
	llst_t	*this;
{
	if (!this)
		return;
	llst_bottom(this);
	while (this->curr_el)
		llst_delete(this);
	/*
	 * NOTE:  the super (or parent) object has already been
	 * destroyed in the constructor for this object.
	 */
	if (this)
		free(this);
	return;
}

/*
 *	methods -- these may not be optimum for all sub-classes
 */

static bool
attop(this)
	llst_t	*this;
{
	assert(this);
	return(this->curr_el == this->first_el);
}

static bool
atbottom(this)
	llst_t	*this;
{
	assert(this);
	return(this->curr_el == this->last_el);
}

static bool
isempty(this)
	llst_t	*this;
{
	assert(this);
	return(this->first_el == NULL);
}

static VOID
top(this)
	llst_t	*this;
{
	if (!this)
		return;
	this->curr_el = this->first_el;
	return;
}

static VOID
bottom(this)
	llst_t	*this;
{
	if (!this)
		return;
	this->curr_el = this->last_el;
	return;
}

static bool
next(this)
	llst_t	*this;
{
	if (!this)
		return(FALSE);
	if (!this->curr_el || !this->curr_el->nxt)
		return(FALSE);
	this->curr_el = this->curr_el->nxt;
	return(TRUE);
}

static bool
prev(this)
	llst_t	*this;
{
	if (!this)
		return(FALSE);
	if (!this->curr_el || !this->curr_el->prv)
		return(FALSE);
	this->curr_el = this->curr_el->prv;
	return(TRUE);
}

static bool
add(this, datum)
	llst_t	*this;
	UnivPtr	datum;
{
	llel_t	*new;

	if (!this)
		return(FALSE);
	if ((new = (llel_t *) malloc(sizeof(llel_t))) == NULL)
		return(FALSE);
	new->d = datum;
	this->num_el++;
	if (!this->first_el) {
		this->first_el = new;
		this->last_el = new;
		new->prv = NULL;
		new->nxt = NULL;
	} else if (this->curr_el == this->last_el) {
		assert(this->curr_el->nxt == NULL);
		this->curr_el->nxt = new;
		this->last_el = new;
		new->prv = this->curr_el;
		new->nxt = NULL;
	} else {
		new->nxt = this->curr_el->nxt;
		this->curr_el->nxt->prv = new;
		this->curr_el->nxt = new;
		new->prv = this->curr_el;
	}
	this->curr_el = new;
	return(TRUE);
}

static VOID
delete(this)
	llst_t	*this;
{
	llel_t	*old;

	if (!this)
		return;
	if (!(old = this->curr_el))
		return;
	if (this->last_el == this->curr_el) {
		this->curr_el = this->last_el = old->prv;
		this->curr_el->nxt = NULL;
	} else if (this->first_el == this->curr_el) {
		this->curr_el = this->first_el = old->nxt;
		this->curr_el->prv = NULL;
	} else {
		this->curr_el = old->prv;
		old->nxt->prv = this->curr_el;
		this->curr_el->nxt = old->nxt;
	}
	if (old->d)
		free(old->d);
	free(old);
	return;
}

static UnivPtr
current(this)
	llst_t	*this;
{
	if (!this || !this->curr_el)
		return(NULL);
	return(this->curr_el->d);
}

static VOID
replace(this, datum)
	llst_t	*this;
	UnivPtr	datum;
{
	assert(this && this->curr_el);
	this->curr_el->d = datum;
	return;
}

static size_t
ltell(this)
	llst_t	*this;
{
	size_t	thisone = 0;
	llel_t	*elem;

	if (!this)
		return(0);
	if (!(elem = this->first_el))
		return(0);
	while (elem != this->curr_el) {
		thisone++;
		elem = elem->nxt;
	}
	return(thisone);
}

static size_t
total(this)
	llst_t	*this;
{
#ifdef NO_COUNT
	size_t	len = 0;
	llel_t	*elem;

	if (!this)
		return(0);
	if (!(elem = this->first_el))
		return(0);
	while (elem) {
		len++;
		elem = elem->nxt;
	}
	return(len);
#else
	if (!this)
		return(0);
	return(this->num_el);
#endif
}

static bool
find(this, el, fn)
	llst_t	*this;
	char	*el;
	cmp_t	(*fn)();
{
	if (!this)
		return(FALSE);
	llst_top(this);
	do {
		if ((*fn)(el, llst_current(this)) == 0)
			return(TRUE);
	} while (llst_next(this));
	return(FALSE);
}

static VOID
sort(this, fn)
	llst_t	*this;
	cmp_t	(*fn)();
{
	if (!this)
		return;
	this->first_el = sort_list(this->first_el, fn);
	this->curr_el = this->first_el;
	while (this->curr_el && this->curr_el->nxt)
		this->curr_el = this->curr_el->nxt;
	this->last_el = this->curr_el;
	this->curr_el = this->first_el;
	return;
}

/*
 * with apologies (though not many!) to R. Sedgewick: Algorithms
 * (1983) - p.  149
 */
static llel_t *
sort_list(lst, fn)
	llel_t	*lst;			/* pointer to first element */
	cmp_t	(*fn)();		/* comparison function */
{
	size_t	lst_len = 0;		/* number of elements */
	llel_t	*lst_a;
	llel_t	*lst_b;
	size_t	i;

	if (!lst || !lst->nxt)				/* recursion termination */
		return(lst);

	lst_a = lst;
	while (lst_a->nxt) {
		lst_len++;
		lst_a = lst_a->nxt;
	}
		
	lst_a = lst;				/* set head of first half */
	for (i = 1; i < (lst_len / 2); i++)	/* skip half (note division) */
		lst = lst->nxt;

	lst_b = lst->nxt;			/* set head of last half */
	lst->nxt = NULL;			/* terminate lst_a */
	lst_b->prv = NULL;			/* and the backptr too */

	return(merge_list(sort_list(lst_a, fn), sort_list(lst_b, fn), fn));
}


/*
 *	merge_list - merge two lists
 */
/*
 * with apologies (though not many!) to R. Sedgewick: Algorithms
 * (1983) - p. 148
 */
static llel_t *
merge_list(lst_a, lst_b, fn)
	llel_t	*lst_a;			/* pointer to first half */
	llel_t	*lst_b;			/* pointer to second half */
	cmp_t	(*fn)();		/* data comparison function */
{
	llel_t	*head;			/* head of the merged list */
	llel_t	*tail;			/* walking tail */

	/*
	 * first, pick the head of the list
	 */
	if (!lst_a && !lst_b)
		return(NULL);
	else if (!lst_a)
		return(lst_b);
	else if (!lst_b)
		return(lst_a);

	if ((*fn)(lst_a->d, lst_b->d) <= 0) {
		head = lst_a;
		tail = lst_a;
		lst_a = lst_a->nxt;
	} else {
		head = lst_b;
		tail = lst_b;
		lst_b = lst_b->nxt;
	}
	/*
	 * while there are still elements on both lists, point to
	 * the list with the next item as determined by strcmp()
	 */
	while (lst_a && lst_b) {
		if ((*fn)(lst_a->d, lst_b->d) <= 0) {
			tail->nxt = lst_a;
			lst_a->prv = tail;
			tail = lst_a;
			lst_a = lst_a->nxt;
		} else {
			tail->nxt = lst_b;
			lst_b->prv = tail;
			tail = lst_b;
			lst_b = lst_b->nxt;
		}
	}
	/*
	 * NOTE: the new list is already terminated.  Now, attatch any
	 * remaining odd lists
	 */
	if (lst_a) {
		tail->nxt = lst_a;
		lst_a->prv = tail;
	} else if (lst_b) {
		tail->nxt = lst_b;
		lst_b->prv = tail;
	}
	return(head);	/* return the head of the merged lists */
}
