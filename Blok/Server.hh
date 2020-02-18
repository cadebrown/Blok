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

        // list of ChunkIDs that have been requested to load by the server
        // none of these should be in 'loaded'
        Set<ChunkID> chunkRequests;

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

        // this virtual function allows the server to process chunk requests (i.e. load in chunks from a data-base,
        //   generate them, etc) for a maximum amount of time
        // it should return the actual number of requests processed
        virtual int processChunkRequests(double maxTime) = 0;

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
            //worldGen = new WG::FlatWG(0);

            // initialize statistics
            stats.n_chunks = 0;
            stats.t_chunks = 0.0;
        }

        // gets a chunk from the server, marking it for creation if it does not yet exist
        Chunk* getChunk(ChunkID id) {
            if (loaded.find(id) == loaded.end()) {

                // just request this ChunkID, the server will generate/load it eventually, but for now return NULL
                chunkRequests.insert(id);
                return NULL;
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

        // process the chunks
        int processChunkRequests(double maxTime) {
            int ct = 0;
            double et = 0.0;
            while (et < maxTime && chunkRequests.size() > 0) {
                double st = getTime();

                // now, process a single chunkID
                ChunkID creq = *chunkRequests.begin();
                chunkRequests.erase(chunkRequests.begin());

                // now, actually compute it
                Chunk* new_chunk = worldGen->getChunk(creq);

                // upload it to loaded chunks
                loaded[creq] = new_chunk;

                // update vars
                et += getTime() - st;
                ct++;
            }
            return ct;
        }

        // raycast() should seek through all possible chunks, checking intersection along 'ray',
        //   up to 'maxDist'. If it ends up hitting a solid block, return true and set all the 'to*'
        //   arguments to the data about the hit
        bool raycastBlock(Ray ray, float dist, RayHit& hitInfo);

    };



}


#endif /* BLOK_WG_HH__ */
