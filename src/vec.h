#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// vec.h - A generic growable array implementation.
//
////////////////////////////////////////////////////////////////////////////////

#include "type.h"
#include "util.h"
#include "basic.h"

// Declare a growable array NAME for elements TYPE.
#define VEC_DECL(NAME, TYPE)				\
  struct NAME { size_t cap; parray_t(TYPE) parray; }

#define vec_t(TYPE)				\
  VEC_DECL(, TYPE)

// Get pointer to the raw parray of the vector.
#define vec_parray(VEC)				\
  ((VEC)->parray)

// Get underlying pointer of the vector.
#define vec_raw(VEC)				\
  (parray_raw(&vec_parray(VEC)))

// Get the number of elements in the vec.
#define vec_len(VEC)				\
  (parray_len(&vec_parray(VEC)))

// Get the capacity of the vec.
#define vec_cap(VEC)				\
  ((VEC)->cap)

// Allocate a new vec of VEC_TYPE with initial capacity CAP.
#define vec_new_w_cap(VEC_TYPE, CAP) ({					\
  size_t __cap = (CAP);						        \
  member_typeof(VEC_TYPE, parray) __parray;				\
  typeof(parray_raw(&__parray)) __ptr =	(__cap == 0)			\
    ? NULL : sys_aligned_malloc_array(parray_elem_typeof(&__parray), __cap); \
  parray_init(&__parray, __ptr, 0);					\
  new(VEC_TYPE, .cap = __cap, .parray = __parray);			\
})

// Allocate a new vec of the type.
#define vec_new(VEC_TYPE) vec_new_w_cap(VEC_TYPE, 8)

// Destroy a vec, freeing its underlying array.
#define vec_destroy(VEC)			\
  do {						\
    sys_free(vec_raw(VEC));			\
  } while (0)

// Put a value, referenced by PTR, at the end of the vec.
#define vec_push_ref(VEC, PTR)						\
  do {									\
    __auto_type __vec = (VEC);						\
    pointer(parray_elem_typeof(&vec_parray(__vec))) __push_ref = (PTR);	\
    if (vec_cap(__vec) <= vec_len(__vec)) {				\
      vec_cap(__vec) +=							\
	min(8UL, cast(size_t, (cast(double, vec_cap(__vec)) * 1.6)));	\
      __vec_grow(__vec, vec_cap(__vec));				\
    }									\
    memcpy(vec_raw(__vec) + vec_len(__vec) + 1,				\
           __push_ref, parray_elem_sizeof(&vec_parray(__vec)));		\
  } while (0)

// Put a value VAL at the end of the vec.
#define vec_push(VEC, VAL)			\
  do {						\
    __auto_type __vec = (VEC);						\
    parray_elem_typeof(&vec_parray(__vec)) __push_val = (VAL);	\
    vec_push_ref((VEC), &__push_val);		\
  } while (0)

// Get the element at the index IDX.
#define vec_get(VEC, IDX)			\
  (vec_raw((VEC))[(IDX)])

// Get a pointer to the last element in the vec.
#define vec_back(VEC)				\
  (&vec_get((VEC), vec_len((VEC)) - 1))

// Remove the last element of the vec.
#define vec_vpop(VEC)				\
  do {						\
    vec_len((VEC))--;				\
  } while (0)

// Remove the last element of the vec, storing its value at ELM_POINTER.
#define vec_pop(VEC) ({			    \
  __auto_type __vec = (VEC);		    \
  __auto_type __val = *vec_back(__vec);	    \
  vec_vpop(__vec);			    \
  __val;				    \
})

// Clear the vec, removing all elements.
#define vec_clear(VEC)					\
  do {							\
    vec_len((VEC)) = 0;					\
  } while (0)

// Reserve CAP spaces for elements in the vec.
#define vec_reserve(VEC, CAP)						\
  do {									\
    __auto_type __cap = (CAP);						\
    __auto_type __vec = (VEC);						\
    if (vec_cap(__vec) < __cap) __vec_grow(__vec, __cap);		\
  } while (0)


// TODO...

// Return ptr_iter for the vec.
/////
// The lifetime of the ptr_iter is tied to immutable usage of the vec.
#define vec_ptr_iter(VEC, ITER)						\
  do {									\
    parray_ptr_iter(&vec_parray(VEC), (ITER));				\
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
#define r_new_vec_w_cap(REG, VEC_TYPE, CAP) ({				\
  pointer(VEC_TYPE) __ret =						\
    r_new_struct((REG), __vec_generic_destructor, VEC_TYPE);		\
  *__ret = vec_new_w_cap(VEC_TYPE, (CAP));				\
  __ret;								\
})

// Create a new vec stored in region REG.
#define r_new_vec(REG, VEC_TYPE) r_new_vec_w_cap(REG, VEC_TYPE, 8)

////////////////////////////////////////////////////////////////////////////////
// Private

static inline void
__vec_generic_destructor(void* vec);

////////////////////////////////////////////////////////////////////////////////

#define __vec_grow(VEC, CAP)						\
  do {								        \
   vec_cap(VEC) = CAP;					         	\
   member_typeof(typeof(*VEC), parray) __parray;			\
   typeof(parray_raw(&__parray)) __ptr =			        \
     sys_aligned_malloc_array(parray_elem_typeof(&__parray), CAP);	\
   memcpy(__ptr,							\
	  vec_raw(VEC),							\
	  parray_sizeof(&vec_parray(VEC)));				\
   parray_init(&__parray, __ptr, vec_len(VEC));				\
   sys_free(vec_raw(VEC));						\
   parray_init(&vec_parray(VEC),					\
	       parray_raw(&__parray),					\
	       parray_len(&__parray));					\
  } while (0)

static inline void
__vec_generic_destructor(void* vec_) {
  vec_t(char)* vec = cast(typeof(vec), vec_);
  vec_destroy(vec);
}
