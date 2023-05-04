#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// test.h - A simple testing suite library.
//
// No harnesses, no safety, just check assertions with nicer formatting.
//
// To enable, pass KC_TESTING when compiling a compilation unit which uses
// TEST_SUITE_DECL.
//
////////////////////////////////////////////////////////////////////////////////

// Assert COND is true inside a test, with error message ....
/////
// tassertf("Greater than 3", x > 3, "x was %d < 3", x)
#define tassertf(NAME, COND, ...)				\
  do {								\
    bool __assert_cond = (COND);				\
    __test_assert_message_(NAME, __assert_cond, __VA_ARGS__);	\
    if (!__assert_cond) { return false; }			\
  } while (0)

// Assert A equals B, with error message ....
/////
// tassert_eqf("Equals 3", x, 3, "x was not 3, it was %d", x)
#define tassert_eqf(NAME, A, B, ...)			\
  tassertf(NAME, (A) == (B), __VA_ARGS__)

// Asserts TRUE, marking a test which is not asserted.
#define tcheckpoint(NAME)			\
  tassertf(NAME, true, "%s", NAME)

#define tassert_error_no(NAME, ERR_PTR, PRINTER)		\
  do {								\
    __auto_type __tassert_err = (ERR_PTR);			\
    if (is_error(__tassert_err)) {				\
      __test_assert_message_(NAME, false, "Error result:");	\
      (PRINTER)(stdout, __tassert_err);				\
      return false;						\
    }								\
  } while (0)

// KC_TESTING enables test suites at compile-time.
#ifdef KC_TESTING

// Declare a test function with NAME and using region REG.
/////
// Must return TRUE if all tests succeed.
// TEST_DECL(foo_tests, r) { assertf(...); ... return TRUE; }
#define TEST_DECL(NAME, REG)				\
  static bool __test_name(NAME)(region_t REG)

// Declare test suite with name NAME, running the given tests.
////
// Tests must be declared/added using 'test_add(<test_name>)'.
// TEST_SUITE_DECL(my_module, test_add(foo_tests), ...);
#define TEST_SUITE_DECL(NAME, ...)					\
  static const struct __test_entry __test_name(NAME ## _array)[] = { __VA_ARGS__ };	\
  static void __attribute__((constructor(200))) __test_name(NAME)() {	\
    __test_suite_execute_(#NAME,					\
			  array_len(__test_name(NAME ## _array)),	\
			  __test_name(NAME ## _array)); }			\
  static const int __attribute__((unused)) __test_name(NAME ## _reserved) = 0

// Add a test to the suite.
#define test_add(NAME)				\
  { .name = #NAME, .fun = __test_name(NAME) }

#else // KC_TESTING is disabled
#define TEST_DECL(NAME, REG)						\
  static bool __attribute__((unused)) __test_name(NAME)(region_t REG)
#define TEST_SUITE_DECL(NAME, ...)					\
  static const int __attribute__((unused)) __test_name(NAME ## _reserved) = 0
#define test_add(NAME) 0
#endif

////////////////////////////////////////////////////////////////////////////////
// Private

#include <stdio.h>

#include "basic.h"
#include "region.h"

#define __test_name(NAME) __test_ ## NAME

#define __test_assert_message_(NAME, PASS, MSG, ...)			\
  printf("    - %s", (NAME)); printf(" ... %s\n", ((PASS)) ? "OK" : "ERR"); \
  if (!(PASS)) { printf("      "); printf((MSG), ## __VA_ARGS__); printf("\n"); }

struct __test_entry {
  const char* name;
  bool (*fun)(region_t);
};

////////////////////////////////////////////////////////////////////////////////

static inline void __attribute__((unused))
__test_suite_execute_(const char*, size_t count, const struct __test_entry[count]);

////////////////////////////////////////////////////////////////////////////////

static inline void __attribute__((unused))
__test_suite_execute_(const char* name,
		      size_t count,
		      const struct __test_entry entries[count]) {
  printf("Running %lu tests in \'%s\' ... \n", count, name);
  size_t num_succeeded = 0;

  const struct __test_entry* test;
  array_foreach(test, count, entries) {
    region_t test_region = r_create();
    printf("[%3lu] %s\n",
	   (size_t) (test - entries) + 1,
	   test->name);
    bool success = test->fun(test_region);
    num_succeeded += (success) ? 1 : 0;
    r_destroy(test_region);
  }

  printf("Completed \'%s\', %lu of %lu tests passed.\n\n",
	 name, num_succeeded, count);
}




