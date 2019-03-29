#ifndef _WALTER_ANDERSON_JITTER_RANDOM_H_
#define _WALTER_ANDERSON_JITTER_RANDOM_H_

#include <inttypes.h>

class JitterRandom {
  public:
    // Returns an unsigned 32-bit random value.
    static uint32_t random32();
};

#endif  // _WALTER_ANDERSON_JITTER_RANDOM_H_