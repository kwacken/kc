#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// contract.h - Assertion functions.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include "type.h"

// Declare code path unreachable, fail in debug mode.
#ifndef NDEBUG
#define UNREACHABLE assertf(false, "Unreachable"); __builtin_unreachable()
#else
#define UNREACHABLE __builtin_unreachable()
#endif

// Declare code path incomplete.
#define INCOMPLETE							\
  fprintf(stderr, "Unfinished code (%s:%d)\n", __FILE__, __LINE__);	\
  exit(1)

// Assert a condition with the given failure message.
/////
// assertf(x > 3, "Expected %d > 3", x);
#ifndef NDEBUG
#define assertf(COND, MSG, ...)						\
  do {									\
    bool __cond = (COND);						\
    if (!__cond) {							\
      fprintf(stderr, "Assertion failed at %s : %d:\n", __FILE__, __LINE__); \
      fprintf(stderr, MSG "\n", ##__VA_ARGS__);				\
      exit(1); }							\
  } while (0)

#else
#define assertf(COND, MSG, ...) (void)0
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
