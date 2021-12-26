#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// util.h - Miscellaneous utility functions.
//
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "type.h"

// Get length of a built-in array type.
#define array_len(A) ((sizeof(A)) / (sizeof(A[0])))

// Get maximum of two comparible values.
#define max(A, B) ({				\
  typeof(A) __max_a = (A);			\
  typeof(A) __max_b = (B);			\
  (__max_a > __max_b) ? __max_a : __max_b;	\
})

// Get minimum of two comparible values.
#define min(A, B) ({				\
  typeof(A) __min_a = (A);			\
  typeof(A) __min_b = (B);			\
  (__min_a > __min_b) ? __min_b : __min_a;	\
})

// Swap the values of two references.
#define swap(A, B)				\
  do {						\
    typeof(A) __swap_a = (A);			\
    typeof(A) __swap_b = (B);			\
    __auto_type __swap_tmp = *__swap_a;		\
    *__swap_a = *__swap_b;			\
    *__swap_b = __swap_tmp;			\
  } while (0)

// Allocate memory using system allocator.
#define sys_malloc(TYPE)			\
  (cast(pointer(TYPE), (malloc(sizeof(TYPE)))))

#define sys_malloc_flex(TYPE, SIZE)				\
  (cast(pointer(TYPE), (malloc(sizeof(TYPE) + (SIZE)))))

// Allocate memory using system allocator.
#define sys_malloc_array(TYPE, COUNT)			\
  (cast(pointer(TYPE), (malloc(sizeof(TYPE) * (COUNT)))))

// Allocate memory using system allocator and zero it.
#define sys_calloc(TYPE)			\
  (cast(pointer(TYPE), (calloc(sizeof(TYPE)))))

// Allocate memory using system allocator and zero it.
#define sys_calloc_array(TYPE, COUNT)			\
  (cast(pointer(TYPE), (calloc(sizeof(TYPE) * (COUNT)))))

// Allocate aligned memory using system allocator.
#define sys_aligned_malloc(TYPE)				\
  (cast(pointer(TYPE), (aligned_alloc(alignof(TYPE), sizeof(TYPE)))))

// Allocate aligned memory using system allocator.
#define sys_aligned_malloc_array(TYPE, COUNT)				\
  (cast(pointer(TYPE), (aligned_alloc(alignof(TYPE), sizeof(TYPE) * (COUNT)))))

// Reallocate aligned memory using system allocator.
#define sys_realloc(TYPE, PTR)			\
  (cast(pointer(TYPE), (realloc((PTR), sizeof(TYPE)))))

// Reallocate aligned memory using system allocator.
#define sys_realloc_array(TYPE, PTR, COUNT)			\
  (cast(pointer(TYPE), (realloc((PTR), sizeof(TYPE) * (COUNT)))))

// Free memory allocated by system allocator.
#define sys_free(PTR)				\
  (free(PTR))

