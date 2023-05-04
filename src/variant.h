#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// variant.h - Define helpers for variant types.
//
////////////////////////////////////////////////////////////////////////////////

#define CONSTRUCTOR_DEF(BASE_TYPE, VARIANT)			\
  static inline __attribute__((warn_unused_result, unused))	\
  struct BASE_TYPE BASE_TYPE ## _ ## VARIANT

#define CONSTRUCTOR(BASE_TYPE, VARIANT, TAG)				\
  static inline __attribute__((warn_unused_result, unused))		\
  bool BASE_TYPE ## _is_ ## VARIANT(const struct BASE_TYPE* b)		\
  { return b->tag == (TAG); }						\
  CONSTRUCTOR_DEF(BASE_TYPE, VARIANT)
