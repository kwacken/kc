////////////////////////////////////////////////////////////////////////////////
//
// hmap.h - A generic hashmap structure, using a robinhood algorithm.
//
// All functions on the hashmap have the name form HMAP_NAME ## _<name>.
//
// Parameters:
//   - HMAP_NAME        :: Name of hashmap type
//   - HMAP_KEY_TYPE    :: Type of keys
//   - HMAP VAL_TYPE    :: Type of values (default: undefined, aka hashset)
//   - HMAP_HASH_FUN    :: Hashing function (default: murmur_hash)
//       size_t (*)(const KEY_TYPE*)
//   - HMAP_KEY_EQ      :: Key equality function (default: memcmp)
//       bool_t (*)(const KEY_TYPE*, const KEY_TYPE*)
//   - HMAP_LOAD_FACTOR :: How full the map should be before growing it.
//       float, [0.0, 1.0] (default: 0.9f)
//
////////////////////////////////////////////////////////////////////////////////

#include "common.h"

////////////////////////////////////////////////////////////////////////////////
// Parameters

#ifndef HMAP_NAME
#error "Must provide name for hmap type."
#define HMAP_NAME debug // Debug
#endif

#ifndef HMAP_KEY_TYPE
#error "Must provide type for hmap key."
#define HMAP_KEY_TYPE char // Debug
#endif

#ifndef HMAP_VAL_TYPE
#define HMAP_HASHSET
#endif

#ifndef HMAP_HASH_FUN
// Defaults to murmur hash.
#endif

#ifndef HMAP_KEY_EQ
// Defaults to memcmp comparison.
#endif

#ifndef HMAP_LOAD_FACTOR
#define HMAP_LOAD_FACTOR 0.9f
#endif

#define HMAP__(NS, ID) NS ## _ ## ID
#define HMAP_(NS, ID) HMAP__(NS, ID)
#define HMAP(ID) HMAP_(HMAP_NAME, ID)

// Create a new hashmap.
static inline
struct HMAP_NAME HMAP(new)();

// Create a new hashmap, reserving a number of slots for entries.
static inline
struct HMAP_NAME HMAP(new_reserve)(size_t count);

// Reserve count slots in the hashmap for entries.
static inline
void HMAP(reserve)(struct HMAP_NAME*, size_t count);

// Insert an entry into the map.
/////
// Returns current value if key already present and does NOT overwrite it, NULL
// otherwise.
#ifndef HMAP_HASHSET
HMAP_VAL_TYPE* HMAP(insert)(struct HMAP_NAME*,
			    const HMAP_KEY_TYPE* key,
			    const HMAP_VAL_TYPE* val);
#else
HMAP_KEY_TYPE* HMAP(insert)(struct HMAP_NAME*, const HMAP_KEY_TYPE* key);
#endif

// Get the value for a key.
/////
// Returns NULL if not present.
// Returns the key pointer if hashmap is a hashset.
#ifndef HMAP_HASHSET
static inline
HMAP_VAL_TYPE* HMAP(get)(struct HMAP_NAME*, const HMAP_KEY_TYPE* key);
#else
static inline
HMAP_KEY_TYPE* HMAP(get)(struct HMAP_NAME*, const HMAP_KEY_TYPE* key);
#endif

// Remove an entry from the map.
/////
// Returns TRUE if successfully removed, FALSE if not present.
bool_t HMAP(erase)(struct HMAP_NAME*, const HMAP_KEY_TYPE* key);

// Remove an entry from the map and return value.
/////
// Returns TRUE if successfully removed, FALSE if not present.
#ifndef HMAP_HASHSET
bool_t HMAP(extract)(struct HMAP_NAME*,
		     const HMAP_KEY_TYPE* key,
		     HMAP_VAL_TYPE* out_val);
#endif

// Destroy the map, freeing its resources.
static inline
void HMAP(destroy)(struct HMAP_NAME*);

#ifndef HMAP_HASHSET
typedef bool_t (*HMAP(visitor_fun_t))(const HMAP_KEY_TYPE* key,
				      HMAP_VAL_TYPE* val,
				      void* arg);
