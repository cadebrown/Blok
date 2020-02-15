/* Blok.hh - main header file for the 'Blok' game. This file describes the basic 
 *   data structures
 *
 * @author: Cade Brown <brown.cade@gmail.com>
 */


#pragma once
#ifndef BLOK_HH__
#define BLOK_HH__


/* C libraries */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* C++ standard datatypes */
#include <vector>
#include <string>
#include <map>

/* GLM (matrix & vector library) */
#include <Blok/glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <Blok/glm/gtx/hash.hpp>

/* gl3w (OpenGL loader) */
#include <Blok/gl3w/gl3w.h>

/* GLFW (Window Library) */
#include <GLFW/glfw3.h>


/* freetype (FontLoading) */
#include <ft2build.h>
#include FT_FREETYPE_H


/* PortAudio (Audio Input/Output Library) */
#include "portaudio.h"

// define this operator so that XZs can be used as keys
bool operator<(const glm::ivec2 A, const glm::ivec2 B);


namespace Blok {

    /* TYPE DEFINITIONS */

    // populate some basic vector data types from GLM, a useful header-only library
    // these are all float types
    using vec2 = glm::vec2;
    using vec3 = glm::vec3;
    using vec4 = glm::vec4;
    using mat2 = glm::mat2;
    using mat3 = glm::mat3;
    using mat4 = glm::mat4;


    // popualte some types as vectors of integer coordinates (always append 'i' to these)
    using vec2i = glm::vec<2, int>;
    using vec3i = glm::vec<3, int>;


    // a type to store a string value, i.e. a character array
    // just use the standard definition, which is good for most
    // use cases
    using String = std::string;

    // a generic type that couples 2 other types together
    template<typename A, typename B>
    using Pair = std::pair<A, B>;

    // a template for a list of a given type, i.e. List<int>
    // I prefer this to the 'vector' name, because we are already
    // using small, fixed size collections called 'vectors' from GLM,
    // so calling things list is clearer when talking about 3D codes
    template<typename T>
    using List = std::vector<T>;

    // a template for a dictionary type, i.e. Map<String, int>
    // Just use the standard library's definition
    // NOTE: std::unordered_map is normally faster, but it causes when,
    //   for example, using Pair<A, B> as a key type, which I do often.
    // there may be a fix, but it's not that drastic of a performance difference
    // so I think this is simpler
    template<typename K, typename V>
    using Map = std::map<K, V>;


    /* CONSTANTS */

    // current build number
    const int BUILD_MAJOR = 0;
    const int BUILD_MINOR = 0;
    const int BUILD_PATCH = 1;

    // whether or not the build is a development build,
    // or an official build
    const bool BUILD_DEV = true;
    

    // number of blocks in the X direction (left/right)
    const int CHUNK_SIZE_X = 16;
    // number of blocks in the Y direction (up/down)
    const int CHUNK_SIZE_Y = 256;
    // number of blocks in the Z direction (forward/backward)
    const int CHUNK_SIZE_Z = 16;

    // the total number of blocks in a chunk
    const int CHUNK_NUM_BLOCKS = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;


    /* SINGLETONS */

    // this is the global FreeType library we use for loading fonts.
    // initialization is in Blok.cc
    extern FT_Library ftlib;


    /* GAME ENGINE SPECIFIC TYPE DEFS */

    // ID - describes a numeric identifier for the type of block
    enum ID : uint8_t {

        // AIR - an empty block, is transparent, and non-physical
        //   basically just a placeholder block
        AIR        = 0,

        // DIRT - a cube made of dirt, with identical textures on all sides
        //   is completely opaque, and is physical
        DIRT       = 1,

        // DIRT_GRASS - a cube made of dirt, with grass on the top side
        //   is completely opaque, and is physical
        DIRT_GRASS = 2,

        // STONE - a cube of stone, same on all sides
        //   is completely opaque, and is physical
        STONE      = 3,

    };


    // BlockData - data for a single block in the world, which includes the type of
    //   block, as well as room for meta data
    struct BlockData {

        // the block ID for this current block
        ID id;

        // various metadata
        uint8_t meta;

        // construct a BlockData from parameters, defaulting to an empty block
        BlockData(ID id=ID::AIR, uint8_t meta=0) {
            this->id = id;
            this->meta = meta;
        }

    };

    // ChunkID - type defining the Chunk's macro coordinates world space
    // The actual world XZ is given by CHUNK_SIZE_X * XZ.X and CHUNK_SIZE_Z * XZ.Z
    struct ChunkID {
        int X, Z;

        ChunkID(int X=0, int Z=0) {
            this->X = X;
            this->Z = Z;
        }
    };

