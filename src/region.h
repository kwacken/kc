#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// region.h - A region allocator with subregions and destructors.
//
// Paramaters: REGION_BLOCK_SIZE
//
////////////////////////////////////////////////////////////////////////////////

#include "contract.h"
#include "basic.h"
#include "util.h"

// Default block size.
#ifndef REGION_BLOCK_SIZE
#define REGION_BLOCK_SIZE 2048
#endif

// Handle for a region.
typedef struct region* region_t;

// Create a fresh region.
static inline
region_t r_create();

// Allocate memory of type TYPE within the region REG.
#define r_malloc(REG, TYPE)			\
  (cast(pointer(TYPE), __r_malloc_((REG), sizeof(TYPE))))

// Allocate LEN bytes within the region REG.
#define r_malloc_bytes(REG, LEN)		\
  __r_malloc_((REG), (LEN))

// Allocate string inside region REG.
#define r_malloc_string(REG, STR)				\
  string(memcpy(r_malloc_bytes((REG), string_len((STR))),	\
		string_raw((STR)),				\
		string_len((STR))), string_len((STR)))

// Allocate memory of type TYPE, with flexible FIELD having COUNT elements.
////
// struct foo { int first; int rest[] };
// r_malloc_flex(r, struct foo, rest, 10);
#define r_malloc_flex(REG, TYPE, FIELD, COUNT)				\
  (cast(pointer(TYPE),							\
	__r_malloc_(							\
	  (REG),							\
	  (sizeof(TYPE)							\
	   + (sizeof(array_elem_typeof(member_typeof(TYPE, FIELD)))	\
	      * (COUNT))))))

// Allocate an array of TYPE with COUNT elements.
////
// parray_t(int) int_array; r_malloc_array(r, int, 5);
#define r_malloc_parray(REG, TYPE, COUNT)				\
  parray_new(__r_malloc_((REG), (sizeof(TYPE) * (COUNT))), (COUNT))

// Allocate into the pointer PTR and initialize the memory.
/////
// struct foo* new; r_malloc_init(r, new, .field = ... );
#define r_malloc_init(REG, PTR, ...)		\
  (PTR) = r_malloc((REG), typeof(*(PTR)));	\
  *(PTR) = new(typeof(*(PTR)), __VA_ARGS__)

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
static inline
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
  __r_get_subregion_((REG), (int)(TAG))

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
#define r_new_struct(REG, DESTRUCTOR, TYPE)				\
  (cast(pointer(TYPE), __r_add_struct((REG), (DESTRUCTOR), sizeof(TYPE))))

////////////////////////////////////////////////////////////////////////////////
// Private

#include "list.h"
#include <stdlib.h>

struct region;

SLIST_DECL(r_block_slist, struct r_block);
struct r_block {
  SLIST_ENTRY(struct r_block) slist;

  size_t num_free;
  char bytes[];
};

SLIST_DECL(r_struct_slist, struct r_struct);
struct r_struct {
  SLIST_ENTRY(struct r_struct) slist;
  r_generic_destructor_t dstr;
  char data[];
};

SLIST_DECL(r_sub_region_slist, struct r_sub_region);
struct r_sub_region {
  SLIST_ENTRY(struct r_sub_region) slist;
  int tag;
  region_t region;
};

struct region {
  struct r_block_slist blocks;
  struct r_block_slist oversized;
  struct r_struct_slist structs;
  struct r_sub_region_slist subs;
};

////////////////////////////////////////////////////////////////////////////////

static const size_t REAL_BLOCK_SIZE =
  (REGION_BLOCK_SIZE - sizeof(struct r_block));

////////////////////////////////////////////////////////////////////////////////

static inline char* __attribute__((always_inline))
r_block_free_ptr(struct r_block*);

static inline void* __attribute__((malloc, warn_unused_result, nonnull, unused))
__r_malloc_(region_t, size_t);

static inline void __attribute__((unused))
__r_add_subregion_(region_t, int, region_t);
static inline region_t __attribute__((warn_unused_result, nonnull, unused))
__r_get_subregion_(region_t, int);
static inline region_t __attribute__((warn_unused_result, nonnull, unused))
__r_extract_subregion_(region_t, int);

static inline void* __attribute__((malloc, warn_unused_result, nonnull, unused))
__r_add_struct(region_t, r_generic_destructor_t, size_t);

////////////////////////////////////////////////////////////////////////////////

static inline char*
r_block_free_ptr(struct r_block* block) {
  return &block->bytes[REAL_BLOCK_SIZE - block->num_free];
}

static inline region_t __attribute__((malloc, warn_unused_result, unused))
r_create() {
  region_t res = sys_malloc(struct region);
  slist_init(&res->blocks);
  slist_init(&res->oversized);
  slist_init(&res->structs);
  slist_init(&res->subs);
  return res;
}

static inline
void* __r_malloc_(region_t region, size_t bytes) {
  struct r_block* curr;
  if (unlikely(bytes > REAL_BLOCK_SIZE)) {
    curr = sys_malloc_flex(struct r_block, bytes);
    slist_insert(&region->oversized, curr, slist);
    curr->num_free = 0;
    return curr->bytes;
  } else {
    slist_foreach(curr, &region->blocks, slist) {
      if (bytes <= curr->num_free) {
	break;
      }
    }

    if (curr == NULL) {
      curr = sys_malloc_flex(struct r_block, REAL_BLOCK_SIZE);
      slist_insert(&region->blocks, curr, slist);
      curr->num_free = REAL_BLOCK_SIZE;
    }

    void* result = r_block_free_ptr(curr);
    curr->num_free -= bytes;
    return result;
  }
}

static inline
void r_destroy(region_t region) {
  struct r_sub_region* sub;
  slist_foreach(sub, &region->subs, slist) {
    r_destroy(sub->region);
  }

  struct r_struct* ds;
  slist_foreach(ds, &region->structs, slist) {
    ds->dstr(ds->data);
  }

  struct r_block* block;
  while (!slist_is_empty(&region->oversized)) {
    slist_pop(&region->oversized, block, slist);
    sys_free(block);
  }

  while (!slist_is_empty(&region->blocks)) {
    slist_pop(&region->blocks, block, slist);
    sys_free(block);
  }

  sys_free(region);
}

static inline void*
__r_add_struct(region_t region, r_generic_destructor_t dstr, size_t bytes) {
  struct r_struct* ds = r_malloc_flex(region, struct r_struct, data, bytes);
  slist_insert(&region->structs, ds, slist);
  ds->dstr = dstr;
  return ds->data;
}


void __r_add_subregion_(region_t region, int tag, region_t sub) {
  struct r_sub_region* res;
  r_malloc_init(region, res, .tag = tag, .region = sub);
  slist_insert(&region->subs, res, slist);
}

region_t __r_get_subregion_(region_t region, int tag) {
  struct r_sub_region* sub;
  slist_foreach(sub, &region->subs, slist) {
    if (sub->tag == tag) {
      return sub->region;
    }
  }

  UNREACHABLE;
}

region_t __r_extract_subregion_(region_t region, int tag) {
  struct r_sub_region** sub;
  struct r_sub_region* extracted;
  slist_mut_foreach(sub, &region->subs, slist) {
    if ((*sub)->tag == tag) {
      extracted = *sub;
      slist_remove(sub, slist);
      return extracted->region;
    }
  }

  UNREACHABLE;
}

