#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// common.h - Common utilities expected everywhere.
//
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// Get type of a member of a type.
/////
// Takes the base type as a non-pointer type.
#define member_typeof(TYPE, MEMBER)		\
  typeof(((TYPE*) 0)->MEMBER)

// Get size of a member of a type.
/////
// Takes the base type as a non-pointer type.
#define member_sizeof(TYPE, MEMBER)		\
  sizeof(((TYPE*) 0)->MEMBER)

// Get pointer to base type from pointer to member field.
/////
// Takes the base type as a non-pointer type.
#define containerof(PTR, TYPE, MEMBER)					\
  ({ const member_typeof(TYPE, MEMBER)* __member_ptr = (PTR);		\
    (TYPE *)((const char *)__member_ptr - offsetof(TYPE, MEMBER)); })

// Get the type of the elements of an array.
/////
// array_elem_typeof(int*[30]) // => int*
#define array_elem_typeof(ARRAY_TYPE)		\
  typeof(**((ARRAY_TYPE*) 0))

// Get maximum of two comparible values.
#define max(A, B) (((A) > (B)) ? (A) : (B))

// Get minimum of two comparible values.
#define min(A, B) (((A) > (B)) ? (B) : (A))

// Declare code path unreachable, fail in debug mode.
#ifndef NDEBUG
#define UNREACHABLE assertf(FALSE, "Unreachable")
#else
#define UNREACHABLE __builtin_unreachable()
#endif

// Declares a condition likely to occur.
#define likely(COND) __builtin_expect((COND), 1)

// Declares a condition likely to occur.
#define unlikely(COND) __builtin_expect((COND), 0)

// Declares a condition likely to occur with [0.0, 1.0] probability.
#define likely_with(COND, PROB)				\
  __builtin_expect_with_probability((COND), 1, (PROB))

// Declares a condition unlikely to occur with [0.0, 1.0] probability.
#define unlikely_with(COND, PROB)			\
  __builtin_expect_with_probability((COND), 0, (PROB))

// Get length of a built-in array type.
#define array_len(A) ((sizeof(A)) / (sizeof(A[0])))


#ifndef NDEBUG

// Assert a condition with the given failure message.
/////
// assertf(x > 3, "Expected %d > 3", x);
#define assertf(COND, MSG, ...)						\
  do {									\
    if (!(COND)) {							\
      fprintf(stderr, "Assertion failed: " MSG "\n", ##__VA_ARGS__);	\
      assert(COND); }							\
  } while (0)

#else
#define assertf(COND, MSG, ...) (void) 0
#endif

#include "basic.h"
#include "iter.h"
