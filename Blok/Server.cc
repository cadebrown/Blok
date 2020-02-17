/* Server.cc - implementation of the server protocol */

#include <Blok/Server.hh>

namespace Blok {

// raycast() should seek through all possible chunks, checking intersection along 'ray',
//   up to 'maxDist'. If it ends up hitting a solid block, return true and set all the 'to*'
//   arguments to the data about the hit
bool LocalServer::raycastBlock(Ray ray, float maxDist, RayHit& hitInfo) {

    // the basic algorithm is: view the entire `XZ` plane as a pixel, and use this algo: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    // along the chunks, then call the individual chunk.raycast() functions with translated coordinates

    // current x, y, z position
    float x = floorf(ray.orig.x);
    float y = floorf(ray.orig.y);
    float z = floorf(ray.orig.z);

    // the amount to change xyz by
    float dx = ray.dir.x;
    float dy = ray.dir.y;
    float dz = ray.dir.z;
    
    // invalid, just report nothing
    if (dx == 0 && dy == 0 && dz == 0) return false;

    // how to step in various directions, i.e. +1 if it is positive, -1 if it is negative, 0 if it is 0
    float stepX = glm::sign(dx);
    float stepY = glm::sign(dy);
    float stepZ = glm::sign(dz);


    // max & delta have to deal with the error term. Essentially, we want to process the minimum maximum error
    //  and update in that axis at a given time
    float tMaxX = intBound(ray.orig.x, dx);
    float tMaxY = intBound(ray.orig.y, dy);
    float tMaxZ = intBound(ray.orig.z, dz);

    // how much to change the minmax by
    float tDeltaX = stepX / dx;
    float tDeltaY = stepY / dy;
    float tDeltaZ = stepZ / dz;

    // escape radius
    float radius = maxDist / sqrtf(dx*dx+dy*dy+dz*dz);

    // the last chunk ID, to speed up/cache results
    ChunkID lastChunkID;
    
    // current chunk
    Chunk* cc = NULL;

    // by default
    //toNormal = {0, 1, 0};

    // probe until we hit an unknown chunk, or exceed the max radius
    while (true) {

        ChunkID cid = {(int)floorf(x/16.0f), floorf(z/16.0f)};

        cc = (!cc || cid != lastChunkID) ? getChunkIfLoaded(cid) : cc;
        if (!cc) return false;

        // convert to local indices
        vec3i local = vec3i(glm::floor(vec3(x, y, z))) - cc->getWorldPos();

        //printf("local:%i,%i,%i\n", local.x, local.y, local.z);

        // probe the block, and check if it is not air
        if ((hitInfo.blockData = cc->get(local.x, local.y, local.z)).id != ID::AIR) {
            //toLoc = vec3(x, y, z);
            hitInfo.hit = true;
            hitInfo.pos = vec3(x, y, z);
            hitInfo.dist = sqrtf(tMaxX*tMaxX + tMaxY*tMaxY + tMaxZ*tMaxZ);
            hitInfo.blockPos = cc->getWorldPos() + local;
            return true;
        }

        // else, update error terms & check escape conditions
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                if (tMaxX > radius) break;
                x += stepX;
                tMaxX += tDeltaX;
                hitInfo.normal = {-stepX, 0, 0};
            } else {
                if (tMaxZ > radius) break;
                z += stepZ;
                tMaxZ += tDeltaZ;
                hitInfo.normal = {0, 0, -stepZ};
            }
        } else {
            if (tMaxY < tMaxZ) {
                y += stepY;
                tMaxY += tDeltaY;
                hitInfo.normal = {0, -stepY, 0};
            } else {
                if (tMaxZ > radius) break;
                z += stepZ;
                tMaxZ += tDeltaZ;
                hitInfo.normal = {0, 0, -stepZ};
            }
        }
    }

    hitInfo.blockData.id = ID::AIR;
    hitInfo.hit = false;

    // we didn't hit anything
    return false;
}


};
