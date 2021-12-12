#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// error.h - Simple error handling type and utilities.
//
// The basic interface of this header relies on "subtyping" the struct error
// type. This is done by defining an error subtype as follows:
//
// struct my_error {
//   enum { MY_ERROR0 = ERROR_FIRST, MY_ERROR1, MY_ERROR2, ... } tag;
//   ...
// };
//
// This allows non-error status to be represented by a tag of 0, ERROR_NO,
// created with the error_no macro.
//
////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//  - Create a printable and castable version for use with setjmp.
//  - Add setjmp capabilities.

// Create a non-error error value of type TYPE.
#define error_no(TYPE)				\
  ((struct TYPE) { .tag = ERROR_NO })

// Evaluate an expression which evals to an error value, return from surrounding
// function if error.
/////
// struct my_error foo() { try_ret(may_error()); ... }
#define try_ret(...)					\
  do {							\
    union error_all __possible_err = (__VA_ARGS__);	\
    if (is_error(&(ERR))) return (ERR);			\
  } while (0)

// Evaluate an expression which evals to an error value assigned to ERR, jump to
// LABEL if it is an error.
/////
// struct my_error foo() {
//   struct my_error err = error_no(my_error);
//   struct contains_resource x = ...;
//   try_jmp(err, cleanup, may_error());
//   // ...
//   cleanup:
//   free_resource(x);
//   return err;
// }
/////
#define try_jmp(ERR, LABEL, ...)		\
  do {						\
    (ERR) = (__VA_ARGS__);			\
    if (is_error(&(ERR))) goto LABEL;		\
  } while (0)


// Converts an error type into the error supertype.
static inline union error_all as_error(void*);

// Checks if the error value represents an error.
static inline bool_t is_error(void* err);

////////////////////////////////////////////////////////////////////////////////
// Private

#include "base.h"

struct error {
  enum { ERROR_NO = 0, ERROR_FIRST = 1 } tag;
};

union error_all {
  void* any;
  struct error* error;
};

////////////////////////////////////////////////////////////////////////////////


static inline union error_all __attribute__((const, unused, always_inline))
as_error(void* v) {
  return (union error_all) { .any = v };
}

static inline bool_t __attribute__((const, unused, always_inline))
is_error(void* err) {
  return as_error(err).error->tag != ERROR_NO;
}


