#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// type.h - Useful macros for manipulating types.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdalign.h>

// Get TYPE as a pointer type.
#define pointer(TYPE)				\
  typeof(TYPE *)

// Get a (null) pointer value of TYPE.
#define pointer_val(TYPE)			\
  ((pointer(TYPE)) NULL)

// Get TYPE as the type of an array of length LEN.
#define array(TYPE, LEN)			\
  typeof(TYPE [ LEN ])

// Cast VAL to TYPE.
#define cast(TYPE, VAL)				\
  ((TYPE) (VAL))

#define new(TYPE, ...)				\
  ((TYPE) { __VA_ARGS__ })

#define as_bytes(PTR)				\
  (cast(pointer(const char), (PTR)))

// Get the type of a member of TYPE.
#define member_typeof(TYPE, MEMBER)		\
  typeof(pointer_val(TYPE)->MEMBER)

// Get the type of the ARRAY_TYPE's elements.
#define array_elem_typeof(ARRAY_TYPE)		\
  typeof(**(pointer_val(ARRAY_TYPE)))

// Get the size of a member of TYPE.
#define member_sizeof(TYPE, MEMBER)		\
  (sizeof(member_typeof(TYPE, MEMBER)))

// Get the size of the ARRAY_TYPE's elements.
#define array_elem_sizeof(ARRAY_TYPE)		\
  (sizeof(parray_elem_typeof(ARRAY_TYPE)))

// Get pointer to base TYPE from MEMBER PTR.
#define containerof(PTR, TYPE, MEMBER) ({				\
  const pointer(member_typeof(TYPE, MEMBER)) __member_ptr = (PTR);	\
  cast(TYPE, (as_bytes(__member_ptr) - offsetof(TYPE, MEMBER)));	\
})
