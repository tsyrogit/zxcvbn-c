
/* Internal tests that cannot be run against the library without unnecessary export of symbols.
   Copyright (c) 2017, Perlbotics / see LICENSE.txt
 */


#include "zxcvbn.h"
/* need access to internals for testing; not linked against library, just inlined */
#include "zxcvbn.c"


/**********************************************************************************
 * Internal checks: Validate if the first element of each group is sorted in
 *                  ascending order. CharBinSearch(...) fails otherwise.
 * Returns 0 on success.
 * Returns element index [1..] of first error entry that is less than previous one.
 */
static int check_order(const uint8_t *Ents, unsigned int NumEnts, unsigned int SizeEnts) {
  const uint8_t *last;
  unsigned int i;

  if (!Ents) return 0;
  last = 0;

  for (i = 0; i < NumEnts; ++i, Ents += SizeEnts) {
    if (last  &&  *last > *Ents) {
      unsigned int j;

      printf("Entry#%d [%d]:  '%c' > '%c'  (0x%02X > 0x%02X)\n    A:  ", i, i * SizeEnts, *last, *Ents, *last, *Ents);
      for (j = 0; j < SizeEnts; ++j) {
        printf("'%c' ", last[j] ? last[j] : ' ');
      }
      printf("\n    >\n    B:  ");
      for (j = 0; j < SizeEnts; ++j) {
        printf("'%c' ", Ents[j] ? Ents[j] : ' ');
      }
      printf("\n");

      return i;
    }
    last = Ents;
  }

  return 0; /* cannot be a misordered position; first possible one: 1 */
}

/**********************************************************************************
 * Internal checks: Checks keyboard data integrity.
 * Returns 0 on succes.
 * Otherwise, number of errors are reported.
 */
static unsigned int selftest_keyboards() {
  unsigned int errors;
  const Keyboard_t *k;
  unsigned int Indx;
  const uint8_t *keys;
  int i,j,errpos, blanks;

  errors = 0;
  for(k = Keyboards, Indx = 0; Indx < (sizeof Keyboards / sizeof Keyboards[0]); ++Indx, ++k) {
    /* if one of these assrtion fails, we cannot use binary search algorithm */
    if (k->Shifts  &&  strlen((const char*)k->Shifts) % 2 == 1) {
      printf("ERROR: Keyboard[%d]: Shifts-string has odd number of entries.\n", Indx);
      ++errors;
    }

    if ( (errpos = check_order(k->Shifts, k->NumShift, 2)) ) {
      printf("ERROR: Keyboard[%d]: Error above in sort order of Shifts-string near item #%d.\n", Indx, errpos);
      ++errors;
    }

    if ( (errpos = check_order(k->Keys, k->NumKeys, k->NumNear)) ) {
      printf("ERROR: Keyboard[%d]: Error above in sort order of keyboard-entries! Problem near item #%d.\n", Indx, errpos);
      ++errors;
      continue;
    }

    /* For each key (c0), check all its neighbours (ci):
     * Does the neighbour key (c1==ci) have an entry (cx) in the opposite direction [rev_idx]
     * pointing back to the current key c0?
     * c0: ...ci..   -->   c1: ..cx...   -->   cx==c0?
     */
    keys = k->Keys;
    blanks = 0;
    for(i = 0; i < k->NumKeys; ++i) {
      uint8_t c0;
      c0 = keys[i * k->NumNear];

      for (j = 0; j < k->NumNear - 1; ++j) {
        const uint8_t *c1;
        uint8_t ci, cx;
        int rev_idx;

        /* rev_idx: reverse/opposite index to find opposite key location [0..6|8] --> [0..6|8] */
        rev_idx = (j + (k->NumNear - 1)/2) % (k->NumNear - 1);
        ci = keys[i * k->NumNear + j + 1];

        if (ci) {
          c1 = CharBinSearch(ci, keys, k->NumKeys, k->NumNear);
          if (c1) {
            if (ci == c0) {
              printf("ERROR: Keyboard[%d]:  recursion - key '%c' cannot be its own neighbour!\n", Indx, *c1);
              ++errors;
            } else {
              if ( (cx = c1[ 1 + rev_idx ]) ) {
                if ( cx != c0 ) {
                  printf("ERROR: Keyboard[%d]:  c0='%c':...(ci=%c)... ->  c1='%c':...(cx=%c)... --!--> c0='%c':... \n",
                         Indx, c0, ci, *c1, cx, c0);
                  ++errors;
                }
              } else { /* reverse pointer is NULL */
                printf("ERROR: Keyboard[%d]:  reverse entry missing in row c1='%c'[%d] pointing back to c0='%c'!\n", Indx, *c1, 1+rev_idx, c0);
                ++errors;
              }
            }
          } else {
            printf("ERROR: Keyboard[%d]:  no entry (neighbour list) found for src-char c1==ci='%c'\n", Indx, ci);
            ++errors;
          }
        } else { /* blank neighbour key reference found */
          ++blanks;
        }
      }
    }
    if (blanks != k->NumBlank) {
      printf("ERROR: Keyboard[%d]:  number of blank keys announced (%d) does not match number of blank keys counted (%d)!\n",
             Indx, k->NumBlank, blanks);
      ++errors;
    }
  }
  return errors;
}




int main() {
  unsigned int errors;

  if( (errors = selftest_keyboards()) ){  /* currently only these */
    printf("FAILED: [KEYBOARDS] - selftest returned %d error(s).\n", errors);
  }

  return errors ? 1 : 0;
}
