/* WG/Flat.cc - implementation of the flat world generator, similar to Minecraft's
 *
 */

// include the world generator protocol
#include <Blok/WG.hh>

namespace Blok::WG {

// construct given seed
FlatWG::FlatWG(uint32_t seed) {
    this->seed = seed;

    // always include a layer of stone (TODO: bedrock)
    layers.push_back({ID::STONE, 60});
    layers.push_back({ID::DIRT, 20});
    layers.push_back({ID::DIRT_GRASS, 1});

}

// generate a single chunk
Chunk* FlatWG::getChunk(ChunkID id) {

    // create a new chunk pointer
    Chunk* res = new Chunk();

    // set the ID of the chunk
    res->XZ = id;

    // current coordinates
    int y = 0;

    int lidx = 0;
    while (y < CHUNK_SIZE_Y && lidx < layers.size()) {
        // get the current layer
        auto& layer = layers[lidx];
        for (int x = 0; x < CHUNK_SIZE_X; ++x) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                for (int ly = 0; ly < layer.second && y + ly < CHUNK_SIZE_Y; ++ly) {
                    // set it to the block
                    res->set(x, y+ly, z, {layer.first});
                }
            }
        }

        // move the Y up
        y += layer.second;
        // move forward in the layers
        lidx++;
    }

    return res;
}


};




