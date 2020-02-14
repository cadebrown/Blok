/* Chunk.cc - implementation of various chunk routines */


#include <Blok/Blok.hh>

namespace Blok {


void Chunk::calcVBO() {
    // temporary variable for the current block
    BlockData cur;

    // reset all the blocks that must be rendered, and recalculate them
    rcache.renderBlocks.clear();

    // get the base position of the bottom left back corner
    vec3 chunk_pos = getWorldPos();

    /* render the highest and lowest layer in the chunk */

    // always render the very top layer and very bottom of each layer
    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
            cur = get(x, 0, z);
            if (cur.id) rcache.renderBlocks.push_back({ chunk_pos + vec3(x, 0, z), cur });

            cur = get(x, CHUNK_SIZE_Y - 1, z);
            if (cur.id) rcache.renderBlocks.push_back({ chunk_pos + vec3(x, CHUNK_SIZE_Y- 1, z), cur });
        }
    }

    /* render all the sides, using rcache.cLose chunks if they are also being rendered */

    if (rcache.cR != NULL) {
        // we are also rendering a chunk to the right, so do special detercache.cTion check here,

        // just compute at the right plane
        int x = CHUNK_SIZE_X - 1;

        // check the right side
        for (int z = 1; z < CHUNK_SIZE_X - 1; ++z) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                cur = get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any dirercache.cTion
                bool hasX = get(x-1, y, z).id != ID::AIR && rcache.cR->get(0, y, z).id != ID::AIR;
                bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
                bool hasZ = get(x, y, z-1).id != ID::AIR && get(x, y, z+1).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

    } else {
        int x = CHUNK_SIZE_X - 1;
        for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = get(x, y, z);
                if (cur.id == ID::AIR) continue;

                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }
    }

    if (rcache.cT != NULL) {
        // we are also rendering a chunk to the right, so do special detercache.cTion check here,

        // just compute at the right plane
        int z = CHUNK_SIZE_Z - 1;

        // check the entire right side (minus very top and bottom)
        for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                cur = get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any dirercache.cTion
                bool hasX = get(x-1, y, z).id != ID::AIR && get(x+1, y, z).id != ID::AIR;
                bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
                bool hasZ = get(x, y, z-1).id != ID::AIR && rcache.cT->get(x, y, 0).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

    } else {
        int z = CHUNK_SIZE_Z - 1;
        for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = get(x, y, z);
                if (cur.id == ID::AIR) continue;

                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }
    }

    if (rcache.cL != NULL) {
        // we are also rendering a chunk to the right, so do special detercache.cTion check here,

        // just compute at the right plane
        int x = 0;

        // check the entire right side (minus very top and bottom)
        for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                // add [x, 0, z] and [x, HEIGHT-1, z] to the render stack
                cur = get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any dirercache.cTion
                bool hasX = rcache.cL->get(CHUNK_SIZE_X-1, y, z).id != ID::AIR && get(x+1, y, z).id != ID::AIR;
                bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
                bool hasZ = get(x, y, z-1).id != ID::AIR && get(x, y, z+1).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

    } else {
        int x = 0;
        for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = get(x, y, z);
                if (cur.id == ID::AIR) continue;

                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }
    }

    if (rcache.cB != NULL) {
        // we are also rendering a chunk to the right, so do special detercache.cTion check here,

        // just compute at the right plane
        int z = 0;

        // check the entire right side (minus very top and bottom)
        for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

                cur = get(x, y, z);

                // skip empty blocks
                if (cur.id == ID::AIR) continue;

                // compute whether there are any blocks blocking us in any dirercache.cTion
                bool hasX = get(x-1, y, z).id != ID::AIR && get(x+1, y, z).id != ID::AIR;
                bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
                bool hasZ = rcache.cB->get(x, y, CHUNK_SIZE_Z-1).id != ID::AIR && get(x, y, z+1).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }

    } else {
        int z = 0;
        for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                
                // check and make sure the current block is renderable and visible
                cur = get(x, y, z);
                if (cur.id == ID::AIR) continue;

                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }
    }

    /* Now, handle the 4 corners */

    // +X,+Z corner
    if (rcache.cR != NULL && rcache.cT != NULL) {
        int x = CHUNK_SIZE_X - 1, z = CHUNK_SIZE_Z - 1;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

            cur = get(x, y, z);

            // skip empty blocks
            if (cur.id == ID::AIR) continue;

            // compute whether there are any blocks blocking us in any dirercache.cTion
            bool hasX = get(x-1, y, z).id != ID::AIR && rcache.cR->get(0, y, z).id != ID::AIR;
            bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
            bool hasZ = get(x, y, z-1).id != ID::AIR && rcache.cT->get(x, y, 0).id != ID::AIR;

            // if there is any chance of visibility
            if (!(hasX && hasY && hasZ)) {
                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
            /*
            // check and make sure the current block is renderable and visible
            cur = get(x, y, z);
            if (cur.id == ID::AIR) continue;

            blocks.push_back({ chunk_pos + vec3(x, y, z), cur });*/
        }

    } else {
        int x = CHUNK_SIZE_X - 1, z = CHUNK_SIZE_Z - 1;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
            
            // check and make sure the current block is renderable and visible
            cur = get(x, y, z);
            if (cur.id == ID::AIR) continue;

            rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
        }
    }

    // -X,+Z corner
    if (rcache.cL != NULL && rcache.cT != NULL) {
        int x = 0, z = CHUNK_SIZE_Z - 1;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

            cur = get(x, y, z);

            // skip empty blocks
            if (cur.id == ID::AIR) continue;

            // compute whether there are any blocks blocking us in any dirercache.cTion
            bool hasX = rcache.cL->get(CHUNK_SIZE_X-1, y, z).id != ID::AIR && get(x, y, z).id != ID::AIR;
            bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
            bool hasZ = get(x, y, z-1).id != ID::AIR && rcache.cT->get(x, y, 0).id != ID::AIR;

            // if there is any chance of visibility
            if (!(hasX && hasY && hasZ)) {
                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

    } else {
        int x = 0, z = CHUNK_SIZE_Z - 1;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
            
            // check and make sure the current block is renderable and visible
            cur = get(x, y, z);
            if (cur.id == ID::AIR) continue;

            rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
        }
    }

    // -X,-Z corner
    if (rcache.cL != NULL && rcache.cB != NULL) {
        int x = 0, z = 0;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

            cur = get(x, y, z);

            // skip empty blocks
            if (cur.id == ID::AIR) continue;

            // compute whether there are any blocks blocking us in any dirercache.cTion
            bool hasX = rcache.cL->get(CHUNK_SIZE_X-1, y, z).id != ID::AIR && get(x, y, z).id != ID::AIR;
            bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
            bool hasZ = rcache.cB->get(x, y, CHUNK_SIZE_Z-1).id != ID::AIR && get(x, y, z+1).id != ID::AIR;

            // if there is any chance of visibility
            if (!(hasX && hasY && hasZ)) {
                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

    } else {
        int x = 0, z = 0;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
            
            // check and make sure the current block is renderable and visible
            cur = get(x, y, z);
            if (cur.id == ID::AIR) continue;

            rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
        }
    }


    // +X,-Z corner
    if (rcache.cR != NULL && rcache.cB != NULL) {
        int x = CHUNK_SIZE_X - 1, z = 0;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {

            cur = get(x, y, z);

            // skip empty blocks
            if (cur.id == ID::AIR) continue;

            // compute whether there are any blocks blocking us in any dirercache.cTion
            bool hasX = get(x-1, y, z).id != ID::AIR && rcache.cR->get(0, y, z).id != ID::AIR;
            bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
            bool hasZ = rcache.cB->get(x, y, CHUNK_SIZE_Z-1).id != ID::AIR && get(x, y, z+1).id != ID::AIR;

            // if there is any chance of visibility
            if (!(hasX && hasY && hasZ)) {
                rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
            }
        }

    } else {
        int x = CHUNK_SIZE_X - 1, z = 0;

        // check up the corner
        for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
            
            // check and make sure the current block is renderable and visible
            cur = get(x, y, z);
            if (cur.id == ID::AIR) continue;

            rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
        }
    }


    /* now, handle all the interior blocks that haven't been handled so far 
        * 
        * The benefit is that all basic culling operations operate solely within the main chunk
        * 
        */
    for (int x = 1; x < CHUNK_SIZE_X - 1; ++x) {
        for (int z = 1; z < CHUNK_SIZE_Z - 1; ++z) {
            for (int y = 1; y < CHUNK_SIZE_Y - 1; ++y) {
                cur = get(x, y, z);

                // skip block
                if (cur.id == ID::AIR) continue;


                // compute whether there are any blocks blocking us
                bool hasX = get(x-1, y, z).id != ID::AIR && get(x+1, y, z).id != ID::AIR;
                bool hasY = get(x, y-1, z).id != ID::AIR && get(x, y+1, z).id != ID::AIR;
                bool hasZ = get(x, y, z-1).id != ID::AIR && get(x, y, z+1).id != ID::AIR;

                // if there is any chance of visibility
                if (!(hasX && hasY && hasZ)) {
                    rcache.renderBlocks.push_back({ chunk_pos + vec3(x, y, z), cur });
                }
            }
        }
    }    
    
    // TODO: add a per-chunk VBO system
    // For now, just build up the VBO here
    List<vec3> positions;
    List<float> ids;

    for (auto ritem : rcache.renderBlocks) {
        positions.push_back(ritem.first);
        ids.push_back((float)ID(ritem.second.id));
    }


    // now, update the OpenGL buffer
    glBindBuffer(GL_ARRAY_BUFFER, rcache.glVBO_blocks);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindBuffer(GL_ARRAY_BUFFER, rcache.glVBO_ids);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ids.size(), &ids[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 


}


};

