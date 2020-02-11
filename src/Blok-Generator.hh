/* Blok-Generator.hh - World generation routines */

#pragma once
#ifndef BLOK_GENERATOR_HH__
#define BLOK_GENERATOR_HH__

#include <Blok.hh>

namespace Blok {

    /* WorldGenerator - abstract class describing the interface for a world generator */
    class WorldGenerator {

        //Map<Pair<int, int>, Chunk*> chunk_cache;

        public:

        // method to generate a single chunk, given the chunk index x and z
        // the position of the given chunk is CHUNK_SIZE * cx, 0 through CHUNK_HEIGHT, CHUNK_SIZE * cz
        virtual Chunk* getChunk(ChunkID id) = 0;


    };

    /* DefaultWorldGenerator - implementation of the WorldGenerator protocol, for a nice default world */
    class DefaultWorldGenerator : public WorldGenerator {
        public:

        // generate a chunk from a given ChunkID
        Chunk* getChunk(ChunkID id);

    };

};


#endif

