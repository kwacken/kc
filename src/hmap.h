#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// hmap.h - A generic hashmap structure, using a robinhood algorithm.
//
////////////////////////////////////////////////////////////////////////////////
//
// TODO
//  - It would be nice if this was faster and still generic... Lack of templates
//    sucks for hashmaps.

#include "common.h"

typedef size_t (*hash_function_t)(const void*, size_t key_len);
typedef bool_t (*key_eq_function_t)(const void*, const void*, size_t key_len);

#define hmap_new(KEY_TYPE, HASH_FUN, KEY_EQ_FUN)	\
  hmap_new_(sizeof(KEY_TYPE), (HASH_FUN), (KEY_EQ_FUN))
struct hmap hmap_new_(size_t key_len,
		      hash_function_t hash_fun,
		      key_eq_function_t key_eq_fun);

#define hmap_new_reserve(KEY_TYPE, HASH_FUN, KEY_EQ_FUN, SLOTS)	\
  hmap_new_reserve_(sizeof(KEY_TYPE), (HASH_FUN), (KEY_EQ_FUN), (SLOTS))
struct hmap hmap_new_reserve_(size_t key_len,
			      hash_function_t hash_fun,
			      key_eq_function_t key_eq_fun,
			      size_t slots);

void hmap_reserve(struct hmap*, size_t slots);

// Returns current value if key already present (does not overwrite)
// NULL if inserted successfully
void* hmap_insert(struct hmap*, void* key, void* val);

// NULL if not present
void* hmap_get(struct hmap*, const void* key);
// NULL if not present, non-null if successfully erased.
void* hmap_erase(struct hmap*, const void* key);

// Returns TRUE to exit prematurely
typedef bool_t (*hmap_foreach_function_t)(void* key,
					  size_t key_len,
					  void* val,
					  void* arg);

// Returns TRUE if exited prematurely
bool_t hmap_foreach(struct hmap*, hmap_foreach_function_t, void* arg);

void hmap_destroy(struct hmap*);

////////////////////////////////////////////////////////////////////////////////

size_t murmur_hash(const void* key, size_t key_len, size_t seed);

__attribute__((unused))
static inline size_t default_hash(const void* key, size_t key_len) {
  return murmur_hash(key, key_len, 0);
}

__attribute__((unused))
static inline bool_t default_key_eq(const void* lhs, const void* rhs, size_t key_len) {
  return memcmp(lhs, rhs, key_len) == 0;
}

__attribute__((unused))
static inline size_t empty_hash(const void* key, size_t key_len) {
  (void)key_len;
  return *(size_t*)key;
}

__attribute__((unused))
static inline bool_t empty_key_eq(const void* lhs, const void* rhs, size_t key_len) {
  (void)key_len;
  return *(size_t*)lhs == *(size_t*)rhs;
}

__attribute__((unused))
static inline size_t string_hash(const void* str, size_t char_size) {
  (void) char_size;
  return murmur_hash(str, strlen(str), 0);
}

__attribute__((unused))
static inline bool_t string_key_eq(const void* lhs, const void* rhs, size_t char_size) {
  (void) char_size;
  return !strcmp(lhs, rhs);
}

////////////////////////////////////////////////////////////////////////////////
// Private

struct hmap_bucket;

#define hmap

struct hmap {
  uint8_t slot_bound;
  size_t num_items;
  size_t key_len;
  hash_function_t hash_fun;
  key_eq_function_t key_eq_fun;
  struct hmap_bucket* buckets;
};

////////////////////////////////////////////////////////////////////////////////




