/* Client.hh - implementation of the main application */

#pragma once

#ifndef BLOK_CLIENT_HH__
#define BLOK_CLIENT_HH__

// general Blok library
#include <Blok/Blok.hh>

// we use the server protocol
#include <Blok/Render.hh>

// we use the server protocol
#include <Blok/Server.hh>

namespace Blok {

    // Client - a renderer/controller for a game
    class Client {
        public:

        // variable to hold all graphics information relevant to the client
        struct {

            // the output window we are rendering to
            GLFWwindow* window;

            // whether or not it is currently focused, and whether or not it was last
            //   frame
            bool isFocused, wasFocused;

            // the main renderer object, responsible for the rendering
            Render::Renderer* renderer;

        } gfx;


        // variable to hold all input related to keyboard/mouse
        struct {

            // the XY position on the screen of the mouse, and the
            //   delta since the last frame
            vec2 mouse, mouseDelta;

            // a lookup table to check if a key is currently being pressed down
            Map<int, bool> keys;


            // LAST TIME CACHES

            // the last array of key values
            Map<int, bool> lastKeys;

            // the last XY position of the mouse
            vec2 lastMouse;

        } input;


        // the current delta time per frame of the client
        double dt;

        // number of frames that have been computed thus far
        int N_frames;

        
        // current orientation (from 0 to 2pi)
        float yaw;

        // from -pi/2 to +pi/2 (minus a few blocks)
        float pitch;


        // state variables

        // the internal server/engine
        Server* server;

        // construct a new client, given the server, and window size
        Client(Server* server, int w, int h);

        // delete a client. Does not free/affect the server
        ~Client();

        // run a single frame on the client, returning true if it should continue
        // false if it has quit
        bool frame();


    };


}


#endif /* BLOK_WG_HH__ */
