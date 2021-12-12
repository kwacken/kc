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
  ((string_t) { .val = (CST), .len = raw_array_size((CST)) - 1 })

static inline bool_t __attribute__((const, unused, warn_unused_result))
string_eq(string_t s1, string_t s2) {
  return (s1.len == s2.len && !memcmp(s1.val, s2.val, s1.len)) ? TRUE : FALSE;
}
