#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// list.h - Defines generic, instrinsic linked list definitions and manipulation
// functions.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Singly linked list (slist)
//
// All HEAD and ENTRY arguments expect a pointer.
//
// SLIST_DECL(foo_list, struct foo);
// struct foo { int x; SLIST_ENTRY(struct foo) next; };
//
// struct foo_list mylist;
// slist_init(&mylist);
//
// struct foo foo0 = { 0, SLIST_ENTRY_INIT(&foo0) };
// slist_insert(&mylist, &foo0, next);
// assert(!slist_is_empty(&mylist));


// Declare a slist NAME on elements TYPE.
#define SLIST_DECL(NAME, TYPE)			\
  struct NAME { TYPE* slisth_head; }

// Declares the field in a struct to use for the slist.
/////
// SLIST_DECL(foo_slist, struct foo);
// struct foo { ...; SLIST_ENTRY(struct foo) next; };
#define SLIST_ENTRY(TYPE)			\
  struct { TYPE* sliste_next; }

// Initialize a slist list.
/////
// SLIST_DECL(, struct foo) mylist; slist_init(&mylist);
#define slist_init(HEAD)			\
  do {						\
    slist_first(HEAD) = NULL;			\
  } while(0)

// Initialize a slist list head HEAD using assignment.
/////
// mylist = SLSIT_HEAD_INIT(&mylist);
#define SLIST_HEAD_INIT(HEAD)			\
  { NULL }

// Initialize a slist entry field using assignment.
#define SLIST_ENTRY_INIT(ENTRY)			\
  { NULL }

// Check if the slist is empty.
#define slist_is_empty(HEAD)			\
  ((HEAD)->slisth_head == NULL)

// Get the first element of the slist.
#define slist_first(HEAD)			\
  ((HEAD)->slisth_head)

// Get the next element of a slist after ENTRY, using FIELD.
#define slist_next(ENTRY, FIELD)			\
  ((ENTRY)->FIELD.sliste_next)

// Get the rest of a list, creating a new slist.
#define slist_rest(HEAD, FIELD)					\
  ((typeof(*HEAD)) { slist_next((HEAD)->slisth_head, FIELD) })

// Insert the value ENTRY at HEAD using FIELD.
#define	slist_insert(HEAD, ENTRY, FIELD)		\
  do {							\
    slist_next((ENTRY), FIELD) = slist_first((HEAD));	\
    slist_first((HEAD)) = (ENTRY);			\
  } while (0)

// Remove the first element of the slist.
#define slist_remove_head(HEAD, FIELD)		\
  do {						\
    *(HEAD) = slist_rest((HEAD), FIELD);	\
  } while (0)

// Remove the first element of the slist, storing it in FIRST.
#define slist_pop(HEAD, FIRST, FIELD)		\
  do {						\
    (FIRST) = slist_first((HEAD));		\
    slist_remove_head((HEAD), FIELD);		\
  } while (0)

// Drop all elements of a list and make it empty.
/////
// Alias for slist_init
#define slist_dropall(HEAD)			\
  slist_init(HEAD)

// Reverse a slist using FIELD.
#define slist_reverse(HEAD, FIELD) \
  do {							\
    typeof(*(HEAD)) __rev_list;				\
    slist_init(&__rev_list);				\
    while (!slist_is_empty((HEAD))) {			\
      typeof(slist_first((HEAD))) __list_top;		\
      slist_pop((HEAD), __list_top, FIELD);		\
      slist_insert(&__rev_list, __list_top, FIELD);}	\
    *(HEAD) = __rev_list;				\
  } while (0)

// Get the length of the slist using FIELD.
#define slist_len(HEAD, FIELD)				\
  ({							\
    size_t __len = 0;					\
    typeof(slist_first((HEAD))) __elem;			\
    slist_foreach(__elem, (HEAD), FIELD) ++__len;	\
    __len; })

// Iterator for slists using FIELD.
#define slist_foreach(VAR, HEAD, FIELD)		\
  for ((VAR) = slist_first(HEAD);		\
       (VAR);					\
       (VAR) = slist_next((VAR), FIELD))

// Checks if this is the last entry in the iterator.
#define slist_foreach_is_last(ENTRY, FIELD)	\
  (slist_next((ENTRY), FIELD) == NULL)

// Iterator for slists, starting from ENTRY, rather than HEAD.
#define slist_foreach_entry(VAR, ENTRY, FIELD)	\
  for ((VAR) = (ENTRY);				\
       (VAR);					\
       (VAR) = slist_next((VAR), FIELD))

// Iterator for slists using FIELD, returns a 2-star reference for use with
// slist_remove.
#define slist_mut_foreach(VAR, HEAD, FIELD)	\
  for ((VAR) = &slist_first(HEAD);		\
       *(VAR);					\
       (VAR) = &slist_next(*(VAR), FIELD))

// Removes the given entry from the list.
#define slist_remove(ENTRY_PTR, FIELD)			\
  do {							\
    if (*(ENTRY_PTR) != NULL) {				\
      *(ENTRY_PTR) = slist_next(*(ENTRY_PTR), FIELD);	\
    }							\
  } while (0)
