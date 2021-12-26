#include "test.h"

#include "vec.h"
#include "basic.h"

VEC_DECL(int_vec, int);

TEST_DECL(test_vec_int, r) {
  (void) r;

  struct int_vec vec = vec_new(struct int_vec);

  int i;
  range_foreach(i, 0, 1000) {
    vec_push(&vec, i);
  }

  i = 0;
  int* j;
  vec_foreach(j, &vec) {
    if (i != *j) {
      tassertf("vec_push", false, "%d vs. %d", i, *j);
    }
    ++i;
  }
  tcheckpoint("vec_push");

  vec_destroy(&vec);

  return true;
}

TEST_SUITE_DECL(vec_test,
  test_add(test_vec_int));
