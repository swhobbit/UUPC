/*
 *	dlst.h - dynamic list classes
 */

#define SID_H	"@(#)dlst:dlst.h	1.5	92/07/12 14:59:15 (woods)"
#define SID_NM	dlst_sccsid
#include <sccsid.h>

/* WARNING: these are bound to cause trouble for someone! */
#ifndef DLST_HAVE_BOOL
typedef int	bool;		/* boolean result */
#endif
#ifndef DLST_HAVE_CMP_T
typedef int	cmp_t;		/* comparison result */
#endif

#ifndef TRUE
# define TRUE	1
#endif
#ifndef FALSE
# define FALSE	0
#endif

/*
 * generic list base class methods definition
 *
 * NOTE:  The proper way to do this is to define the arguments to the
 * methods with prototyping, but since that's not available we'll do it
 * below with a set of macros for "sending" the messages.
 */
#define LIST_CLASS	\
		bool	(*attop)(), \
			(*atbottom)(), \
			(*isempty)(), \
			(*add)(), \
			(*prev)(), \
			(*next)(), \
			(*seek)(), \
			(*find)(); \
		cmp_t	(*cmp)(); \
		VOID	(*top)(), \
			(*bottom)(), \
			(*replace)(), \
			(*delete)(), \
			(*display)(); \
		UnivPtr	(*current)(), \
			(*cdr)(), \
			(*car)(); \
		size_t	(*total)(), \
			(*ltell)();

typedef struct list {
	LIST_CLASS
} lst_t;

/*
 * constructor and destructor
 */
extern lst_t	*new_list();
extern void	destroy_list();

/*
 * method "messages"
 *
 * NOTE:  make sure we don't de-reference NULL!
 */
#define lst_new			new_list
#define lst_destroy(list)	destroy_list(list)
#define lst_attop(list)		((list) ? ((*((list)->attop))(list)) : 0)
#define lst_atbottom(list)	((list) ? ((*((list)->atbottom))(list)) : 0)
#define lst_isempty(list)	((list) ? ((*((list)->isempty))(list)) : 0)
#define lst_add(list, el)	((list) ? ((*((list)->add))(list, el)) : 0)
#define lst_find(list, el, fn)	((list) ? ((*((list)->find))(list, el, fn)) : 0)
#define lst_cmp(list, a, b)	((list) ? ((*((list)->cmp))(list, a, b)) : (cmp_t) 0)
#define lst_prev(list)		((list) ? ((*((list)->prev))(list)) : 0)
#define lst_next(list)		((list) ? ((*((list)->next))(list)) : 0)
#define lst_top(list)		((list) ? ((*((list)->top))(list)) : (VOID) 0)
#define lst_bottom(list)	((list) ? ((*((list)->bottom))(list)) : (VOID) 0)
#define lst_seek(list, w, s)	((list) ? ((*((list)->seek))(list, w, s)) : 0)
#define lst_replace(list, el)	((list) ? ((*((list)->replace))(list, el)) : (VOID) 0)
#define lst_delete(list)	((list) ? ((*((list)->delete))(list)) : (VOID) 0)
#define lst_display(list, f)	((list) ? ((*((list)->display))(list, f)) : (VOID) 0)
#define lst_current(list)	((list) ? ((*((list)->current))(list)) : (UnivPtr) 0)
#define lst_car(list)		((list) ? ((*((list)->car))(list)) : (UnivPtr) 0)
#define lst_cdr(list)		((list) ? ((*((list)->cdr))(list)) : (UnivPtr) 0)
#define lst_total(list)		((list) ? ((*((list)->total))(list)) : (size_t) 0)
#define lst_ltell(list)		((list) ? ((*((list)->ltell))(list)) : (size_t) 0)

/*
 * a doubly linked list sub-class methods definition.
 *
 * NOTE:  data elements are also defined.
 */
typedef struct llist_element {
	struct llist_element	*nxt;
	struct llist_element	*prv;
	UnivPtr			d;
} llel_t;

#define LINKED_LIST_CLASS \
		LIST_CLASS \
		VOID	(*sort)(); \
		llel_t	*first_el; \
		llel_t	*last_el; \
		llel_t	*curr_el; \
		size_t	num_el;

typedef struct llist {
	LINKED_LIST_CLASS
} llst_t;

/*
 * constructor and destructor
 */
extern llst_t	*new_llist();
extern void	destroy_llist();

/*
 * method "messages"
 */
#define llst_new		new_llist
#define llst_destroy(list)	destroy_llist(list)
#define llst_attop(list)	lst_attop(list)
#define llst_atbottom(list)	lst_atbottom(list)
#define llst_isempty(list)	lst_isempty(list)
#define llst_add(list, el)	lst_add(list, el)
#define llst_find(list, el, fn)	lst_find(list, el, fn)
#define llst_cmp(list, a, b)	lst_cmp(list, a, b)
#define llst_prev(list)		lst_prev(list)
#define llst_next(list)		lst_next(list)
#define llst_top(list)		lst_top(list)
#define llst_seek(list, w, s)	lst_seek(list, w, s)
#define llst_bottom(list)	lst_bottom(list)
#define llst_replace(list, el)	lst_replace(list, el)
#define llst_delete(list)	lst_delete(list)
#define llst_display(list, f)	lst_display(list, f)
#define llst_current(list)	lst_current(list)
#define llst_car(list)		lst_car(list)
#define llst_cdr(list)		lst_cdr(list)
#define llst_total(list)	lst_total(list)
#define llst_ltell(list)	lst_ltell(list)
#define llst_sort(list, f)	((list) ? ((*((list)->sort))(list, f)) : (VOID) 0)

/*
 * an array list sub-class methods definition
 *
 * NOTE:  data elements are also defined.
 */
#define ARRAY_LIST_CLASS \
		LIST_CLASS \
		size_t	curr_el; \
		size_t	num_el;

typedef struct alist {
	ARRAY_LIST_CLASS
} alst_t;

/*
 * constructor and destructor
 */
extern alst_t	*new_alist();
extern void	destroy_alist();

/*
 * method "messages"
 */
#define alst_new		new_alist
#define alst_destroy(list)	destroy_alist(list)
#define alst_attop(list)	lst_attop(list)
#define alst_atbottom(list)	lst_atbottom(list)
#define alst_isempty(list)	lst_isempty(list)
#define alst_add(list, el)	lst_add(list, el)
#define alst_find(list, el, fn)	lst_find(list, el, fn)
#define alst_cmp(list, a, b)	lst_cmp(list, a, b)
#define alst_prev(list)		lst_prev(list)
#define alst_next(list)		lst_next(list)
#define alst_top(list)		lst_top(list)
#define alst_seek(list, w, s)	lst_seek(list, w, s)
#define alst_bottom(list)	lst_bottom(list)
#define alst_replace(list, el)	lst_replace(list, el)
#define alst_delete(list)	lst_delete(list)
#define alst_display(list, f)	lst_display(list, f)
#define alst_current(list)	lst_current(list)
#define alst_car(list)		lst_car(list)
#define alst_cdr(list)		lst_cdr(list)
#define alst_total(list)	lst_total(list)
#define alst_ltell(list)	lst_ltell(list)
