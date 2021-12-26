#include "test.h"

#include "list.h"
#include "region.h"

struct size_node {
  size_t entry;
  SLIST_ENTRY(struct size_node) slist;
};

SLIST_DECL(size_list, struct size_node);

bool corruption_test(region_t r, const char* name) {
  static const size_t MAX = 100000;
  size_t i;
  struct size_list list;
  slist_init(&list);

  range_foreach(i, 0, MAX) {
    struct size_node* node = r_malloc(r, struct size_node);
    node->entry = i;
    slist_insert(&list, node, slist);
  }

  i = MAX - 1;
  struct size_node* curr;
  slist_foreach(curr, &list, slist) {
    if (curr->entry != i) {
      tassert_eqf(name, i, curr->entry, "%lu vs. %lu", i, curr->entry);
    }
    --i;
  }
  tcheckpoint(name);

  return true;
}

TEST_DECL(test_regions, r) {
  tassertf("corrupt_parent",
	   corruption_test(r, "corruption 0"),
	   "Parent region corrupted");

  size_t i;
  range_foreach(i, 0, 10) {
    tassertf("corrupt_child",
	     corruption_test(r, "corruption"),
	     "Child region %lu corrupted", i);
  }

  return true;
}


TEST_SUITE_DECL(region_test,
  test_add(test_regions));
