/* WG.hh - definition of the WorldGenerator protocol */

#pragma once

#ifndef BLOK_WG_HH__
#define BLOK_WG_HH__

// general Blok library
#include <Blok/Blok.hh>

// generators use the randomness library
#include <Blok/Random.hh>

namespace Blok::WG {

    // WG - abstract class describing a world WorldGenerator
    class WG {
        public:

        // the seed for the world generator
        uint32_t seed;


        // construct a world generator from the seed
        WG(uint32_t seed=0) {
            this->seed = seed;
        }
        
        virtual ~WG() {
            // do nothing by default, so that C++ is okay with virtual destructors on abstract classes
        }


        // method to generate a single chunk, given the ChunkXZ macro coordinates
        // the position of the given chunk is CHUNK_SIZE * cx, 0 through CHUNK_HEIGHT, CHUNK_SIZE * cz
        virtual Chunk* getChunk(ChunkXZ id) = 0;
    };


    // DefaultWG - the default world generator used by Blok.
    // See the file `WG/Default.cc` for the implmentation

    class DefaultWG : public WG {
        public:

        // perlin noise generator
        Random::PerlinMux pmgen;

        // construct given a seed
        DefaultWG(uint32_t seed=0);

        // generate a chunk from a given ChunkID
        Chunk* getChunk(ChunkXZ id);

    };


}


#endif /* BLOK_WG_HH__ */
