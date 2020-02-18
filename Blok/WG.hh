/* WG.hh - definition of the WorldGenerator protocol
 *
 * The world generator protocol is quite simple:
 *   - Construct given a 32 bit unsigned integer seed (which is typically given as
 *       a hex value)
 *   - getChunk() returns a new chunk (i.e. newly allocated) filled with everything
 * 
 * World Generators can have internal state (i.e. caching, list of worms for cave generation, 
 *   tree generation, etc)
 * 
 * 
 * 
 */

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

        // method to generate a single chunk, given the ChunkID macro coordinates
        // the position of the given chunk is CHUNK_SIZE * cx, 0 through CHUNK_HEIGHT, CHUNK_SIZE * cz
        virtual Chunk* getChunk(ChunkID id) = 0;

    };


    // DefaultWG - the default world generator used by Blok.
    // See the file `WG/Default.cc` for the implmentation
    class DefaultWG : public WG {
        public:

        // perlin noise generator
        Random::PerlinMux pmgen;

        // cave generator
        Random::PerlinMux cavegen;

        // construct given a seed
        DefaultWG(uint32_t seed=0);

        // generate a chunk from a given ChunkID
        Chunk* getChunk(ChunkID id);

    };


    // FlatWG - a 'flat' world generator, with constant, unchanging layers, which can be set by
    // "addLayer()"
    // See the file `WG/Flat.cc` for the implmentation
    class FlatWG : public WG {
        public:

        // list of blockID's and size of the layers, starting from the floor
        List< Pair<ID, int> > layers;

        // construct given a seed (seed is never used)
        FlatWG(uint32_t seed=0);

        // generate a chunk from a given ChunkID
        Chunk* getChunk(ChunkID id);

    };


}


#endif /* BLOK_WG_HH__ */
