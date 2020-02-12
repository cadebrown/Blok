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


/* PerlinGen - a perlin noise generator */
class PerlinGen {

    public:

    // the seed of the generator
    uint seed;

    // a permutation array
    List<int> perm;

    // get a single sample of noise (omit the Z parameter to generate just 2D noise)
    double noise(double x, double y, double z=0.5);

    PerlinGen(uint seed=0);

    private:

    // internal helper methods
    double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);


};

}


#endif