#else
typedef bool_t (*HMAP(visitor_fun_t))(const HMAP_KEY_TYPE* key,
				      void* arg);
#endif

// Visit all entries in the map.
/////
// Return TRUE from visitor to prematurely abort.
bool_t HMAP(foreach)(struct HMAP_NAME*, HMAP(visitor_fun_t), void* arg);


////////////////////////////////////////////////////////////////////////////////
// Private

#include <math.h>
#include <stdlib.h>

struct HMAP_NAME {
  parray_t(struct HMAP(_bucket)) buckets;
  size_t num_items;
  uint8_t slot_bound;
};

struct HMAP(_bucket) {
  HMAP_KEY_TYPE key;
#ifndef HMAP_HASHSET
  HMAP_VAL_TYPE val;
#endif
  size_t hash;
  int8_t dist;
};

struct HMAP(_slots) {
  size_t cap;
  int8_t max_dist;
};

////////////////////////////////////////////////////////////////////////////////

#ifndef HMAP_HASHSET
#define HMAP__GET(ENTRY) ((ENTRY)->val)
#define HMAP__RET HMAP_VAL_TYPE
#else
#define HMAP__GET(ENTRY) ((ENTRY)->key)
#define HMAP__RET HMAP_KEY_TYPE
#endif

#ifndef HMAP_HASH_FUN
#include "murmur.h"
static inline size_t __attribute__((always_inline))
HMAP(_hash)(const HMAP_KEY_TYPE* key) {
  return murmur_hash(key, sizeof(HMAP_KEY_TYPE));
}
#define HMAP_HASH_FUN HMAP(_hash)
#endif

#ifndef HMAP_KEY_EQ
static inline bool_t __attribute__((always_inline))
HMAP(_key_eq_default)(const HMAP_KEY_TYPE* lhs,
		      const HMAP_KEY_TYPE* rhs) {
  return memcmp(lhs, rhs, sizeof(HMAP_KEY_TYPE)) == 0;
}
#define HMAP_KEY_EQ HMAP(_key_eq_default)
#endif

typedef size_t (*HMAP(hash_fun_t))(const HMAP_KEY_TYPE*);
typedef bool_t (*HMAP(key_eq_fun_t))(const HMAP_KEY_TYPE*,
				     const HMAP_KEY_TYPE*);

////////////////////////////////////////////////////////////////////////////////

static const HMAP(hash_fun_t) HMAP(_hash_fun) = HMAP_HASH_FUN;
static const HMAP(key_eq_fun_t) HMAP(_key_eq_fun) = HMAP_KEY_EQ;
static const float HMAP(_load_factor) = HMAP_LOAD_FACTOR;

