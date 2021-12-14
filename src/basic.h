#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// basic.h - Common types expected everywhere.
//
// Included in "common.h".
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// bool_t

typedef unsigned char bool_t;
static const bool_t __attribute__((unused)) TRUE = 0xFF;
static const bool_t __attribute__((unused)) FALSE = 0x00;

////////////////////////////////////////////////////////////////////////////////
// string_t

#include <string.h>

typedef struct { const char* val; size_t len; } string_t;

#define as_string_t(CST)						\
  ((string_t) { .val = (CST), .len = array_len((CST)) - 1 })

static inline const char* __attribute__((const, unused, warn_unused_result))
string(string_t s1) {
  return s1.val;
}

static inline bool_t __attribute__((const, unused, warn_unused_result))
string_eq(string_t s1, string_t s2) {
  return (s1.len == s2.len && !memcmp(s1.val, s2.val, s1.len)) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// parray_t

#include "iter.h"

#define PARRAY_DECL(NAME, TYPE)			\
  struct NAME { size_t len; TYPE* inner; }

#define parray_t(TYPE)				\
  struct { size_t len; TYPE* inner; }

#define parray_new(PTR, LEN)			\
  { .len = (LEN), .inner = (PTR) }

#define parray_raw(PARRAY)			\
  ((PARRAY)->inner)

#define parray_len(PARRAY)			\
  ((PARRAY)->len)

#define parray_get(PARRAY, IDX)			\
  (parray_raw((PARRAY))[(IDX)])

#define parray_move(PARRAY)			\
  { .len = parray_len((PARRAY)), .inner = parray_raw((PARRAY)) }

#define parray_foreach(VAR, PARRAY)					\
  array_foreach(VAR, parray_len((PARRAY)), parray_raw((PARRAY)))

#define parray_foreach_rev(VAR, PARRAY)					\
  array_foreach_rev(VAR, parray_len((PARRAY)), parray_raw((PARRAY)))

#define parray_idx_foreach(VAR, VAR_IDX, PARRAY)			\
  array_foreach_rev(VAR, VAR_IDX, parray_len((PARRAY)), parray_raw((PARRAY)))

