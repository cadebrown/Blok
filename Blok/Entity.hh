/* Blok/Entity.hh - declaration of the 'Entity' protocol
 *
 */

#pragma once
#ifndef BLOK_ENTITY_HH__
#define BLOK_ENTITY_HH__

/* main Blok library */
#include <Blok/Render.hh>

namespace Blok {

    // Entity - a physical, moving object that is not tied to the grid
    // This is an abstract class, which is implemented by all the different
    //   types of entities
    class Entity {

        public:

        // the unique identifier
        UUID uuid;



        // Return the world position of the entity
        virtual vec3 getPos() = 0;

        // Attempt to set the position of an entity
        virtual void setPos(vec3 pos) = 0;

        // Return the mesh that the entity should be rendered with
        virtual Render::RenderData getRender() = 0;

        Entity() {
            this->uuid = "<none>";
        }


        Entity(UUID uuid) {
            this->uuid = uuid;
        }

    };


    class ItemEntity : public Entity {
        public:
        // world position
        vec3 pos;

        // what is the ID of the item entity?
        ID id;

        // Return the world position of the entity
        vec3 getPos() {
            return pos;
        }

        // Attempt to set the position of an entity
        void setPos(vec3 pos) {
            this->pos = pos;
        }

        // Return the mesh that the entity should be rendered with
        Render::RenderData getRender() {
            return Render::RenderData(Render::Mesh::loadConst("assets/obj/Suzanne.obj"), glm::translate(pos));
        }

        ItemEntity(UUID uuid) {
            this->uuid = uuid;
        }

    };

}

#endif

