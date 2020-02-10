/* Blocc-Entity.hh - basic entity management */

#pragma once
#ifndef BLOCC_ENTITY_HH__
#define BLOCC_ENTITY_HH__

#include <Blocc.hh>

namespace Blocc {

    class Entity {

        public:

        // the location of the entity in world space
        Coord loc;

        // the current name of the entity
        String name;

    };


    class Player : public Entity {

        public:

        Player(String name) {
            this->name = name;
            this->loc = Coord(0, 0, 0);
        }

    };

};


#endif

