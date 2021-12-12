#include "hmap.h"

#include <stdlib.h>
#include <math.h>

struct map_slots {
  size_t cap;
  int8_t max_dist;
};

// Primes after to prev_prime * 1.5 and log2.
// Capacity growth by powers of 2 are memory inefficient, though we now suffer
// from slower mod times.
static const struct map_slots slot_bounds[] = {
  { 5lu, 2 }, { 7lu, 3 }, { 11lu, 3 },
  { 17lu, 4 }, { 23lu, 5 }, { 37lu, 5 },
  { 53lu, 6 }, { 79lu, 6 }, { 113lu, 7 },
  { 163lu, 7 }, { 229lu, 8 }, { 331lu, 8 },
  { 463lu, 9 }, { 653lu, 9 }, { 919lu, 10 },
  { 1289lu, 10 }, { 1811lu, 11 }, { 2539lu, 11 },
  { 3557lu, 12 }, { 4987lu, 12 }, { 6983lu, 13 },
  { 9781lu, 13 }, { 13693lu, 14 }, { 19181lu, 14 },
  { 26861lu, 15 }, { 37607lu, 15 }, { 52667lu, 16 },
  { 73751lu, 16 }, { 103289lu, 17 }, { 144611lu, 17 },
  { 202471lu, 18 }, { 283463lu, 18 }, { 396871lu, 19 },
  { 555637lu, 19 }, { 777901lu, 20 }, { 1089091lu, 20 },
  { 1524763lu, 21 }, { 2134697lu, 21 }, { 2988607lu, 22 },
  { 4184087lu, 22 }, { 5857727lu, 22 }, { 8200847lu, 23 },
  { 11481199lu, 23 }, { 16073693lu, 24 }, { 22503181lu, 24 },
  { 31504453lu, 25 }, { 44106241lu, 25 }, { 61748749lu, 26 },
  { 86448259lu, 26 }, { 121027583lu, 27 }, { 169438627lu, 27 },
  { 237214097lu, 28 }, { 332099741lu, 28 }, { 464939639lu, 29 },
  { 650915521lu, 29 }, { 911281733lu, 30 }, { 1275794449lu, 30 },
  { 1786112231lu, 31 }, { 2500557133lu, 31 }, { 3500779987lu, 32 },
  { 4901092003lu, 32 }, { 6861528851lu, 33 }, { 9606140399lu, 33 },
  { 13448596583lu, 34 }, { 18828035323lu, 34 }, { 26359249459lu, 35 },
  { 36902949287lu, 35 }, { 51664129003lu, 36 }, { 72329780647lu, 36 },
  { 101261692921lu, 37 }, { 141766370143lu, 37 }, { 198472918207lu, 38 },
  { 277862085493lu, 38 }, { 389006919737lu, 39 }, { 544609687669lu, 39 },
  { 762453562751lu, 39 }, { 1067434987873lu, 40 }, { 1494408983027lu, 40 },
  { 2092172576243lu, 41 }, { 2929041606761lu, 41 }, { 4100658249511lu, 42 },
  { 5740921549367lu, 42 }, { 8037290169151lu, 43 }, { 11252206236863lu, 43 },
  { 15753088731613lu, 44 }, { 22054324224277lu, 44 }, { 30876053913989lu, 45 },
  { 43226475479591lu, 45 }, { 60517065671459lu, 46 }, { 84723891940099lu, 46 },
  { 118613448716141lu, 47 }, { 166058828202599lu, 47 }, { 232482359483653lu, 48 },
  { 325475303277143lu, 48 }, { 455665424588069lu, 49 }, { 637931594423311lu, 49 },
  { 893104232192647lu, 50 }, { 1250345925069739lu, 50 }, { 1750484295097673lu, 51 },
  { 2450678013136757lu, 51 }, { 3430949218391537lu, 52 }, { 4803328905748207lu, 52 },
  { 6724660468047551lu, 53 }, { 9414524655266669lu, 53 }, { 13180334517373357lu, 54 },
  { 18452468324322709lu, 54 }, { 25833455654051813lu, 55 }, { 36166837915672573lu, 55 },
  { 50633573081941709lu, 55 }, { 70887002314718437lu, 56 }, { 99241803240605819lu, 56 },
  { 138938524536848189lu, 57 }, { 194513934351587491lu, 57 }, { 272319508092222467lu, 58 },
  { 381247311329111473lu, 58 }, { 533746235860756057lu, 59 }, { 747244730205058483lu, 59 },
  { 1046142622287081827lu, 60 }, { 1464599671201914671lu, 60 }, { 2050439539682680411lu, 61 },
  { 2870615355555752471lu, 61 }
};

