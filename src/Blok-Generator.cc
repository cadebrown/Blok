/* Blok-Generator.cc - world generator routines */

#include <Blok-Generator.hh>

// for randomness
#include <Blok-Random.hh>

namespace Blok {
DefaultWorldGenerator::DefaultWorldGenerator(uint seed) {
    this->perlinGen = new Random::PerlinGen();
}


Chunk* DefaultWorldGenerator::getChunk(ChunkID id) {
    // for now, just return a very basic chunk with Y <= 20 being stone, and everything else being empty
    Chunk* res = new Chunk(BIOME_NONE);
    
    // XZ & Y scaling
    double XZscale = 0.06, Yscale = 10.0;

    // base of the Y level
    double Ybase = 5.0;

    int x, y, z;
    for (x = 0; x < CHUNK_SIZE; ++x) {
        for (z = 0; z < CHUNK_SIZE; ++z) {
            // for now, just a basic Perlin noise generator
            // in the future, perhaps have a class that is a PerlinMuxGen, to generate combined perlin noise
            int h = Ybase + Yscale * perlinGen->noise(XZscale * (id.X * CHUNK_SIZE + x), XZscale * (id.Z * CHUNK_SIZE + z));
            for (y = 0; y < h; ++y) {
                res->set(x, y, z, BlockInfo(ID_STONE));
            }

            // set the rest to air
            while (y++ < CHUNK_HEIGHT) res->set(x, y, z, BlockInfo(ID_NONE));
        }
    }

    return res;
}


};