    // elementwise operators
    ChunkID operator+(ChunkID A, ChunkID B);
    ChunkID operator-(ChunkID A, ChunkID B);

    // so that ChunkIDs are well ordered
    bool operator<(ChunkID A, ChunkID B);

    // Chunk - represents a vertical column of data of size:
    //   CHUNK_SIZE_X*CHUNK_SIZE_Y*CHUNK_SIZE_Z
    // This should extend from the bottom of the physical world to the top,
    //   and is indexed starting at 0 in all local directions (x, y, z)
    // There are also the macro coordinates (type: ) (X, Z), which describe
    //   the overall position in the 2D lattice on the XZ plane:
    // view the chunk as top down, with +X being right, +Z being up, etc
    // like so:
    // +-----+-----+-----+
    // |     |  T  |     |
    // |     |     |     |
    // +-----+-----+-----+
    // |  L  | cur |  R  |
    // |     |     |     |
    // +-----+-----+-----+
    // |     |  B  |     |
    // |     |     |     |
    // +-----+-----+-----+
    //
    // (Z)
    //  ^
    //  + > (X)
    //
    // If the macro coordinates for 'cur' are X=0,Z=0, then that means the back left bottom
    //   blocks are 0,0,0 in world space
    // The back left bottom of a chunk is given by: (CHUNK_SIZE_X*X, 0, CHUNK_SIZE_Z*Z)
    //   and it extends through: (CHUNK_SIZE_X*(X+1), CHUNK_SIZE_Y, CHUNK_SIZE_Z*(Z+1))
    //
    class Chunk {
        public:

        // the macro coordinates, i.e. 2D lattice index of the Chunk
        ChunkID XZ;

        // the array of blocks that make up the chunk, they are ordered in XZY order,
        // i.e. the the Y coordinates are the fastest changing
        // The index (x, y, z) maps to the linear index (CHUNK_SIZE_Y * (CHUNK_SIZE_Z * x + z) + y)
        //  so, for loop iteration should be like:
        // for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        //   for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
        //     for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
        //       chunk->set(x, y, z, ...);
        //     }
        //   }
        // }
        BlockData* blocks;

        // rcache - the render cache, meant to be mainly managed by the rendering engine
        //   to improve efficiency
        // all 'cur' values mean current as of this frame, and
        // 'last' values mean the values for last frames
        struct {

            // keep track of hashes, to check if anything changed
            uint64_t curHash, lastHash;

            bool isDirty;

            // pointers to other chunks that are spacially touching this chunk
            // NOTE: see the diagram above the definition for 'class Chunk' for a visual
            //   diagram of these
            // if one is NULL, that means that Chunk is not in the rendering engine currently,
            //   so the chunk is 'open'
            Chunk *cL, *cT, *cR, *cB;

            // vertex buffer of all the blocks, as (x:f, y:f, z:f)
            uint glVBO_blocks;

            // vertex buffer of all the blocks's IDs, as (id:f)
            uint glVBO_ids;


            List< Pair<vec3, BlockData> > renderBlocks;



        } rcache;

        // internal methods called by the rendering engine

        // calculate the glVBO_blocks used by the rendering engine
        void calcVBO();


        // construct an empty chunk, defaulting to all air blocks
        Chunk() {
            // allocate the array of data, which should set them all to empty air
            this->blocks = new BlockData[CHUNK_NUM_BLOCKS];

            // initialize the render cache
            // 0=not calculated yet
            rcache.curHash = rcache.lastHash = 0;

            rcache.isDirty = true;

            // assume there are no valid chunks to start off with
            rcache.cL = rcache.cT = rcache.cR = rcache.cB = NULL;


            // graphics init
            glGenBuffers(1, &rcache.glVBO_blocks);
            glGenBuffers(1, &rcache.glVBO_ids);

        }

        // free all resources in the chunk
        ~Chunk() {
            // free our allocated array
            delete[] this->blocks;

            // remove our neighbor's references
            rcache.cR->rcache.cL = NULL;
            rcache.cT->rcache.cB = NULL;
            rcache.cL->rcache.cR = NULL;
            rcache.cB->rcache.cT = NULL;

            // remove OpenGL handle
            glDeleteBuffers(1, &rcache.glVBO_blocks);
        }

        // calculate the hash for the chunk
        uint64_t calcHash() const {
            // I use a djb-like hash function, which seems to be working fine
            uint64_t res = 5381;
            for (int i = 0; i < CHUNK_NUM_BLOCKS; ++i) {
                // convert this into a single value
                uint32_t ctmp = (blocks[i].id << 8) |  this->blocks[i].meta;
                // now, XOR it
                res ^= (ctmp << 5) + ctmp;
            }

            // never return 0, because that means that the hash has not been initialized
            return res == 0 ? 1 : res;
        }