static const struct HMAP(_slots) HMAP(_slot_bounds)[] = {
  { 5lu, 2 }, { 7lu, 3 }, { 11lu, 3 }, { 17lu, 4 }, { 23lu, 5 }, { 37lu, 5 },
  { 53lu, 6 }, { 79lu, 6 }, { 113lu, 7 }, { 163lu, 7 }, { 229lu, 8 },
  { 331lu, 8 }, { 463lu, 9 }, { 653lu, 9 }, { 919lu, 10 }, { 1289lu, 10 },
  { 1811lu, 11 }, { 2539lu, 11 }, { 3557lu, 12 }, { 4987lu, 12 },
  { 6983lu, 13 }, { 9781lu, 13 }, { 13693lu, 14 }, { 19181lu, 14 },
  { 26861lu, 15 }, { 37607lu, 15 }, { 52667lu, 16 }, { 73751lu, 16 },
  { 103289lu, 17 }, { 144611lu, 17 }, { 202471lu, 18 }, { 283463lu, 18 },
  { 396871lu, 19 }, { 555637lu, 19 }, { 777901lu, 20 }, { 1089091lu, 20 },
  { 1524763lu, 21 }, { 2134697lu, 21 }, { 2988607lu, 22 }, { 4184087lu, 22 },
  { 5857727lu, 22 }, { 8200847lu, 23 }, { 11481199lu, 23 }, { 16073693lu, 24 },
  { 22503181lu, 24 }, { 31504453lu, 25 }, { 44106241lu, 25 },
  { 61748749lu, 26 }, { 86448259lu, 26 }, { 121027583lu, 27 },
  { 169438627lu, 27 }, { 237214097lu, 28 }, { 332099741lu, 28 },
  { 464939639lu, 29 }, { 650915521lu, 29 }, { 911281733lu, 30 },
  { 1275794449lu, 30 }, { 1786112231lu, 31 }, { 2500557133lu, 31 },
  { 3500779987lu, 32 }, { 4901092003lu, 32 }, { 6861528851lu, 33 },
  { 9606140399lu, 33 }, { 13448596583lu, 34 }, { 18828035323lu, 34 },
  { 26359249459lu, 35 }, { 36902949287lu, 35 }, { 51664129003lu, 36 },
  { 72329780647lu, 36 }, { 101261692921lu, 37 }, { 141766370143lu, 37 },
  { 198472918207lu, 38 }, { 277862085493lu, 38 }, { 389006919737lu, 39 },
  { 544609687669lu, 39 }, { 762453562751lu, 39 }, { 1067434987873lu, 40 },
  { 1494408983027lu, 40 }, { 2092172576243lu, 41 }, { 2929041606761lu, 41 },
  { 4100658249511lu, 42 }, { 5740921549367lu, 42 }, { 8037290169151lu, 43 },
  { 11252206236863lu, 43 }, { 15753088731613lu, 44 }, { 22054324224277lu, 44 },
  { 30876053913989lu, 45 }, { 43226475479591lu, 45 }, { 60517065671459lu, 46 },
  { 84723891940099lu, 46 }, { 118613448716141lu, 47 },
  { 166058828202599lu, 47 }, { 232482359483653lu, 48 },
  { 325475303277143lu, 48 }, { 455665424588069lu, 49 },
  { 637931594423311lu, 49 }, { 893104232192647lu, 50 },
  { 1250345925069739lu, 50 }, { 1750484295097673lu, 51 },
  { 2450678013136757lu, 51 }, { 3430949218391537lu, 52 },
  { 4803328905748207lu, 52 }, { 6724660468047551lu, 53 },
  { 9414524655266669lu, 53 }, { 13180334517373357lu, 54 },
  { 18452468324322709lu, 54 }, { 25833455654051813lu, 55 },
  { 36166837915672573lu, 55 }, { 50633573081941709lu, 55 },
  { 70887002314718437lu, 56 }, { 99241803240605819lu, 56 },
  { 138938524536848189lu, 57 }, { 194513934351587491lu, 57 },
  { 272319508092222467lu, 58 }, { 381247311329111473lu, 58 },
  { 533746235860756057lu, 59 }, { 747244730205058483lu, 59 },
  { 1046142622287081827lu, 60 }, { 1464599671201914671lu, 60 },
  { 2050439539682680411lu, 61 }, { 2870615355555752471lu, 61 }
};

////////////////////////////////////////////////////////////////////////////////

static inline size_t __attribute__((always_inline))
HMAP(_slot_count)(uint8_t slot_bound);

static inline size_t __attribute__((always_inline))
HMAP(_slot_load_count)(uint8_t slot_bound);

static inline uint8_t __attribute__((always_inline))
HMAP(_find_slot_bound)(size_t slots);

static inline bool_t __attribute__((always_inline))
HMAP(_key_eq)(struct HMAP(_bucket)* lhs, struct HMAP(_bucket)* rhs);

static inline void
HMAP(_grow_to)(struct HMAP_NAME* table, uint8_t slot_bound);

static inline void __attribute__((always_inline))
HMAP(_grow)(struct HMAP_NAME* table);

static inline
HMAP__RET* HMAP(_insert_inner)(struct HMAP_NAME* table,
			       struct HMAP(_bucket) new);

static inline struct HMAP(_bucket)*
HMAP(_find)(struct HMAP_NAME* table, const HMAP_KEY_TYPE* key);

static inline void
HMAP(_remove)(struct HMAP(_bucket)* entry);

////////////////////////////////////////////////////////////////////////////////

