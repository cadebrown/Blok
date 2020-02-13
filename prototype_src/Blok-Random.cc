/* Blok-Random.cc - implementation of random number generation */

#include <Blok-Random.hh>


#include <numeric>

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


/* perlin noise */

PerlinGen::PerlinGen(uint seed, vec3 scale, double clipMin, double clipMax, double valMin, double valMax) {

	this->seed = seed;
	this->scale = scale;

	this->clipMin = clipMin;
	this->clipMax = clipMax;
	this->valMin = valMin;
	this->valMax = valMax;

	// Initialize the permutation vector with the reference values
	if (seed == 0) {
		perm = {
			151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
			8,99,37,240,21,10,23,190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
			35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,
			134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
			55,46,245,40,244,102,143,54, 65,25,63,161,1,216,80,73,209,76,132,187,208, 89,
			18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,
			250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
			189,28,42,223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 
			43,172,9,129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,
			97,228,251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,
			107,49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 
		};
	} else {
		perm = {};

		for (int i = 0; i < 256; ++i) perm.push_back(fwd() & 0xFF);
	}

	// Duplicate the permutation vector
	perm.insert(perm.end(), perm.begin(), perm.end());
}


double PerlinGen::fade(double t) { 
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinGen::lerp(double t, double a, double b) { 
	return a + t * (b - a); 
}


double PerlinGen::grad(int hash, double x, double y, double z) {
	int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
	double u = h < 8 ? x : y,
		   v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinGen::noise(double x, double y, double z) {

	x *= scale.x;
	y *= scale.y;
	//z *= scale.z;

    // Find the unit cube that contains the point
	int X = (int) floor(x) & 255;
	int Y = (int) floor(y) & 255;
	int Z = (int) floor(z) & 255;

	// Find relative x, y, z of point in cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves for each of x, y, z
	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	// Hash coordinates of the 8 cube corners
	int A = perm[X] + Y;
	int AA = perm[A] + Z;
	int AB = perm[A + 1] + Z;
	int B = perm[X + 1] + Y;
	int BA = perm[B] + Z;
	int BB = perm[B + 1] + Z;

	// Add blended results from 8 corners of cube
	double res = lerp(w, lerp(v, lerp(u, grad(perm[AA], x, y, z), grad(perm[BA], x-1, y, z)), lerp(u, grad(perm[AB], x, y-1, z), grad(perm[BB], x-1, y-1, z))),	lerp(v, lerp(u, grad(perm[AA+1], x, y, z-1), grad(perm[BA+1], x-1, y, z-1)), lerp(u, grad(perm[AB+1], x, y-1, z-1),	grad(perm[BB+1], x-1, y-1, z-1))));
	res = (res + 1.0)/2.0;


	//res = glm::clamp(res, clipMin, clipMax);
	if (res < clipMin) res = clipMin;
	else if (res > clipMax) res = clipMax;
	res = ((res - clipMin) / (clipMax - clipMin)) * (valMax - valMin) + valMin;

/*
	res = ((res - clipMin) / (clipMax - clipMin)) * (valMax - valMin) + valMin;
	*/

	// return the result
	return res;

}



    // individual generators to sum
    List<PerlinGen*> pgens;

    // construct one
LayeredGen::LayeredGen() {

}

// add a layer to the result, returning the index
int LayeredGen::addLayer(PerlinGen* pgen) {
	int idx = pgens.size();
	pgens.push_back(pgen);
	return idx;
}

// return the ith layer
PerlinGen* LayeredGen::getLayer(int idx) {
	return pgens[idx];
}

double LayeredGen::noise(double x, double y, double z) {

	double res = 0.0;
	for (PerlinGen* pg : pgens) {
		res += pg->noise(x, y, z);
	}

	return res;
}


};
