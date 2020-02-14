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
    pmgen.addLayer(Random::Perlin(seed++, vec3(0.002), vec2(0.3, 0.65), vec2(30, 80)));
    pmgen.addLayer(Random::Perlin(seed++, vec3(0.02), vec2(0.2, 0.9), vec2(0, 20)));
    pmgen.addLayer(Random::Perlin(seed++, vec3(0.002, .03, 0.0), vec2(0.7, 0.72), vec2(0, -40)));

    cavegen = Random::PerlinMux();
    //cavegen.addLayer(Random::Perlin(seed++, vec3(0.03, 0.09, 0.03)));

}

Chunk* DefaultWG::getChunk(ChunkID id) {

    // create a new chunk pointer
    Chunk* res = new Chunk();

    // set the ID of the chunk
    res->XZ = id;

    // first, do default terrain pass
    int x, y, z;

    for (x = 0; x < CHUNK_SIZE_X; ++x) {
        for (z = 0; z < CHUNK_SIZE_Z; ++z) {
            // for now, just a basic Perlin noise generator
            // in the future, perhaps have a class that is a PerlinMuxGen, to generate combined perlin noise
            //int stone_h = layerGen->noise(id.X * CHUNK_SIZE + x, id.Z * CHUNK_SIZE + z, 0.5) + 20;
            int stone_h = pmgen.noise2d(id.X * CHUNK_SIZE_X + x, id.Z * CHUNK_SIZE_Z + z);
            if (stone_h < 3) stone_h = 3;

            int dirt_h = stone_h + 2;

            // just set the stone data
            for (y = 0; y < stone_h; ++y) {
                res->set(x, y, z, BlockData(ID::STONE));
            }

            for (; y < dirt_h; ++y) {
                res->set(x, y, z, BlockData(ID::DIRT));
            }
            for (; y < dirt_h + 1 && y > 15; ++y) {
                res->set(x, y, z, BlockData(ID::DIRT_GRASS));
            }
            
            // set the rest to air
            //while (y++ < CHUNK_HEIGHT) res->set(x, y, z, BlockInfo(ID::NONE));
        }
    }
    // now, do cave pass


    for (x = 0; x < CHUNK_SIZE_X; ++x) {
        for (z = 0; z < CHUNK_SIZE_Z; ++z) {

            for (y = 10; y < 50; ++y) {
                double smp = cavegen.noise3d(id.X * CHUNK_SIZE_X + x, y, id.Z * CHUNK_SIZE_Z + z);
                if (smp > 0.6) {
                    // clear it out
                    res->set(x, y, z, BlockData(ID::AIR));
                }
            }
        }
    }




    return res;


}


};




