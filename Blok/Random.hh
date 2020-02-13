/* Blok/Random.hh - various random number routines, as well as 
 *   noise generation routines
 *
 * The entire algorithm is included in these headers, in the hope that these
 *   functions can be inlined wherever they are used, in order to speed up processing speed,
 *   and simplify code reuse
 * 
 */


#pragma once
#ifndef BLOK_RANDOM_HH__
#define BLOK_RANDOM_HH__

// general Blok library
#include <Blok/Blok.hh>

namespace Blok::Random {

// XorShift - simple XOR-shift (https://en.wikipedia.org/wiki/Xorshift) based
//   PRNG
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
        static const uint32_t fmask = 0xFFFFFFUL;
        // compute it as a fixed point value
        return (float)(getU32() & fmask) / (float)(1 + fmask);
    }

    // generate a floating point variable in [0, 1)
    double getD() {
        static const uint32_t fmask = 0xFFFFFFFFUL;
        // compute it as a fixed point value
        return (double)(getU32() & fmask) / (double)(1 + fmask);
    }

    // construct a generator, given an initial the seed
    XorShift(uint32_t seed=0) {
        // ensure it is never 0 to begin with
        a = seed == 0 ? 1 : seed;

        // seed up the generator
        getU32();
        getU32();
    }

};


// Perlin - a Perlin noise (https://en.wikipedia.org/wiki/Perlin_noise) generator
class Perlin {
    public:

    // the size of the table for a Perlin noise generation algorithm
    static const int tableSize = 256;

    // a list of values to be used in the internal algorithm
    List<uint32_t> perms;

    // the scale of the perlin noise, i.e. the input coordinates
    //   are multiplied by this
    vec3 scale;

    // the space that the initial sample is clipped to, in (min, max) format
    // default is (0, 1) which does nothing
    vec2 clipSpace;

    // the range of the output, in (min, max) format
    // default is (0, 1) which does nothing
    vec2 outputSpace;

    // construct a perlin generator from a given seed
    Perlin(uint32_t seed=0, vec3 scale={1.0, 1.0, 1.0}, vec2 clipSpace={0.0, 1.0}, vec2 outputSpace={0.0, 1.0}) {
        // set member vars
        this->scale = scale;
        this->clipSpace = clipSpace;
        this->outputSpace = outputSpace;

        // generate random integers from an XorShift generator
        XorShift permgen(seed);

        perms = {};
        
        // populate it with random numbers in [0, 256)
        for (int i = 0; i < tableSize; ++i) {
            perms.push_back(permgen.getU32() % tableSize);
        }
    }


    // apply clipping & scaling to a value
    double toOutput(double res) {

        //res = glm::clamp(res, clipMin, clipMax);
        if (res < clipSpace[0]) res = clipSpace[0];
        else if (res > clipSpace[1]) res = clipSpace[1];
        //res = ((res - clipMin) / (clipMax - clipMin)) * (valMax - valMin) + valMin;

        // scale the result to the requested range
        res = ((res - clipSpace[0]) / (clipSpace[1] - clipSpace[0])) * (outputSpace[1] - outputSpace[0]) + outputSpace[0];

        return res;
    }

