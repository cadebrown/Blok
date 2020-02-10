/* Blok-Server.hh - the definition of the server/engine */

#pragma once
#ifndef BLOCC_SERVER_HH__
#define BLOCC_SERVER_HH__

#include <Blok.hh>

namespace Blok {

    class World {
        public:

        // cache of already existing chunks
        Map<ChunkID, Chunk*> cache;

        // the generator the world comes from
        WorldGenerator* gen;

        World() {
            gen = new DefaultWorldGenerator();
        }

        String getName() {
            return "world";
        }


        Chunk* getChunk(ChunkID chunk) {
            auto got = cache.find(chunk);

            if (got == cache.end()) {
                
                Chunk* new_chunk = gen->getChunk(chunk);
                return cache[chunk] = new_chunk;

            } else {
                // it already exists, so return it
                return got->second;
            }

        }

        ~World() {
            delete gen;
        }

    };

    class Server {
        public:

        // a map of all existing worlds in the server
        Map<String, World*> worlds;

        // the loaded chunks in the world
        Map<Pair<World*, ChunkID>, Chunk*> loaded;

        Server() {

        }

        // ensure the chunk is loaded
        void loadChunk(World* world, ChunkID chunk) {
            if (loaded.find({ world, chunk }) == loaded.end()) {
                b_trace("Loaded chunk %s<%i,%i>", world->getName().c_str(), (int)chunk.X, (int)chunk.Z);
                // not found in loaded, so generate/load it
                loaded[{ world, chunk }] = world->getChunk(chunk);
            }
        }

        // ensure the chunk is out of memory, unloaded
        void unloadChunk(World* world, ChunkID chunk) {
            if (loaded.find({ world, chunk }) == loaded.end()) {
                // there was nothing
                return;
            }

            // first, grab it
            Chunk* val = loaded[{ world, chunk }];

            // remove from the loaded chunks list
            loaded.erase({ world, chunk });

            // and free the resources
            delete val;

        }

        void addWorld(String name, World* world) {
            worlds[name] = world;
        }

    };


};


#endif

