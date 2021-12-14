#pragma once

////////////////////////////////////////////////////////////////////////////////
//
// iter.h - Contains common iterators and iterator utilities.
//
// Included in "common.h".
//
////////////////////////////////////////////////////////////////////////////////

// Iterate from START to END, setting VAR.
#define range_foreach(VAR, START, END)		\
  for ((VAR) = (START);				\
       (VAR) < (END);				\
       (VAR)++)

// Iterate from END to START, setting VAR.
#define range_foreach_rev(VAR, END, START)	\
  for ((VAR) = (END);				\
       (START) < (VAR);				\
       (VAR)--)

////////////////////////////////////////////////////////////////////////////////

// Iterate over ARR of type <type>[LEN], setting VAR.
#define array_foreach(VAR, LEN, ARR)		\
  for ((VAR) = (ARR);				\
       (VAR) != ((ARR) + (LEN));		\
       (VAR)++)

// Iterate over ARR of type <type>[LEN], in reverse, setting VAR.
#define array_foreach_rev(VAR, LEN, ARR)	\
  for ((VAR) = ((ARR) + ((LEN) - 1));		\
       (ARR) < (VAR) || (VAR) == (ARR);		\
       (VAR)--)

// Iterate over ARR of type <type>[LEN], setting VAR_ARR and index VAR_IDX.
#define array_idx_foreach(VAR_ARR, VAR_IDX, LEN, ARR)	\
  for ((VAR_ARR) = (ARR), (VAR_IDX) = (LEN);			\
       (VAR_ARR) != ((ARR) + (LEN));				\
       (VAR_ARR)++, (VAR_IDX)--)

// Iterate over ARR of type <type>[LEN], in reverse, setting VAR_ARR and index
// VAR_IDX.
#define array_idx_foreach_rev(VAR_ARR, VAR_IDX, LEN, ARR)	\
  for ((VAR_ARR) = ((ARR) + ((LEN) - 1)), (VAR_IDX) = 0;	\
       (ARR) < (VAR_ARR) || (VAR_ARR) == (ARR);			\
       (VAR_ARR)--, (VAR_IDX)++)
