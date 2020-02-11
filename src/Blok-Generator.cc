/* Blok-Generator.cc - world generator routines */

#include <Blok-Generator.hh>

// for randomness
#include <Blok-Random.hh>

namespace Blok {


Chunk* DefaultWorldGenerator::getChunk(ChunkID id) {
    // for now, just return a very basic chunk with Y <= 20 being stone, and everything else being empty
    Chunk* res = new Chunk(BIOME_NONE);

    int x, y, z;
    for (x = 0; x < CHUNK_SIZE; ++x) {
        for (z = 0; z < CHUNK_SIZE; ++z) {
            int h = 6 + 10 * Random::get_f();
            for (y = 0; y < h; ++y) {
                res->get(x, y, z) = BlockInfo(ID_STONE);
            }

            // set the rest to air
            while (y++ < CHUNK_HEIGHT) res->get(x, y, z) = BlockInfo(ID_NONE);

        }
    }

    return res;
}


};
