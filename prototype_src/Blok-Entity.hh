/* Blok-Entity.hh - basic entity management */

#pragma once
#ifndef BLOK_ENTITY_HH__
#define BLOK_ENTITY_HH__

#include <Blok-Render.hh>

namespace Blok {

    class Entity {

        public:

        // the location of the entity in world space
        vec3 loc;

        // the current name of the entity
        String name;

    };

    class Player : public Entity {

        public:

        Player(String name) {
            this->name = name;
            this->loc = vec3(0, 0, 0);
        }

    };

};


#endif

