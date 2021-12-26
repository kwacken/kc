#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// basic.h - Common types expected everywhere.
//
// Included in "common.h".
//
////////////////////////////////////////////////////////////////////////////////

#include "util.h"
#include "type.h"

#include <stdbool.h>
#include <stddef.h>

////////////////////////////////////////////////////////////////////////////////
// string_t

typedef struct { const char* val; size_t len; } string_t;

// Convert constant string literal into a string.
#define as_string_t(CST)					\
  new(string_t, .val = (CST), .len = array_len((CST)) - 1)

// Create a new string.
/////
// Return value relies on the resources in str.
static inline string_t string(const char* str, size_t);

// Return underlying char array pointer.
static inline const char* string_raw(string_t);

// Return the length of the string.
static inline size_t string_len(string_t);

// Check if two strings are equal.
static inline bool string_eq(string_t, string_t);

////////////////////////////////////////////////////////////////////////////////
// parray_t

#define PARRAY_DECL(NAME, TYPE)			\
  struct NAME { size_t len; pointer(TYPE) inner; }

#define parray_t(TYPE)				\
  PARRAY_DECL(, TYPE)

#define parray_raw(PARRAY)			\
  ((PARRAY)->inner)

#define parray_len(PARRAY)			\
  ((PARRAY)->len)

#define parray_elem_typeof(PARRAY)		\
  array_elem_typeof(typeof(parray_raw(PARRAY)))

#define parray_elem_sizeof(PARRAY)		\
  (sizeof(parray_elem_typeof(PARRAY)))

#define parray_sizeof(PARRAY)				\
  (parray_elem_sizeof(PARRAY) * parray_len(PARRAY))

#define parray_init(PARRAY, PTR, LEN)			\
  do {							\
    __auto_type __parray_init = (PARRAY);			\
    parray_raw(__parray_init) = (PTR);			\
    parray_len(__parray_init) = (LEN);			\
  } while (0)

#define parray_get(PARRAY, IDX)			\
  (parray_raw((PARRAY))[(IDX)])

#define parray_ptr_iter(PARRAY, ITER)					\
  do {									\
    __auto_type __parray = (PARRAY);					\
    ptr_iter_init((ITER), parray_raw(__parray), parray_raw(_parray) + len); \
  } while (0)

#define parray_foreach(VAR, PARRAY)					\
  array_foreach(VAR, parray_len((PARRAY)), parray_raw((PARRAY)))

#define parray_foreach_rev(VAR, PARRAY)					\
  array_foreach_rev(VAR, parray_len((PARRAY)), parray_raw((PARRAY)))

#define parray_idx_foreach(VAR, VAR_IDX, PARRAY)			\
  array_idx_foreach(VAR, VAR_IDX, parray_len((PARRAY)), parray_raw((PARRAY)))

////////////////////////////////////////////////////////////////////////////////
// Iterators

// Iterate from START to END, setting VAR.
#define range_foreach(VAR, START, END)		\
  for ((VAR) = (START);				\
       (VAR) < (END);				\
       (VAR)++)

// Iterate from END to START, setting VAR.
#define range_foreach_rev(VAR, END, START)	\
  for ((VAR) = (END);				\
       (START) < (VAR);				\
       (VAR)--)

////////////////////////////////////////////////////////////////////////////////

// Iterate over ARR of type <type>[LEN], setting VAR.
#define array_foreach(VAR, LEN, ARR)		\
  for ((VAR) = (ARR);				\
       (VAR) != ((ARR) + (LEN));		\
       (VAR)++)

// Iterate over ARR of type <type>[LEN], in reverse, setting VAR.
#define array_foreach_rev(VAR, LEN, ARR)	\
  for ((VAR) = ((ARR) + ((LEN) - 1));		\
       (ARR) < (VAR) || (VAR) == (ARR);		\
       (VAR)--)

// Iterate over ARR of type <type>[LEN], setting VAR_ARR and index VAR_ID.
#define array_idx_foreach(VAR_ARR, VAR_IDX, LEN, ARR)	\
  for ((VAR_ARR) = (ARR), (VAR_IDX) = 0;		\
       (VAR_ARR) != ((ARR) + (LEN));			\
       (VAR_ARR)++, (VAR_IDX)++)

// Iterate over ARR of type <type>[LEN], in reverse, setting VAR_ARR and index
// VAR_IDX.
#define array_idx_foreach_rev(VAR_ARR, VAR_IDX, LEN, ARR)	\
  for ((VAR_ARR) = ((ARR) + ((LEN) - 1)),			\
	 (VAR_IDX) = cast(typeof(VAR_IDX), (LEN));		\
       (ARR) < (VAR_ARR) || (VAR_ARR) == (ARR);			\
       (VAR_ARR)--, (VAR_IDX)--)

////////////////////////////////////////////////////////////////////////////////

#define PTR_ITER_DECL(NAME, TYPE)		\
  struct NAME { pointer(TYPE) start, end; }

#define ptr_iter_t(TYPE)			\
  PTR_ITER_DECL(, TYPE)

#define ptr_iter_init(ITER, START, END)		   \
  do {						   \
    __auto_type __iter = (ITER);		   \
    __iter->start = (START); __iter->end = (END);  \
  } while (0)

#define ptr_iter_finished(ITER)			\
  ((ITER)->start == (ITER)->end)

#define ptr_iter_len(ITER)			\
  (cast(size_t, ((ITER)->end - (ITER)->start)))

#define ptr_iter_peek(ITER)			\
  ((ptr_iter_finished((ITER))) ? NULL : (ITER)->start)

#define ptr_iter_pop(ITER)			\
  ((ptr_iter_finished((ITER))) ? NULL : (ITER)->start++)

#define ptr_iter_rev_pop(ITER)			\
  ((ptr_iter_finished((ITER))) ? NULL : (ITER)->end--)

#define ptr_iter_foreach(VAR, ITER)		\
  while (((VAR) = ptr_iter_pop((ITER))))

#define ptr_iter_foreach_rev(VAR, ITER)		\
  for ((ITER)->end--, (ITER)->start--;		\
       ((VAR) = ptr_iter_rev_pop((ITER)));)

////////////////////////////////////////////////////////////////////////////////
// Private

#include <string.h>

static inline string_t __attribute__((const, unused, warn_unused_result))
string(const char* val, size_t len) {
  return new(string_t, .val = val, .len = len);
}

static inline const char* __attribute__((const, unused, warn_unused_result))
string_raw(string_t s1) {
  return s1.val;
}

static inline size_t __attribute__((const, unused, warn_unused_result))
string_len(string_t s1) {
  return s1.len;
}

static inline bool __attribute__((const, unused, warn_unused_result))
string_eq(string_t s1, string_t s2) {
  return s1.len == s2.len && !memcmp(s1.val, s2.val, s1.len);
}