static inline size_t __attribute__((always_inline))
HMAP(_slot_count)(uint8_t slot_bound) {
  return HMAP(_slot_bounds)[slot_bound].cap
    + (uint8_t) HMAP(_slot_bounds)[slot_bound].max_dist;
}

static inline size_t __attribute__((always_inline))
HMAP(_slot_load_count)(uint8_t slot_bound) {
  return (size_t) lround((float) HMAP(_slot_count)(slot_bound) * HMAP(_load_factor));
}

static inline uint8_t __attribute__((always_inline))
HMAP(_find_slot_bound)(size_t slots) {
  uint8_t slot_bound;
  range_foreach(slot_bound, 0, array_len(HMAP(_slot_bounds))) {
    if (HMAP(_slot_load_count)(slot_bound) >= slots) {
      return slot_bound;
    }
  }
  return slot_bound;
}

static inline bool_t __attribute__((always_inline))
HMAP(_key_eq)(struct HMAP(_bucket)* lhs, struct HMAP(_bucket)* rhs) {
  return lhs->hash == rhs->hash
      && HMAP(_key_eq_fun)(&lhs->key, &rhs->key);
}

static inline void
HMAP(_init)(struct HMAP_NAME* table, uint8_t slot_bound) {
  size_t slot_count = HMAP(_slot_count)(slot_bound);
  parray_t(struct HMAP(_bucket)) ret =
    parray_new(malloc(sizeof(struct HMAP(_bucket)) * slot_count),
	       slot_count);

  struct HMAP(_bucket)* bucket;
  parray_foreach(bucket, &ret) {
    bucket->dist = -1;
  }
  *table = (struct HMAP_NAME) {
    .buckets = parray_move(&ret),
    .num_items = 0,
    .slot_bound = slot_bound,
  };
}

static inline void
HMAP(_grow_to)(struct HMAP_NAME* table, uint8_t slot_bound) {
  struct HMAP_NAME new;
  HMAP(_init)(&new, slot_bound);

  struct HMAP(_bucket)* entry;
  parray_foreach(entry, &table->buckets) {
    if (entry->dist >= 0) {
      HMAP(_insert_inner)(&new, *entry);
    }
  }

  HMAP(destroy)(table);
  *table = new;
}

static inline void __attribute__((always_inline))
HMAP(_grow)(struct HMAP_NAME* table) {
  return HMAP(_grow_to)(table, table->slot_bound + 1);
}

static inline
HMAP__RET* HMAP(_insert_inner)(struct HMAP_NAME* table,
                               struct HMAP(_bucket) new) {
  size_t index;
  struct HMAP(_bucket)* entry;

insert:
  new.dist = 0;
  index = new.hash % HMAP(_slot_bounds)[table->slot_bound].cap;
  entry = &parray_get(&table->buckets, index);

  for (; entry->dist >= new.dist; ++entry, ++new.dist) {
    if (HMAP(_key_eq)(entry, &new)) {
      return &HMAP__GET(entry);
    }
  }

  if (new.dist == HMAP(_slot_bounds)[table->slot_bound].max_dist) {
    HMAP(_grow)(table);
    goto insert;
  } else if (entry->dist < 0) {
    ++table->num_items;
    *entry = new;
    return NULL;
  }

  swap(entry, &new);

  for (++entry, ++new.dist;; ++entry) {
    if (entry->dist < 0) {
      ++table->num_items;
      *entry = new;
      return NULL;
    } else if (entry->dist < new.dist) {
      swap(entry, &new);
      ++new.dist;
    } else {
      ++new.dist;
      if (new.dist == HMAP(_slot_bounds)[table->slot_bound].max_dist) {
	break;
      }
    }
  }

  HMAP(_grow)(table);
  goto insert;
}

static inline struct HMAP_NAME __attribute__((warn_unused_result))
HMAP(new)() {
  struct HMAP_NAME new;
  HMAP(_init)(&new, 0);
  return new;
}

static inline struct HMAP_NAME __attribute__((warn_unused_result))
HMAP(new_reserve)(size_t slots) {
  struct HMAP_NAME new;
  HMAP(_init)(&new, HMAP(_find_slot_bound)(slots));
  return new;
}

