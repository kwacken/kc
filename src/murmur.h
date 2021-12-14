#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// murmur.h - An implementation of the murmur hash algorithm.
//
////////////////////////////////////////////////////////////////////////////////

#include "common.h"

static inline size_t
murmur_hash(const void* key, size_t len);

static size_t
murmur_hash_(const void* key, size_t len, size_t seed);

////////////////////////////////////////////////////////////////////////////////
// Private

static size_t murmur_hash_(const void* key, size_t len, size_t seed) {
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


static inline size_t __attribute__((unused, warn_unused_result))
murmur_hash(const void* key, size_t len) {
  return murmur_hash_(key, len, 0);
}
