#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// region.h - A region allocator with subregions and destructors.
//
// Paramaters: REGION_BLOCK_SIZE
//
////////////////////////////////////////////////////////////////////////////////

// Default block size.
#ifndef REGION_BLOCK_SIZE
#define REGION_BLOCK_SIZE 2048
#endif

// Handle for a region.
typedef struct region* region_t;

// Create a fresh region.
__attribute__((warn_unused_result))
region_t r_create();

// Allocate memory of type TYPE within the region REG.
#define r_malloc(REG, TYPE)			\
  __r_malloc_((REG), sizeof(TYPE))

// Allocate LEN bytes within the region REG.
#define r_malloc_bytes(REG, LEN)		\
  __r_malloc_((REG), (LEN))

// Allocate memory of type TYPE, with flexible FIELD having COUNT elements.
////
// struct foo { int first; int rest[] };
// r_malloc_flex(r, struct foo, rest, 10);
#define r_malloc_flex(REG, TYPE, FIELD, COUNT)				\
  __r_malloc_((REG),							\
	      (sizeof(TYPE)						\
	       + (sizeof(array_elem_typeof(member_typeof(TYPE, FIELD))) \
		  * (COUNT))))

// Allocate an array of TYPE with COUNT elements.
////
// int* int_array; r_malloc_array(r, int, 5);
#define r_malloc_array(REG, TYPE, COUNT)	\
  __r_malloc_((REG), (sizeof(TYPE) * (COUNT)))

// Allocate into the pointer PTR and initialize the memory.
/////
// struct foo* new; r_malloc_init(r, new) { .field = ... };
#define r_malloc_init(REG, PTR)			\
  (PTR) = r_malloc((REG), typeof(*(PTR)));	\
  *(PTR) = (typeof(*(PTR)))

// Create a region and allocate its container inside.
////
// struct { region_t r; ... }* container; r_create_container(container, r);
#define r_create_container(CONTAINER, FIELD)			\
  do {								\
    region_t __r_new_reg = r_create();				\
    (CONTAINER) = r_malloc(__r_new_reg, typeof(*(CONTAINER)));	\
    (CONTAINER)->FIELD = __r_new_reg;				\
  } while (0)

// Destroy a region, freeing all memory, subregions, and structures.
void r_destroy(region_t);

////////////////////////////////////////////////////////////////////////////////
// Subregions
//
// Subregions are regions which are contained with another region. They are
// destroyed when the parent region is destroyed, unless the are extracted first.

// Create a subregion with tag TAG (int) in region REG.
/////
// Returns the handle of the subregion.
#define r_create_subregion(REG, TAG)			\
  ({ region_t __new_subregion = r_create();		\
    r_add_subregion((REG), (TAG), __new_subregion);	\
    __new_subregion; })

// Register an existing region as a subregion.
#define r_add_subregion(REG, TAG, SUBREG)		\
  __r_add_subregion_((REG), (int)(TAG), (SUBREG))

// Retrieve the handle a subregion with tag TAG.
#define r_get_subregion(REG, TAG)		\
  __r_get_subregion_((REG), (int)(TAG));

// Remove a subregion from REG and return its handle.
#define r_extract_subregion(REG, TAG)		\
  __r_extract_subregion_((REG), (int)(TAG))

////////////////////////////////////////////////////////////////////////////////
// Structures
//
// Structures are data in the region with an attached destructor. The destructor
// is invoked upon the destruction of the region.

typedef void (*r_generic_destructor_t)(void*);

// Allocate memory of type TYPE, with destructor DESTRUCTOR.
#define r_new_struct(REG, DESTRUCTOR, TYPE)		\
  __r_add_struct((REG), (DESTRUCTOR), sizeof(TYPE))

////////////////////////////////////////////////////////////////////////////////
// Private

#include "common.h"

struct region;

void* __attribute__((malloc, warn_unused_result, nonnull))
__r_malloc_(region_t, size_t);
void* __attribute__((malloc, warn_unused_result, nonnull))
__r_add_struct(region_t, r_generic_destructor_t, size_t);

void __r_add_subregion_(region_t, int, region_t);
region_t __attribute__((warn_unused_result, nonnull))
__r_get_subregion_(region_t, int);
region_t __attribute__((warn_unused_result, nonnull))
__r_extract_subregion_(region_t, int);
