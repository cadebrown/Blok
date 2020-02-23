/* Client.hh - implementation of the main application */

#pragma once

#ifndef BLOK_CLIENT_HH__
#define BLOK_CLIENT_HH__

// general Blok library
#include <Blok/Blok.hh>

// we use the server protocol
#include <Blok/Render.hh>

// we use the audio lib
#include <Blok/Audio.hh>

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

            // the monitor (may be NULL)
            GLFWmonitor* monitor;

            // whether or not it is currently focused, and whether or not it was last
            //   frame
            bool isFocused, wasFocused;

            // the main renderer object, responsible for the rendering
            Render::Renderer* renderer;


            // the window position
            vec2i windowPos{0, 0};

            // the window size
            vec2i windowSize{0, 0};

        } gfx;


        // our audio engine
        Audio::Engine* aEngine;

        // variable to hold all input related to keyboard/mouse
        struct {

            // the XY position on the screen of the mouse, and the
            //   delta since the last frame
            vec2 mouse, mouseDelta;

            // a lookup table to check if a key is currently being pressed down
            Map<int, bool> keys;

            // a lookup table to check if a mouse button is currently being pressed down
            Map<int, bool> mouseButtons;


            // LAST TIME CACHES

            // the last array of key values
            Map<int, bool> lastKeys;

            // the last table of mouse buttons
            Map<int, bool> lastMouseButtons;

            // the last XY position of the mouse
            vec2 lastMouse;

        } input;


        // the current delta time per frame of the client
        double dt;

        // a time-smoothed valud of FPS
        double smoothFPS;

        // the last time (internal use)
        double lastTime;

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


        // get whether or not the client is in full screen mode
        bool getFullscreen();

        // set full screen or not
        void setFullscreen(bool toFullscreen=true);

        // run a single frame on the client, returning true if it should continue
        // false if it has quit
        bool frame();


    };

    // the last updated client. Maybe NULL
    extern Client* dirtyClient;

}


#endif /* BLOK_WG_HH__ */
