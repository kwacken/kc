#include "test.h"
#include "basic.h"

#define HMAP_NAME hmap_int_int
#define HMAP_KEY_TYPE int
#define HMAP_VAL_TYPE unsigned int
#include "hmap.h"

TEST_DECL(test_int_int, r) {
  (void) r;

  struct hmap_int_int map = hmap_int_int_new();

  int i;
  unsigned int* prev = NULL;
  range_foreach(i, 0, 400) {
    prev = hmap_int_int_insert(&map, &i, (unsigned int*)&i);
    if (prev != NULL) {
      tassert_eqf("insert", prev, NULL, "Got non-NULL, got %d", *prev);
    }
  }

  hmap_int_int_reserve(&map, 1000);

  range_foreach(i, 400, 1500) {
    hmap_int_int_insert(&map, &i, (unsigned int*)&i);
  }

  unsigned int j = 0;
   range_foreach(i, 300, 600) {
    hmap_int_int_extract(&map, &i, &j);
    if (i != (int)j) {
      break;
    }
  }
  hmap_int_int_extract(&map, &i, &j);
  tassert_eqf("extract", i, (int)j, "%d vs. %du", i, j);

  range_foreach(i, 600, 700) {
    hmap_int_int_erase(&map, &i);
  }

  unsigned int* h;
  range_foreach_rev(i, 1499, 700) {
    h = hmap_int_int_get(&map, &i);
    if (h == NULL) {
      tassertf("get", h != NULL, "Got NULL on %d", i);
    }
    if (i != (int)*h) {
      break;
    }
  }
  h = hmap_int_int_get(&map, &i);
  tassert_eqf("get 0", i, (int)*h, "%d vs. %du", i, *h);

  range_foreach_rev(i, 299, 0) {
    h = hmap_int_int_get(&map, &i);
    if (i != (int)*h) {
      break;
    }
  }
  h = hmap_int_int_get(&map, &i);
  tassert_eqf("get 1", i, (int)*h, "%d vs. %du", i, *h);

  hmap_int_int_destroy(&map);

  return TRUE;
}

static inline size_t shift_str_hash(const string_t* ptr) {
  return ((size_t) string(*ptr)) >> 4;
}

static inline bool_t string_t_eq(const string_t* s1, const string_t* s2) {
  return string_eq(*s1, *s2);
}

#define HMAP_NAME hset_string
#define HMAP_KEY_TYPE string_t
#define HMAP_HASH_FUN shift_str_hash
#define HMAP_KEY_EQ string_t_eq
#define HMAP_LOAD_FACTOR 0.5f
#include "hmap.h"

TEST_DECL(test_string_set, r) {
  (void)r;

  struct hset_string set = hset_string_new();

  string_t array[] = {
    as_string_t("foo"),
    as_string_t("bar"),
    as_string_t("cat"),
    as_string_t("baz"),
    as_string_t("bir"),
    as_string_t("bun"),
  };

  const string_t* str;
  array_foreach(str, array_len(array), array) {
    hset_string_insert(&set, str);
  }

  array_foreach_rev(str, array_len(array), array) {
    const string_t* key = hset_string_get(&set, str);
    tassertf("get", string_eq(*key, *str), "%s vs. %s", string(*key), string(*str));
  }

  hset_string_destroy(&set);

  return TRUE;
}


TEST_SUITE_DECL(hmap_test,
  test_add(test_int_int),
  test_add(test_string_set));
