/* Server.hh - implementation/definition of the server protocol

The server is basically the game engine, where the client requests to change something
  or retrieve information about the game state. Since 'Server' is an abstract class, this means
  clients/mods/etc can treat a local server the same as a networked one.

The biggest problem is with networked servers, there will be a slight delay, so if a client requests 
  a chunk over a network, it definitely will NOT be available for the current frame. In that case, it
  was a valid request, and the data is present, but not currently available. In that case, the getChunk()
  method will return 'NULL'. So, the client/mods/etc should always check whether the call returned a valid
  Chunk, and not NULL

*/

#pragma once

#ifndef BLOK_SERVER_HH__
#define BLOK_SERVER_HH__

// general Blok library
#include <Blok/Blok.hh>

// we use the WG protocol for generating worlds
#include <Blok/WG.hh>

namespace Blok {

    // Server - an abstract class describing the server/game engine protocol,
    //   for management & gameplay
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

        // get a chunk from the server, if it is already loaded. This will not attempt to load/generate the chunk
        // else, return NULL
        virtual Chunk* getChunkIfLoaded(ChunkID id) = 0;

        // attempt to cast a ray (in world space), up to 'dist', returning whether or not it hit something
        // In the case that it did hit something, also set `hitInfo` to the relevant data about the collision
        // See `Blok.hh`, specifically around `struct RayHit` for more information
        virtual bool raycastBlock(Ray ray, float dist, RayHit& hitInfo) = 0;

    };

    // LocalServer - a server implementation that operates locally (i.e. on the current machine,
    //   not over a network)
    // NOTE: This is also the server running on hosted servers, but it simply allows for managed network connections
    //   to interact and request things rather than clients
    class LocalServer : public Server {
        public:

        // a structure describing statistics of performance
        struct {

            // the number of chunks generated
            int n_chunks;

            // the total time spent generating chunks
            double t_chunks;

        } stats;

        // the world generator that is currently being used to generate chunks
        WG::WG* worldGen;

        // construct a new local server.
        // For now, just create a default world generator
        LocalServer() {
            worldGen = new WG::DefaultWG(0);

            // initialize statistics
            stats.n_chunks = 0;
            stats.t_chunks = 0.0;
        }
        
        // gets a chunk from the server, creating it if not yet created
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
