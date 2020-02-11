/* Blok-Client.hh - the input control/client side */

#pragma once
#ifndef BLOK_CLIENT_HH__
#define BLOK_CLIENT_HH__

#include <Blok-Server.hh>
#include <Blok-Render.hh>

namespace Blok {

    /* Block::Client - the client side application that is ran to display the game, and take 
         user input into account */
    class Client {
        public:

        // the output window we are rendering to
        GLFWwindow* window;

        // mouse position & movement since last frame
        vec2 mousePos, mouseDelta;

        // whether or not the game is focused
        bool isFocused;

        // current input status for every key
        // use GLFW_KEY_* macros to index this
        Map<int, bool> keysPressed;

        // the current delta time per frame of the client
        double dt;

        // number of frames that have been computed thus far
        int N_frames;


        // history of input

        // last mouse position
        vec2 lastMousePos;

        // whether or not it was focused last frame
        bool wasFocused;

        // the last frame's key status
        // use GLFW_KEY_* macros to index this
        Map<int, bool> lastKeysPressed;

        // current orientation (from 0 to 2pi)
        float yaw;

        // from -pi/2 to +pi/2 (minus a few blocks)
        float pitch;

        // state variables

        // the main renderer object, responsible for the rendering
        Render::Renderer* renderer;

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


#endif


