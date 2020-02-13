/* WG/Default.cc - implementation of the default world generator for Blok */

// include the world generator protocol
#include <Blok/WG.hh>

namespace Blok::WG {


// construct given seed
DefaultWG::DefaultWG(uint32_t seed) {
    this->seed = seed;

    // create a muxer
    pmgen = Random::PerlinMux();

    // add a basic layer
    pmgen.addLayer(Random::Perlin(seed, vec3(0.1, 0.1, 0.1), vec2(0.2, 0.6), vec2(10, 40)));

}

Chunk* DefaultWG::getChunk(ChunkXZ id) {

    // create a new chunk pointer
    Chunk* res = new Chunk();

    int x, y, z;
    for (x = 0; x < CHUNK_SIZE_X; ++x) {
        for (z = 0; z < CHUNK_SIZE_Z; ++z) {
            // for now, just a basic Perlin noise generator
            // in the future, perhaps have a class that is a PerlinMuxGen, to generate combined perlin noise
            //int stone_h = layerGen->noise(id.X * CHUNK_SIZE + x, id.Z * CHUNK_SIZE + z, 0.5) + 20;
            int stone_h = pmgen.noise2d(id[0] * CHUNK_SIZE_X + x, id[1] * CHUNK_SIZE_Z + z);

            // just set the stone data
            for (y = 0; y < stone_h; ++y) {
                res->set(x, y, z, BlockData(ID::STONE));
            }

            /*
            int dirt_h = stone_h + 4;
            for (; y < dirt_h; ++y) {
                res->set(x, y, z, BlockInfo(ID::DIRT));
            }

            for (; y < dirt_h + 1; ++y) {
                res->set(x, y, z, BlockInfo(ID::DIRT_GRASS));
            }*/
            
            // set the rest to air
            //while (y++ < CHUNK_HEIGHT) res->set(x, y, z, BlockInfo(ID::NONE));
        }
    }

    return res;


}


};




