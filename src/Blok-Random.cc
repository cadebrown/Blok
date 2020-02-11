/* Blok-Random.cc - implementation of random number generation */

#include <Blok-Random.hh>

namespace Blok::Random {


// the current state of the generator
RandomState state = { 42 };

// run a forward pass on the generator
uint32_t fwd() {
    uint32_t x = state.a;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state.a = x;
}

// return a floating point value from (0, 1)
float get_f() {
    return fwd() / (float)(UINT32_MAX);
}

// return a double floating point value from (0, 1)
double get_d() {
    return fwd() / (double)(UINT32_MAX);
}


};
