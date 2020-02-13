/* Blok.hh - a minecraft-style voxel game 
 *
 * 
 *
 */

#pragma once
#ifndef BLOK_HH__
#define BLOK_HH__

/* C libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* C++ standard datatypes */
#include <vector>
#include <string>
#include <map>

/* GLM (matrix & vector library) */
#include <glm/vec3.hpp>

/* Assimp (Asset Importing Library) */
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/* GLFW (Window Library) */
#include <gl3w/gl3w.h>
#include <GLFW/glfw3.h>

/* PortAudio (Audio Input/Output Library) */
#include "portaudio.h"


namespace Blok {


    /* Other namespaces */

    // so Blok::vec is glm::vec
    using namespace glm;


    /* Global Constants */

    // the X/Z chunk size (in blocks and/or meters)
    #define CHUNK_SIZE    16

    // the height of each chunk, default to 256
    // the maximum height
    #define CHUNK_HEIGHT 256

    // number of blocks per chunk
    #define CHUNK_NBLOCK (CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT)

    /* Block IDs */
    enum ID : uint8_t {

        // 0: NONE/AIR block
        NONE       = 0,

        // 1: DIRT block
        DIRT       = 1,
        DIRT_GRASS = 2,

        STONE      = 3,

    };

    /* Biome IDs */
    enum {
        // none/default biome
        BIOME_NONE    = 0,


        BIOME__END
    };

    // list of a given type
    template<typename T>
    using List = std::vector<T>;

    // mapping from one type to another
    template<typename K, typename V>
    using Map = std::map<K, V>;

    // mapping from one type to another
    template<typename A, typename B>
    using Pair = std::pair<A, B>;

    // the string type
    using String = std::string;

    // a chunk ID is the x, z macro coordinates
    struct ChunkID { int X; int Z; };

    // so that ChunkIDs are well ordered
    bool operator<(ChunkID A, ChunkID B);

    // list of resource paths to try and find textures
    extern List<String> paths;

    /* FORWARD DECLARATIONS */

    class Entity;


    // BlockConstProperties - global information about a given block type,
    // not an instance
    struct BlockConstProp {
        // the ID of the block
        ID id;

        // the enum value name
        String id_name;

        // the human readable name
        String hr_name;

    };

    // return the name of a given block ID
    const char* id_to_name(ID id);

    // return the ID from a given name, AIR if not found
    ID id_from_name(const char* name);


    // information of a single block in the world
    struct BlockInfo {

        // the ID of the block (see ID_* enum values)
        uint8_t id;  

        // the meta-data of the block (each block can interperet this differently)
        uint8_t meta;

        BlockInfo(ID id=ID::NONE, uint8_t meta=0) {
            this->id = id;
            this->meta = meta;
        }
    };

    // a single chunk of data, representing a cross section of the world
    //   of CHUNK_SIZE*CHUNK_HEIGHT*CHUNK_SIZE
    class Chunk {
        public:

        // metadata about a chunk
        struct ChunkInfo {

            // one of the BIOME_* values
            uint8_t biome;

        } meta;

        // the data. This is ordered in XZY order
        // so, to compute the index of a specific block, do:
        // x, y, z -> y + CHUNK_HEIGHT * (CHUNK_SIZE * x + z)
        BlockInfo* blocks;

        // internal dirty cache
        struct Cache {
            
            // the up-to-date rendering hash
            uint64_t curRenderHash;
            
            // the hash of the chunk last time it was rendered
            uint64_t lastRenderHash;

            // whether the chunk has changed since it was rendered
            bool isRenderDirty;

            // the result of that the renderer should render
            List< Pair<vec3, BlockInfo> > renderBlocks;


            // a 2D linked list/fence

            Chunk *cL, *cT, *cR, *cB;

        } cache;

        // construct a new data
        Chunk(uint8_t biome=BIOME_NONE) {
            this->blocks = (BlockInfo*)malloc(sizeof(*this->blocks) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT);
            this->meta.biome = biome;
            for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; ++i) {
                this->blocks[i].id = ID::NONE;
                this->blocks[i].meta = 0;
            }
            // 0 to uninitializedd
            this->cache.curRenderHash = 0;
            this->cache.lastRenderHash = 0;
            this->cache.isRenderDirty = true;

            // set all entries to NULL
            this->cache.cL = NULL;
            this->cache.cT = NULL;
            this->cache.cR = NULL;
            this->cache.cB = NULL;
        }

        // deallocation of a chunk
        ~Chunk() {
            free(this->blocks);
        }

        // calculates the current hash
        uint64_t getHash() {
            // djb-like hash function to quickly scatter values.
            // most hash changes will occur when a single block has been broken,
            // so as long as a single value is guaranteed to change it, the hash function is fine

            uint64_t res = 5381;
            for (int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_HEIGHT; ++i) {
                uint64_t ctmp = (blocks[i].id ^ this->blocks[i].meta);
                res += (ctmp << 5) + ctmp + 91;
            }

            return res;
        }

        int getIndex(int x, int y, int z) const {
            return y + CHUNK_HEIGHT * (CHUNK_SIZE * x + z);
        }

        // get the block info at given LOCAL coordinates
        BlockInfo get(int x, int y, int z) const {
            int idx = getIndex(x, y, z);
            return blocks[idx];
        }


        // set the block info at given LOCAL coordinates (if nothing is given, it default to air)
        void set(int x, int y, int z, BlockInfo info = {ID::NONE, 0}) {
            int idx = getIndex(x, y, z);
            blocks[idx] = info;
            cache.isRenderDirty = true;
        }


    };


    /* misc library functionality */

    // get the current time (in seconds) since initialization
    double getTime();

    /* logging */

    // enumeration for levels of logging, from least important to most important
    enum {
        LOG_TRACE = 0,
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,


        KS_LOG__END
    };
    
    // return the current logging level, one of KS_LOG_* enum values
    int b_log_level();

    // set the logging level to `new_level`
    void b_log_level_set(int new_level);

    // generically log given a level, the current file, line, and a C-style format string, with a list of arguments
    // NOTE: don't use this, use the macros like `ks_info`, and `ks_warn`, which make it easier to use the logging
    //   system
    void b_log(int level, const char *file, int line, const char* fmt, ...);

    // prints a trace message, assuming the current log level allows for it
    #define b_trace(...) b_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
    // prints a debug message, assuming the current log level allows for it
    #define b_debug(...) b_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
    // prints a info message, assuming the current log level allows for it
    #define b_info(...)  b_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
    // prints a warn message, assuming the current log level allows for it
    #define b_warn(...)  b_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
    // prints a error message, assuming the current log level allows for it
    #define b_error(...) b_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

    // internal method to check for any OpenGL errors that have occured
    void opengl_error_check();

}

#endif /* BLOK_HH__ */