static inline void
HMAP(reserve)(struct HMAP_NAME* table, size_t slots) {
  uint8_t slot_bound = HMAP(_find_slot_bound)(slots);
  if (slot_bound > table->slot_bound) {
    HMAP(_grow_to)(table, slot_bound);
  }
}


HMAP__RET* HMAP(insert)(struct HMAP_NAME* table,
			const HMAP_KEY_TYPE* key
#ifndef HMAP_HASHSET
			, const HMAP_VAL_TYPE* val
#endif
  ) {
  if (table->num_items + 1 > HMAP(_slot_load_count)(table->slot_bound)) {
    HMAP(_grow)(table);
  }
  struct HMAP(_bucket) buck = (struct HMAP(_bucket)) {
    .key = *key,
#ifndef HMAP_HASHSET
    .val = *val,
#endif
    .hash = HMAP(_hash_fun)(key),
    .dist = 0,
  };
  return HMAP(_insert_inner)(table, buck);
}

static inline struct HMAP(_bucket)*
HMAP(_find)(struct HMAP_NAME* table, const HMAP_KEY_TYPE* key) {
  struct HMAP(_bucket) to_find = {
    .key = *key,
    .hash = HMAP(_hash_fun)(key),
    .dist = 0,
  };

  size_t index = to_find.hash % HMAP(_slot_bounds)[table->slot_bound].cap;

  struct HMAP(_bucket)* entry = &parray_get(&table->buckets, index);
  for (;; ++to_find.dist, ++entry) {
    if (entry->dist < to_find.dist) {
      return NULL;
    } else if (HMAP(_key_eq)(entry, &to_find)) {
      return entry;
    }
  }

  UNREACHABLE;
}

static inline void
HMAP(_remove)(struct HMAP(_bucket)* entry) {
  for (entry->dist = -1, ++entry; entry->dist >= 1; entry->dist = -1, ++entry) {
    --entry->dist;
    *(entry - 1) = *entry;
  }
}

static inline
HMAP__RET* HMAP(get)(struct HMAP_NAME* table, const HMAP_KEY_TYPE* key) {
  struct HMAP(_bucket)* found = HMAP(_find(table, key));
  return (found != NULL) ? &HMAP__GET(found) : NULL;
}

bool_t HMAP(erase)(struct HMAP_NAME* table, const HMAP_KEY_TYPE* key) {
  struct HMAP(_bucket)* found = HMAP(_find(table, key));
  if (found == NULL) {
    return FALSE;
  } else {
    HMAP(_remove)(found);
    return TRUE;
  }
}

#ifndef HMAP_HASHSET
bool_t HMAP(extract)(struct HMAP_NAME* table,
		     const HMAP_KEY_TYPE* key,
		     HMAP_VAL_TYPE* out_val) {
  struct HMAP(_bucket)* found = HMAP(_find(table, key));
  if (found == NULL) {
    return FALSE;
  } else {
    *out_val = found->val;
    HMAP(_remove)(found);
    return TRUE;
  }
}
#endif

static inline void __attribute__((always_inline))
HMAP(destroy)(struct HMAP_NAME* table) {
  free(parray_raw(&table->buckets));
}

bool_t HMAP(foreach)(struct HMAP_NAME* table, HMAP(visitor_fun_t) fun, void* arg) {
  struct HMAP(_bucket)* entry;
  parray_foreach(entry, &table->buckets) {
    if (entry->dist >= 0) {
      if (
#ifndef HMAP_HASHSET
	  fun(&entry->key, &entry->val, arg)
#else
	  fun(&entry->key, arg)
#endif
      ) {
	return TRUE;
      }
    }
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

#undef HMAP_NAME
#undef HMAP_KEY_TYPE
#undef HMAP_VAL_TYPE
#undef HMAP_HASHSET
#undef HMAP_HASH_FUN
#undef HMAP_KEY_EQ
#undef HMAP_LOAD_FACTOR
#undef HMAP__GET
#undef HMAP__RET
#undef HMAP__
#undef HMAP_
#undef HMAP
