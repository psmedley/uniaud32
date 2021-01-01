/* $Id: list.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H
#include <linux/types.h>
#ifdef __KERNEL__

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __list_add(struct list_head * new,
	struct list_head * prev,
	struct list_head * next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/*
 * Insert a new entry after the specified head..
 */
static __inline__ void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/*
 * Insert a new entry before the specified head..
 */
static __inline__ void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static __inline__ void __list_del(struct list_head * prev,
				  struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static __inline__ void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static __inline__ int list_empty(struct list_head *head)
{
	return head->next == head;
}

/*
 * Splice in "list" into "head"
 */
static __inline__ void list_splice(struct list_head *list, struct list_head *head)
{
	struct list_head *first = list->next;

	if (first != list) {
		struct list_head *last = list->prev;
		struct list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}

_WCRTLINK extern int     snprintf( char *__buf, size_t __bufsize,
                                   const char *__fmt, ... );

#define offsetof(__typ,__id) ((size_t)&(((__typ*)0)->__id))

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_last_entry - get the last element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/**
 * list_next_entry - get the next element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_struct within the struct.
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * list_prev_entry - get the prev element in list
 * @pos:	the type * to cursor
 * @member:	the name of the list_struct within the struct.
 */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_for_each(entry, listhead) \
        for(entry=(listhead)->next;entry != listhead;entry=entry->next) 

#endif /* __KERNEL__ */

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))
 */
#define list_for_each_entry(itemptr, headptr, struct_listmember_name, container_type) \
    for (itemptr=(container_type *) \
         (((char *)((headptr)->next))-offsetof(container_type, struct_listmember_name)); \
         &(itemptr->struct_listmember_name)!=(headptr); \
         itemptr=(container_type *) \
         (((char *)(itemptr->struct_listmember_name.next))-offsetof(container_type, struct_listmember_name)))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
	for (pos = list_last_entry(head, typeof(*pos), member);		\
	     &pos->member != (head); 					\
	     pos = list_prev_entry(pos, member))

 */

#define list_for_each_entry_reverse(itemptr, headptr, struct_listmember_name, container_type)			\
    for (itemptr=(container_type *) \
         (((char *)((headptr)->prev))-offsetof(container_type, struct_listmember_name)); \
         &(itemptr->struct_listmember_name)!=(headptr); \
         itemptr=(container_type *) \
         (((char *)(itemptr->struct_listmember_name.prev))-offsetof(container_type, struct_listmember_name)))



#define list_for_each_entry_safe(itemptr, n, headptr, struct_listmember_name, container_type) \
    for (itemptr=(container_type *) \
         (((char *)((headptr)->next))-offsetof(container_type, struct_listmember_name)), \
         n=(container_type *) \
         (((char *)(itemptr->struct_listmember_name.next))-offsetof(container_type, struct_listmember_name)); \
         &(itemptr->struct_listmember_name)!=(headptr); \
         itemptr=n, \
         n=(container_type *) \
         (((char *)(n->struct_listmember_name.next))-offsetof(container_type, struct_listmember_name)))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(itemptr, n, headptr, struct_listmember_name, container_type) \
    for (itemptr=(container_type *) \
         (((char *)((headptr)->prev))-offsetof(container_type, struct_listmember_name)), \
         n=(container_type *) \
         (((char *)(itemptr->struct_listmember_name.prev))-offsetof(container_type, struct_listmember_name)); \
         &(itemptr->struct_listmember_name)!=(headptr); \
         itemptr=n, \
         n=(container_type *) \
         (((char *)(n->struct_listmember_name.prev))-offsetof(container_type, struct_listmember_name)))

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static __inline__ void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry); 
}

static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member) \
	(!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int list_is_singular(const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}

#endif
