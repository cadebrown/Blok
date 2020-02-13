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

    };

    // LocalServer - a server implementation that operates locally (i.e. on the current machine,
    //   no network connections)
    class LocalServer : public Server {
        public:

        // how to generate the world
        WG::WG* worldGen;

        // construct a new server
        LocalServer() {
            worldGen = new WG::DefaultWG(0);
        }
        
        Chunk* getChunk(ChunkID id) {
            if (loaded.find(id) == loaded.end()) {
                // the chunk was not found in the loaded chunks, so generate it now
                return loaded[id] = worldGen->getChunk(id);
            } else {
                return loaded[id];
            }

        }

    };



}


#endif /* BLOK_WG_HH__ */
