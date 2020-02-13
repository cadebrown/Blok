/* Blok/Random.hh - various random number routines, as well as 
 *   noise generation routines
 *
 */


#pragma once
#ifndef BLOK_RANDOM_HH__
#define BLOK_RANDOM_HH__

// general Blok library
#include <Blok/Blok.hh>

namespace Blok::Random {

/* XorShift : simple XOR-shift (https://en.wikipedia.org/wiki/Xorshift) based
 * PRNG
 */
class XorShift {

    public:

    // state variable
    uint32_t a;

    // generate a U32 variable, equally likely for all values
    uint32_t getU32() {
        // apply XorShift algo
        uint32_t x = a;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return a = x;
    }

    // generate a U64 variable, equally likely for all values
    uint64_t getU64() {
        // do 2 forward passes, concatting the bits
        uint64_t res = ((uint64_t)getU32() << 32) | getU32();
        return res;
    }

    // generate a floating point variable in [0, 1)
    float getF() {
        static const uint32_t fmask = 0xFFFFFFFFUL;
        // compute it as a fixed point value
        return (float)(getU32() & fmask) / fmask;
    }

    // generate a floating point variable in [0, 1)
    double getD() {
        static const uint32_t fmask = 0xFFFFFFFFUL;
        // compute it as a fixed point value
        return (double)(getU32() & fmask) / fmask;
    }

    XorShift(uint32_t seed=0) {
        // ensure it is never 0 to begin with
        a = seed == 0 ? 1 : seed;

        // seed up the generator
        getU32();
        getU32();
    }

};

}

#endif /* BLOK_RANDOM_HH__ */