        // get the linear index into 'blocks' array, given the 3D local coordinates
        // i.e. 0 <= x < BLOCK_SIZE_X
        // i.e. 0 <= y < BLOCK_SIZE_Y
        // i.e. 0 <= z < BLOCK_SIZE_Z
        int getIndex(int x=0, int y=0, int z=0) const {
            return CHUNK_SIZE_Y * (CHUNK_SIZE_Z * x + z) + y;
        }

        // get the linear index into 'blocks' array, given the 3D local coordinates
        int getIndex(vec3i xyz) {
            return getIndex(xyz[0], xyz[1], xyz[2]);
        }

        // inverse the linear index, and decompose it back into individual components, x, y, z
        // NOTE: getIndexInv(getIndex(xyz)) == xyz
        vec3i getIndexInv(int idx) {
            vec3i res;
            // first, get Y component
            res[1] = idx % CHUNK_SIZE_Y;
            idx /= CHUNK_SIZE_Y;
            // then, get the Z component
            res[2] = idx % CHUNK_SIZE_Z;
            idx /= CHUNK_SIZE_Z;
            // then, the X component is the last left
            res[0] = idx % CHUNK_SIZE_X;

            // ensure they are all positive
            if (res[0] < 0) res[0] += CHUNK_SIZE_X;
            if (res[1] < 0) res[1] += CHUNK_SIZE_Y;
            if (res[2] < 0) res[2] += CHUNK_SIZE_Z;
            return res;
        }

        // get the block data at a given local coordinate
        // i.e. 0 <= x < BLOCK_SIZE_X
        // i.e. 0 <= y < BLOCK_SIZE_Y
        // i.e. 0 <= z < BLOCK_SIZE_Z
        BlockData get(int x=0, int y=0, int z=0) const {
            const int idx = getIndex(x, y, z);
            return blocks[idx];
        }

        // set the block data at a given local coordinate to a given value
        // i.e. 0 <= x < BLOCK_SIZE_X
        // i.e. 0 <= y < BLOCK_SIZE_Y
        // i.e. 0 <= z < BLOCK_SIZE_Z
        void set(int x=0, int y=0, int z=0, BlockData val=BlockData()) {
            const int idx = getIndex(x, y, z);
            blocks[idx] = val;
            rcache.isDirty = true;
        }

        // return the world coordinates of the (0, 0, 0) local position 
        vec3i getWorldPos(vec3i xyz=vec3i(0, 0, 0)) {
            return vec3i(CHUNK_SIZE_X * XZ.X, 0, CHUNK_SIZE_Z * XZ.Z) + xyz;
        }

        // return a bounding box start in world position (inclusive)
        vec3i getWorldBB0() {
            return getWorldPos({0, 0, 0});
        }

        // return a bounding box end in world position (inclusive)
        vec3i getWorldBB1() {
            return getWorldPos({CHUNK_SIZE_X-1, CHUNK_SIZE_Y-1, CHUNK_SIZE_Z-1});
        }

    };

    /* GLOBAL VARIABLES */

    // list of resource paths to try and find textures/shaders/models/etc
    extern List<String> paths;

    /* GENERAL LIBRARY FUNCTIONS */

    // this should be the first function called, in order to initialize Blok
    // returns true on sucess, false when there was a failure
    bool initAll();

    // get the current time (in seconds) since Blok has been initialized,
    //   i.e. the relative wall time
    double getTime();

    // check/handle any OpenGL errors, returning true if there were errors,
    //   false if there were none
    bool check_GL();
    

    // LogLevel - represents a severity of a log,
    //   which messages can be filtered on
    enum LogLevel {
        TRACE = 0,
        DEBUG = 1,
        INFO  = 2,
        WARN  = 3,
        ERROR = 4
    };

    // set the log level
    void setLogLevel(LogLevel level);

    // get the current log level
    LogLevel getLogLevel();

    // internal logging method, DON'T USE
    // instead, use `blok_info`, `block_trace`, etc
    void log_internal(LogLevel level, const char* file, int line, const char* fmt, ...);

    // prints a message, using the blok logging system
    #define blok_trace(...) Blok::log_internal(Blok::LogLevel::TRACE, __FILE__, __LINE__, __VA_ARGS__)
    #define blok_debug(...) Blok::log_internal(Blok::LogLevel::DEBUG, __FILE__, __LINE__, __VA_ARGS__)
    #define blok_info(...) Blok::log_internal(Blok::LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
    #define blok_warn(...) Blok::log_internal(Blok::LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
    #define blok_error(...) Blok::log_internal(Blok::LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)


};





#endif /* BLOK_HH__ */