static const float ALLOC_LOAD_FACTOR = 0.9f;

size_t murmur_hash(const void* key, size_t len, size_t seed) {
  const size_t m = 0x5bd1e995;
  const int r = 24;

  size_t h = seed ^ len;

  const size_t* data = (const size_t *)key;
  const size_t* end = data + (len / sizeof(size_t));

  while(data != end) {
    size_t k = *data++;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  const unsigned char* data2 = (const unsigned char*) data;

  switch ((len - (size_t) (end -  data))) {
  case 7: h ^= ((size_t) data2[6]) << 48; /* fall through */
  case 6: h ^= ((size_t) data2[5]) << 40; /* fall through */
  case 5: h ^= ((size_t) data2[4]) << 32; /* fall through */
  case 4: h ^= ((size_t) data2[3]) << 24; /* fall through */
  case 3: h ^= ((size_t) data2[2]) << 16; /* fall through */
  case 2: h ^= ((size_t) data2[1]) << 8;  /* fall through */
  case 1: h ^= ((size_t) data2[0]);       /* fall through */
    h *= m;
  };

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

struct hmap_bucket {
  void* key;
  void* val;
  size_t hash;
  int8_t dist;
};

static inline size_t slot_count(uint8_t slot_bound) {
  return slot_bounds[slot_bound].cap + (uint8_t) slot_bounds[slot_bound].max_dist;
}

static inline size_t slot_load_count(uint8_t slot_bound) {
  float bound = (float) slot_count(slot_bound) * ALLOC_LOAD_FACTOR;
  return (size_t) lround(bound);
}

static inline uint8_t find_slot_bound(size_t slots) {
  uint8_t len = array_size(slot_bounds);
  uint8_t slot_bound;
  for (slot_bound = 0; slot_bound < len; ++slot_bound) {
    if (slot_load_count(slot_bound) >= slots) {
      return slot_bound;
    }
  }
  return slot_bound;
}

static inline bool_t bucket_has_value(const struct hmap_bucket* bucket) {
  return bucket->dist >= 0;
}

static inline bool_t bucket_is_empty(const struct hmap_bucket* bucket) {
  return bucket->dist < 0;
}

static inline void bucket_swap(struct hmap_bucket* lhs, struct hmap_bucket* rhs) {
  struct hmap_bucket tmp;
  tmp = *lhs;
  *lhs = *rhs;
  *rhs = tmp;
}

static inline bool_t key_eq(struct hmap* table, struct hmap_bucket* lhs, struct hmap_bucket* rhs) {
  return lhs->hash == rhs->hash && table->key_eq_fun(lhs->key, rhs->key, table->key_len);
}


static inline void hmap_init(struct hmap* table, size_t key_len,
			     hash_function_t hash_fun,
			     key_eq_function_t key_eq_fun,
			     uint8_t slot_bound) {
  struct hmap_bucket* ret = sys_malloc(sizeof(struct hmap_bucket) * slot_count(slot_bound));

  struct hmap_bucket* bucket;
  array_foreach(bucket, slot_count(slot_bound), ret) {
    bucket->dist = -1;
  }
  *table = (struct hmap) {
    .slot_bound = slot_bound,
    .num_items = 0,
    .key_len = key_len,
    .hash_fun = hash_fun,
    .key_eq_fun = key_eq_fun,
    .buckets = ret };
}

struct hmap hmap_new_(size_t key_len,
		      hash_function_t hash_fun,
		      key_eq_function_t key_eq_fun) {
  struct hmap new;
  hmap_init(&new, key_len, hash_fun, key_eq_fun, 0);
  return new;
}

struct hmap hmap_new_reserve_(size_t key_len,
			      hash_function_t hash_fun,
			      key_eq_function_t key_eq_fun,
			      size_t slots) {
  struct hmap new;
  hmap_init(&new, key_len, hash_fun, key_eq_fun, find_slot_bound(slots));
  return new;
}

void hmap_destroy(struct hmap* table) {
  free(table->buckets);
}

static void* hmap_insert_inner(struct hmap* table, struct hmap_bucket new);

static void hmap_grow_to(struct hmap* table, uint8_t slot_bound) {
  struct hmap new;
  hmap_init(&new, table->key_len, table->hash_fun, table->key_eq_fun, slot_bound);

  struct hmap_bucket* entry;
  array_foreach(entry, slot_count(table->slot_bound), table->buckets) {
    if (bucket_has_value(entry)) {
      hmap_insert_inner(&new, *entry);
    }
  }

  hmap_destroy(table);
  *table = new;
}

static void hmap_grow(struct hmap* table) {
  return hmap_grow_to(table, table->slot_bound + 1);
}

void hmap_reserve(struct hmap* table, size_t slots) {
  uint8_t slot_bound = find_slot_bound(slots);
  if (slot_bound > table->slot_bound) {
    hmap_grow_to(table, slot_bound);
  }
}

static void* hmap_insert_inner(struct hmap* table, struct hmap_bucket new) {
  size_t index;
  struct hmap_bucket* entry;

insert:
  new.dist = 0;
  index = new.hash % slot_bounds[table->slot_bound].cap;
  entry = &table->buckets[index];

  for (; entry->dist >= new.dist; ++entry, ++new.dist) {
    if (key_eq(table, entry, &new)) {
      return entry->val;
    }
  }

  if (new.dist == slot_bounds[table->slot_bound].max_dist) {
    hmap_grow(table);
    goto insert;
  } else if (bucket_is_empty(entry)) {
    ++table->num_items;
    *entry = new;
    return NULL;
  }

  bucket_swap(entry, &new);

  for (++entry, ++new.dist;; ++entry) {
    if (bucket_is_empty(entry)) {
      ++table->num_items;
      *entry = new;
      return NULL;
    } else if (entry->dist < new.dist) {
      bucket_swap(entry, &new);
      ++new.dist;
    } else {
      ++new.dist;
      if (new.dist == slot_bounds[table->slot_bound].max_dist) {
	hmap_grow(table);
	goto insert;
      }
    }
  }

  hmap_grow(table);
  goto insert;
}

void* hmap_insert(struct hmap* table, void* key, void* val) {
  if (table->num_items + 1 > slot_load_count(table->slot_bound)) {
    hmap_grow(table);
  }

  return hmap_insert_inner(table, (struct hmap_bucket) {
    .key = key,
    .val = val,
    .hash = table->hash_fun(key, table->key_len),
    .dist = 0,
  });
}

void* hmap_get(struct hmap* table, const void* key) {
  struct hmap_bucket to_find = {
    .key = (void*) key,
    .hash = table->hash_fun(key, table->key_len),
    .dist = 0,
  };

  size_t index = to_find.hash % slot_bounds[table->slot_bound].cap;

  struct hmap_bucket* entry = &table->buckets[index];
  for (;; ++to_find.dist, ++entry) {
    if (entry->dist < to_find.dist) {
      return NULL;
    } else if (key_eq(table, entry, &to_find)) {
      return entry->val;
    }
  }

  UNREACHABLE;
}

void* hmap_erase(struct hmap* table, const void* key) {
  struct hmap_bucket to_find = {
    .key = (void*) key,
    .hash = table->hash_fun(key, table->key_len),
    .dist = 0,
  };

  size_t index = to_find.hash % slot_bounds[table->slot_bound].cap;

  struct hmap_bucket* entry = &table->buckets[index];
  for (;; ++to_find.dist, ++entry) {
    if (entry->dist < to_find.dist) {
      return NULL;
    } else if (key_eq(table, entry, &to_find)) {
      entry->dist = -1;
      return entry->val;
    }
  }

  UNREACHABLE;
}


bool_t hmap_foreach(struct hmap* table, hmap_foreach_function_t fun, void* arg) {
  struct hmap_bucket* entry;
  array_foreach(entry, slot_count(table->slot_bound), table->buckets) {
    if (bucket_has_value(entry)) {
      if (fun(entry->key, table->key_len, entry->val, arg)) {
	return TRUE;
      }
    }
  }
  return FALSE;
}

