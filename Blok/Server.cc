/* Server.cc - implementation of the server protocol */

#include <Blok/Server.hh>

#include <Blok/Client.hh>

namespace Blok {

// raycast() should seek through all possible chunks, checking intersection along 'ray',
//   up to 'maxDist'. If it ends up hitting a solid block, return true and set all the 'to*'
//   arguments to the data about the hit
bool LocalServer::raycastBlock(Ray ray, float maxDist, RayHit& hitInfo) {

    // the basic algorithm is: view the entire `XZ` plane as a pixel, and use this algo: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    // along the chunks, then call the individual chunk.raycast() functions with translated coordinates

    ray.dir = glm::normalize(ray.dir);

    // current x, y, z position
    vec3 xyz = glm::floor(ray.orig);

    // the amount to change xyz by
    vec3 dxyz = ray.dir;

    // how to step in various directions, i.e. +1 if it is positive, -1 if it is negative, 0 if it is 0
    vec3 step_xyz = glm::sign(dxyz);

    // invalid, just report nothing
    if (step_xyz.x == 0.0f && step_xyz.y == 0.0f && step_xyz.z == 0.0f) {
        static double cTime = 0.0;
        if (getTime() >= cTime) {
            cTime = getTime() + 4.0;
            blok_warn("Raycast given dx=dy=dz=0");
        }
        return false;
    }

    // max & delta have to deal with the error term. Essentially, we want to process the minimum maximum error
    //  and update in that axis at a given time
    vec3 tMax = { 
        intBound(ray.orig.x, dxyz.x),
        intBound(ray.orig.y, dxyz.y),
        intBound(ray.orig.z, dxyz.z)
    };
    

    // how much to change the minmax by at any given iteration
    vec3 tDelta = step_xyz / dxyz;

    // the last chunk ID, to speed up/cache results by not looking it up every block
    ChunkID lastChunkID;
    
    // current chunk
    Chunk* cc = NULL;

    // keep trying, break inside if there's a problem
    while (true) {
        
        // get block coordinates
        vec3i xyz_i = vec3i(glm::floor(xyz));

        // skip blocks that are invalid coordinates, but continue the loop, in case it
        //   comes back into existence
        if (xyz_i.y >= 0 && xyz_i.y < CHUNK_SIZE_Y) {

            // see what chunk the block would fall in, from the given integer coordinates
            ChunkID cid = ChunkID::fromPos(xyz_i);

            // check if we have changed chunks. If so, look it up by ID
            if (cc == NULL || cid != lastChunkID) cc = getChunkIfLoaded(cid);

            // if the current chunk doesn't exist, we can raycast no farther, so we say we haven't hit anything
            if (!cc) return false;

            // update variable so that we can keep the results cached
            lastChunkID = cid;

            // convert to local coordinates
            vec3i local = xyz_i - cc->getWorldPos();

            // debug codes:
            //printf("local:%i,%i,%i\n", local.x, local.y, local.z);
            //dirtyClient->gfx.renderer->renderMesh(Render::Mesh::loadConst("assets/obj/Sphere.obj"), glm::translate(xyz + vec3(0.5)) * glm::scale(vec3(0.3)));

            // probe the block, and check if it is not air
            if ((hitInfo.blockData = cc->get(local.x, local.y, local.z)).id != ID::AIR) {
                // obviously, we've hit
                hitInfo.hit = true;

                // get the distance
                hitInfo.dist = glm::distance(glm::floor(ray.orig), xyz);

                // set the block index to the one that was checked
                hitInfo.blockPos = xyz_i;

                // set the position it was hit at

                // now, update the position, since it must be on the face of the dot
                if (hitInfo.normal.x != 0.0f) {
                    // calculate actual intersection point
                    float ds = (xyz_i.x + (hitInfo.normal.x>0?1:0)) - ray.orig.x;
                    hitInfo.pos = ray.orig + ds / ray.dir.x * ray.dir;
                } else if (hitInfo.normal.y != 0.0f) {
                    // calculate actual intersection point
                    float ds = (xyz_i.y + (hitInfo.normal.y>0?1:0)) - ray.orig.y;
                    hitInfo.pos = ray.orig + ds / ray.dir.y * ray.dir;
                } else if (hitInfo.normal.z != 0.0f) {
                    // calculate actual intersection point
                    float ds = (xyz_i.z + (hitInfo.normal.z>0?1:0)) - ray.orig.z;
                    hitInfo.pos = ray.orig + ds / ray.dir.z * ray.dir;
                }

                return true;
            }
        }

        // enow, update error terms, update the 'neediest' axis, and recheck all bounds
        if (tMax.x < tMax.y) {
            if (tMax.x < tMax.z) {
                if (tMax.x > maxDist) break;
                xyz.x += step_xyz.x;
                tMax.x += tDelta.x;
                hitInfo.normal = { -step_xyz.x, 0, 0 };
            } else {
                if (tMax.z > maxDist) break;
                xyz.z += step_xyz.z;
                tMax.z += tDelta.z;
                hitInfo.normal = { 0, 0, -step_xyz.z };
            }
        } else {
            if (tMax.y < tMax.z) {
                xyz.y += step_xyz.y;
                tMax.y += tDelta.y;
                hitInfo.normal = { 0, -step_xyz.y, 0 };
            } else {
                if (tMax.z > maxDist) break;
                xyz.z += step_xyz.z;
                tMax.z += tDelta.z;
                hitInfo.normal = { 0, 0, -step_xyz.z };
            }
        }
    }
    
    // make sure these 2 variables are set
    hitInfo.blockData.id = ID::AIR;
    hitInfo.hit = false;

    // we didn't hit anything
    return false;
}


};
