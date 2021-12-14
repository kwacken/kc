#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// vec.h - A generic growable array implementation.
//
////////////////////////////////////////////////////////////////////////////////


// Declare a growable array NAME for elements TYPE.
#define VEC_DECL(NAME, TYPE)				\
  struct NAME { size_t len, cap, elm_size; TYPE* vec; }

// Get the raw pointer to underlying array.
#define vec_raw(VEC)				\
  ((VEC)->vec)

// Allocate a new vec of VEC_TYPE with initial capacity CAP.
#define vec_new_w_cap(VEC_TYPE, CAP)					\
  ((VEC_TYPE) {	.len = 0, .cap = (CAP),					\
     .elm_size = sizeof(array_elem_typeof(member_typeof(VEC_TYPE, vec))), \
     .vec = ((CAP) != 0)						\
     ? malloc(sizeof(array_elem_typeof(member_typeof(VEC_TYPE, vec))) * (CAP)) \
     : NULL })

// Allocate a new vec of the type.
#define vec_new(VEC_TYPE) vec_new_w_cap(VEC_TYPE, 8)

// Reserve CAP spaces for elements in the vec.
#define vec_reserve(VEC, CAP)					\
  do {								\
    __vec_reserve_((struct __vec_internal*)(VEC), (CAP));	\
  } while (0)

// Destroy a vec, freeing its underlying array.
#define vec_destroy(VEC) do {				\
    __vec_destroy((struct __vec_internal*)(VEC));	\
  } while (0)

// Put a value VAL at the end of the vec.
#define vec_push(VEC, VAL)						\
  do {									\
    typeof(*vec_raw(VEC)) __push_val = (VAL);				\
    __vec_push_((struct __vec_internal*)(VEC), (char*)(&__push_val));	\
  } while (0)

// Put a value, referenced by VAL_REF, at the end of the vec.
#define vec_push_ref(VEC, VAL_REF)					\
  do {									\
    typeof(vec_raw(VEC)) __push_val = (VAL_REF);			\
    __vec_push_((struct __vec_internal*)(VEC), (char*)(__push_val));	\
  } while (0)

// Get the number of elements in the vec.
#define vec_len(VEC)				\
  __vec_len_((struct __vec_internal*)(VEC))

// Get the element at the index IDX.
#define vec_get(VEC, IDX)			\
  (vec_raw((VEC))[(IDX)])

// Get a pointer to the last element in the vec.
#define vec_back(VEC)				\
  (&vec_get((VEC), vec_len((VEC)) - 1))

// Remove the last element of the vec.
#define vec_vpop(VEC)				\
  do {						\
    __vec_vpop_((struct __vec_internal*)(VEC));	\
  } while (0)

// Remove the last element of the vec, storing its value at ELM_POINTER.
#define vec_pop(VEC, ELM_PTR)				\
  do {							\
    *(ELM_PTR) = vec_get((VEC), vec_len((VEC)) - 1);	\
    vec_vpop((VEC));					\
  } while (0)

// Clear the vec, removing all elements.
#define vec_clear(VEC)					\
  do {							\
    __vec_clear_((struct __vec_internal*)(VEC));	\
  } while (0)

// Iterate over the elements of the vec.
#define vec_foreach(VAR, VEC)					\
  array_foreach((VAR), vec_len((VEC)), vec_raw((VEC)))

// Iterate over the elements of the vec in reverse.
#define vec_foreach_rev(VAR, VEC)				\
  array_foreach_rev((VAR), vec_len((VEC)), vec_raw((VEC)))

// Iterate over the elements of the vec, with their index in VAR_IDX.
#define vec_idx_foreach(VAR, VAR_IDX, VEC)			\
  array_idx_foreach((VAR), (VAR_IDX), vec_len((VEC)), vec_raw((VEC)))

// Iterate over the elements of the array, in reverse, with their index.
#define vec_idx_foreach_rev(VAR, VAR_IDX, VEC)				\
  array_idx_foreach_rev((VAR), (VAR_IDX), vec_len((VEC)), vec_raw((VEC)))


// Create a new vec stored in region REG with capacity CAP.
#define r_new_vec_w_cap(REG, VEC_TYPE, CAP)				\
  ({ VEC_TYPE* __ret =							\
      r_new_struct((REG), (r_generic_destructor_t)__vec_destroy, VEC_TYPE); \
    vec_init_w_cap(__ret, (CAP));					\
    __ret; })

// Create a new vec stored in region REG.
#define r_new_vec(REG, VEC_TYPE) r_new_vec_w_cap(REG, VEC_TYPE, 8)

////////////////////////////////////////////////////////////////////////////////
// Private

#include "common.h"
#include <stdlib.h>

VEC_DECL(__vec_internal, char);

////////////////////////////////////////////////////////////////////////////////

static inline void __attribute__((unused))
__vec_push_(struct __vec_internal*, char*);
static inline void __attribute__((unused, always_inline))
__vec_vpop_(struct __vec_internal*);
static inline void __attribute__((unused, always_inline))
__vec_reserve_(struct __vec_internal*, size_t);
static inline void __attribute__((unused, always_inline))
__vec_destroy(struct __vec_internal*);

////////////////////////////////////////////////////////////////////////////////

static inline void __attribute__((unused, always_inline))
__vec_clear_(struct __vec_internal* vec) {
  vec->len = 0;
}

static inline size_t __attribute__((unused, always_inline))
__vec_len_(struct __vec_internal* vec) {
  return vec->len;
}

static inline void __attribute__((unused))
__vec_push_(struct __vec_internal* vec, char* val) {
  if (vec->cap <= vec->len) {
    vec->cap += max(8, vec->cap * 1.6);
    vec->vec = realloc(vec->vec, vec->elm_size * vec->cap);
  }
  memcpy(((char*)vec->vec) + (vec->len++ * vec->elm_size),
	 val,
	 vec->elm_size);
}

static inline void __attribute__((unused, always_inline))
__vec_vpop_(struct __vec_internal* vec) {
  vec->len--;
}

static inline void __attribute__((unused, always_inline))
__vec_reserve_(struct __vec_internal* vec, size_t cap) {
  if (vec->cap < cap) {
    vec->cap = cap;
    vec->vec = realloc(vec->vec, vec->elm_size * vec->cap);
  }
}

static inline void __attribute__((unused, always_inline))
__vec_destroy(struct __vec_internal* vec) {
  if (vec->vec != NULL) {
    free(vec->vec);
  }
}
