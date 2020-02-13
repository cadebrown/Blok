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

    // the elementwise scale in all directions, the coordinates
    // are multiplied by this
    vec3 scale;

    // then a sample is taken, and clipped at these values
    // NOTE: These should always be between 0 and 1
    double clipMin, clipMax;

    // then, the sample is scaled to this range
    double valMin, valMax;

    // get a single sample of noise (omit the Z parameter to generate just 2D noise)
    double noise(double x, double y, double z=0.5);

    // create a generator given min and max values,
    // clip values
    PerlinGen(uint seed=0, vec3 scale=vec3(1), double clipMin=0.0, double clipMax=1.0, double valMin=0.0, double valMax=1.0);

    private:

    // internal helper methods
    double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);

};

/* LayeredGen - a multiple level noise generator */
class LayeredGen {

    public:

    // individual generators to sum
    List<PerlinGen*> pgens;

    // construct one
    LayeredGen();

    // add a layer to the result, returning the index
    int addLayer(PerlinGen* pgen);

    // return the ith layer
    PerlinGen* getLayer(int idx=0);


    // get noise at given coordinates
    double noise(double x, double y, double z=0.0);

};

}


#endif