    // internal utility method to fade a double
    double fade(double t) { 
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    // linearly interpolate between 'a' and 'b', given a parameter 't' from 0-1
    double lerp(double t, double a, double b) { 
        return a + t * (b - a); 
    }

    // internally compute a gradient direction, based on the permutation table
    double grad(int hash, double x, double y=0.0, double z=0.0) {
        int h = hash & 15;
        // Convert lower 4 bits of hash into 12 gradient directions
        double u = h < 8 ? x : y,
            v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }


    // generate noise from 1 spatial coordinate
    double noise1d(double x) {

        // first, scale the coordinate
        x *= scale.x;

        // compute discrete sample points, and their next coordinate
        int x0 = (int)floor(x) & 0xFF;

        // convert them all to unit cube coordinates
        x -= floor(x);

        // compute fractional portion of the sample point, always between 0 and 1
        double xf = fade(x);

        // alterate:
        // x= fade(x), xf = x, for a slightly different algo
        // this also works pretty well, but lead to some glitches

        // coordinates of sphere
        int A = perms[x0];
        int AA = perms[A % tableSize];
        int B = perms[(x0 + 1) % tableSize];
        int BA = perms[B % tableSize];

        // blend sides of a line
        double res = lerp(xf, grad(perms[AA] % tableSize, x), grad(perms[BA] % tableSize, x-1));
        res = (res + 1.0)/2.0;

        return toOutput(res);
    }

    // generate noise from 2 spatial coordinates
    double noise2d(double x, double y=0.0) {

        // first, scale the coordinates
        x *= scale.x;
        y *= scale.y;

        // compute discrete sample points, and their next coordinate
        int x0 = (int)floor(x) & 0xFF, y0 = (int)floor(y) & 0xFF;

        // convert them all to unit cube coordinates
        x -= floor(x);
        y -= floor(y);

        // compute fractional portion of the sample point, always between 0 and 1
        double xf = fade(x), yf = fade(y);

        // coordinates of sphere
        int A = perms[x0] + y0;
        int AA = perms[A % tableSize];
        int AB = perms[(A + 1) % tableSize];
        int B = perms[(x0 + 1) % tableSize] + y0;
        int BA = perms[B % tableSize];
        int BB = perms[(B + 1) % tableSize];


        // blend corners of the square
        double res = lerp(yf, 
            lerp(xf, grad(perms[AA] % tableSize, x, y), grad(perms[BA] % tableSize, x-1, y)), 
            lerp(xf, grad(perms[AB] % tableSize, x, y-1), grad(perms[BB] % tableSize, x-1, y-1))
        );
        res = (res + 1.0)/2.0;

        return toOutput(res);
    }

    // generate noise from 3 spatial coordinates
    double noise3d(double x, double y=0.0, double z=0.0) {

        // first, scale the coordinates
        x *= scale.x;
        y *= scale.y;
        z *= scale.z;

        // compute discrete sample points, and their next coordinate
        int x0 = (int)floor(x) % tableSize, y0 = (int)floor(y) % tableSize, z0 = (int)floor(z) % tableSize;

        if (x0 < 0) x0 += tableSize;
        if (y0 < 0) y0 += tableSize;
        if (z0 < 0) z0 += tableSize;
        //printf("%i,%i,%i\n", x0, y0, z0);


        // convert them all to unit cube coordinates
        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        // compute fractional portion of the sample point, always between 0 and 1
        double xf = fade(x), yf = fade(y), zf = fade(z);


        // coordinates in a 3D cube
        int A = perms[x0 % tableSize] + y0;
        int AA = perms[A % tableSize] + z0;
        int AB = perms[(A + 1) % tableSize] + z0;

        int B = perms[(x0 + 1) % tableSize] + y0;
        int BA = perms[B % tableSize] + z0;
        int BB = perms[(B + 1) % tableSize] + z0;

        // blender corners of the cube
        double res = lerp(zf, 
            lerp(yf, 
                lerp(xf, grad(perms[AA % tableSize], x, y, z), grad(perms[BA % tableSize], x-1, y, z)), 
                lerp(xf, grad(perms[AB % tableSize], x, y-1, z), grad(perms[BB % tableSize], x-1, y-1, z))
            ),	
            lerp(yf, 
                lerp(xf, grad(perms[(AA+1) % tableSize], x, y, z-1), grad(perms[(BA+1) % tableSize], x-1, y, z-1)), 
                lerp(xf, grad(perms[(AB+1) % tableSize], x, y-1, z-1), grad(perms[(BB+1) % tableSize], x-1, y-1, z-1))
            )
        );
        res = (res + 1.0)/2.0;

        return toOutput(res);

    }
};


// PerlinMux : a mix/muxer of multiple layers of perlin noise,
//   additively
class PerlinMux {
    public:

    // a list of layers to be added to the generator
    List<Perlin> layers;

    // construct an (empty) perlin layered generator
    PerlinMux() {
        layers = {};
    }


    // add a layer to the internal layers array
    void addLayer(const Perlin& lyr) {
        layers.push_back(lyr);
    }

    // generate 1D noise
    double noise1d(double x) {
        // just sum up all the layers
        double val = 0.0;

        // loop through all layers
        for (Perlin& lyr : layers) {
            val += lyr.noise1d(x);
        }

        return val;
    }

    // generate 2D noise
    double noise2d(double x, double y=0.0) {
        // just sum up all the layers
        double val = 0.0;

        // loop through all layers
        for (Perlin& lyr : layers) {
            val += lyr.noise2d(x, y);
        }

        return val;
    }

    // generate 3D noise
    double noise3d(double x, double y=0.0, double z=0.0) {
        // just sum up all the layers
        double val = 0.0;

        // loop through all layers
        for (Perlin& lyr : layers) {
            val += lyr.noise3d(x, y, z);
        }

        return val;
    }


};

}

#endif /* BLOK_RANDOM_HH__ */


