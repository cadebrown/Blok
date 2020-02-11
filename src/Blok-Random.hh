/* Blok-Random.hh - random generation library for Blok */

#pragma once
#ifndef BLOK_RANDOM_HH__
#define BLOK_RANDOM_HH__

#include <Blok.hh>

namespace Blok::Random {



// construct a randomised state
struct RandomState {
    uint32_t a;
};

// the current state of the generator
extern RandomState state;

// run a forward pass on the generator
uint32_t fwd();

// return a floating point value from (0, 1)
float get_f();

// return a double floating point value from (0, 1)
double get_d();



};


#endif

