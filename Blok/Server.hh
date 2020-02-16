/* Server.hh - implementation/definition of the server protocol */

#pragma once

#ifndef BLOK_SERVER_HH__
#define BLOK_SERVER_HH__

// general Blok library
#include <Blok/Blok.hh>

// we use the WG protocol for generating worlds
#include <Blok/WG.hh>

namespace Blok {

    // Server - an abstract class describing a server protocol
    class Server {
        public:

        // this is a map of all currently loaded chunks by the server
        Map<ChunkID, Chunk*> loaded;

        // allow for virtual deconstructors
        virtual ~Server() { }

        // getChunk should return a chunk by an ID, possibly generating it if required
        // NOTE: This may return NULL for some networked servers, so be sure and check if its NULL!
        //   if it is, within the next few frames, when the network request goes through, it should start returning
        //   the actual chunk
        virtual Chunk* getChunk(ChunkID id) = 0;

        // get a chunk from the server, if it is already loaded
        // else, return NULL
        virtual Chunk* getChunkIfLoaded(ChunkID id) = 0;

        // raycast() should seek through all possible chunks, checking intersection along 'ray',
        //   up to 'maxDist'. If it ends up hitting a solid block, return true and set all the 'to*'
        //   arguments to the data about the hit
        virtual bool raycast(Ray ray, float maxDist, vec3& toPos, vec3& toNormal, BlockData& toData) = 0;

    };

    // LocalServer - a server implementation that operates locally (i.e. on the current machine,
    //   no network connections)
    class LocalServer : public Server {
        public:

        // a structure describing statistics of performance
        struct {

            // the number of chunks generated
            int n_chunks;

            // the total time spent generating chunks
            double t_chunks;

        } stats;

        // the world generator for generating chunks
        WG::WG* worldGen;

        // construct a new server
        LocalServer() {
            worldGen = new WG::DefaultWG(0);

            // initialize statistics
            stats.n_chunks = 0;
            stats.t_chunks = 0.0;
        }
        
        // get a chunk from the server
        Chunk* getChunk(ChunkID id) {
            if (loaded.find(id) == loaded.end()) {
                // the chunk was not found in the loaded chunks, so generate it now
                double stime = getTime();
                Chunk* new_chunk = worldGen->getChunk(id);
                stime = getTime() - stime;
                if (new_chunk == NULL) {
                    blok_warn("Error generating chunk <%d,%d>", id.X, id.Z);
                    return NULL;
                } else {
                    stats.n_chunks++;
                    stats.t_chunks += stime;
                    return loaded[id] = new_chunk;
                }
            } else {
                return loaded[id];
            }

        }

        // get a chunk from the server, if it is already loaded
        // else, return NULL
        Chunk* getChunkIfLoaded(ChunkID id) {
            if (loaded.find(id) == loaded.end()) {
                return NULL;
            } else {
                return loaded[id];
            }
        }

        // raycast() should seek through all possible chunks, checking intersection along 'ray',
        //   up to 'maxDist'. If it ends up hitting a solid block, return true and set all the 'to*'
        //   arguments to the data about the hit
        bool raycast(Ray ray, float maxDist, vec3& toLoc, vec3& toNormal, BlockData& toData);
    };



}


#endif /* BLOK_WG_HH__ */
