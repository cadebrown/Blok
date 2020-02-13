/* Blok-Generator.cc - world generator routines */

#include <Blok-Generator.hh>

// for randomness
#include <Blok-Random.hh>

namespace Blok {
DefaultWorldGenerator::DefaultWorldGenerator(uint seed) {
    this->layerGen = new Random::LayeredGen();
    //this->layerGen->addLayer(new Random::PerlinGen(seed+1, vec3(.01), 0, 1, 5, 15));
    this->layerGen->addLayer(new Random::PerlinGen(seed, vec3(.012, .012, 1.0), 0, 1, 10, 8));
    this->layerGen->addLayer(new Random::PerlinGen(seed, vec3(.1, .1, 1.0), 0.5, 0.9, 0, 10));
    this->layerGen->addLayer(new Random::PerlinGen(seed, vec3(.008, .008, 1.0), 0.6, 0.65, 0, 25));
}


Chunk* DefaultWorldGenerator::getChunk(ChunkID id) {
    // for now, just return a very basic chunk with Y <= 20 being stone, and everything else being empty
    Chunk* res = new Chunk(BIOME_NONE);
    

    int x, y, z;
    for (x = 0; x < CHUNK_SIZE; ++x) {
        for (z = 0; z < CHUNK_SIZE; ++z) {
            // for now, just a basic Perlin noise generator
            // in the future, perhaps have a class that is a PerlinMuxGen, to generate combined perlin noise
            int stone_h = layerGen->noise(id.X * CHUNK_SIZE + x, id.Z * CHUNK_SIZE + z, 0.5) + 20;
            for (y = 0; y < stone_h; ++y) {
                res->set(x, y, z, BlockInfo(ID::STONE));
            }
            int dirt_h = stone_h + 4;
            for (; y < dirt_h; ++y) {
                res->set(x, y, z, BlockInfo(ID::DIRT));
            }

            for (; y < dirt_h + 1; ++y) {
                res->set(x, y, z, BlockInfo(ID::DIRT_GRASS));
            }
            
            // set the rest to air
            while (y++ < CHUNK_HEIGHT) res->set(x, y, z, BlockInfo(ID::NONE));
        }
    }

    return res;
}


};
