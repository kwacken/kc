#include "region.h"

#include <stdlib.h>

#include "list.h"

////////////////////////////////////////////////////////////////////////////////

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

static inline char*
r_block_free_ptr(struct r_block*);

////////////////////////////////////////////////////////////////////////////////

static inline char*
r_block_free_ptr(struct r_block* block) {
  return &block->bytes[REAL_BLOCK_SIZE - block->num_free];
}

region_t r_create() {
  region_t new = malloc(sizeof(struct region));
  slist_init(&new->blocks);
  slist_init(&new->oversized);
  slist_init(&new->structs);
  slist_init(&new->subs);
  return new;
}

void* __r_malloc_(region_t region, size_t bytes) {
  struct r_block* curr;
  if (unlikely(bytes > REAL_BLOCK_SIZE)) {
    curr = malloc(bytes + sizeof(struct r_block));
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
      curr = malloc(REGION_BLOCK_SIZE);
      slist_insert(&region->blocks, curr, slist);
      curr->num_free = REAL_BLOCK_SIZE;
    }

    void* result = r_block_free_ptr(curr);
    curr->num_free -= bytes;
    return result;
  }
}

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
    free(block);
  }

  while (!slist_is_empty(&region->blocks)) {
    slist_pop(&region->blocks, block, slist);
    free(block);
  }

  free(region);
}

void* __r_add_struct(region_t region,
		     r_generic_destructor_t dstr,
		     size_t bytes) {
  struct r_struct* ds = r_malloc_flex(region, struct r_struct, data, bytes);
  slist_insert(&region->structs, ds, slist);
  ds->dstr = dstr;
  return ds->data;
}


void __r_add_subregion_(region_t region, int tag, region_t sub) {
  struct r_sub_region* new;
  r_malloc_init(region, new) {
    .tag = tag,
    .region = sub,
  };
  slist_insert(&region->subs, new, slist);
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
