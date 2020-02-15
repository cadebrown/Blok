/* Client.cc - implementation of the client side code */

#include <Blok/Client.hh>

namespace Blok {

// callback to handle results from GLFW
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    void* usr_ptr = glfwGetWindowUserPointer(window);
    if (usr_ptr != NULL) {
        // then we have a client object, so set the current key
        Client* client = (Client*)usr_ptr;
        client->input.keys[key] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        //printf("%d: %d\n", key, client->input.keys[key]);
    }
}

// callback to be called when the window is resized, so we can resize everything
static void glfw_resize_callback(GLFWwindow* window, int w, int h) {
    void* usr_ptr = glfwGetWindowUserPointer(window);
    if (usr_ptr != NULL) {
        // then we have a client object, so set the current key
        Client* client = (Client*)usr_ptr;
        blok_debug("resize to %ix%i", w, h);
        client->gfx.renderer->resize(w, h);
        //client->input.keys[key] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
        //printf("%d: %d\n", key, client->input.keys[key]);
    }
}


// callback to be called when the window's framebuffer (i.e. output) is resized, so update
//   the area openGL is rendering to
static void glfw_fbresize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}



// construct a new client
Client::Client(Server* server, int w, int h) {
    this->server = server;

    // store the monitor
    gfx.monitor = glfwGetPrimaryMonitor();

    /*
    const GLFWvidmode* mode = glfwGetVideoMode(gfx.monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    */

    // create a window to render to
    gfx.window = glfwCreateWindow(w, h, "Blok", NULL, NULL);
    if (!gfx.window) {
        blok_error("Failed to create GLFW Window!");
    }

    glfwGetWindowSize(gfx.window, &w, &h);

    // set it to focused
    glfwMakeContextCurrent(gfx.window);
    gfx.isFocused = true;
    // 1 = vsync, 0 = as fast as possible
    glfwSwapInterval(1); 

    // set the window user pointer to this class so we can get it later
    glfwSetWindowUserPointer(gfx.window, this);

    // set up resizing call back
    glfwSetWindowSizeCallback(gfx.window, glfw_resize_callback);
    glfwSetFramebufferSizeCallback(gfx.window, glfw_fbresize_callback);

    // set up input
    glfwSetKeyCallback(gfx.window, glfw_key_callback);



    // raw mouse (i.e. no acceleration)
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(gfx.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    //glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize history things/state
    N_frames = 0;

    lastTime = getTime();

    yaw = 0.0f;
    pitch = 0.0f;

    this->gfx.renderer = new Render::Renderer(w, h);

    // just check the errors
    check_GL();

}

// destroy a client
Client::~Client() {
    // this was constructed for the client
    delete gfx.renderer;

    // as was this
    glfwDestroyWindow(gfx.window);
}


// run a frame on the client
bool Client::frame() {

    // amount (in radians) the pitch has to be within
    const float pitch_slop = .1f;

    // sanitize the input
    if (pitch > M_PI / 2 - pitch_slop) pitch = M_PI / 2 - pitch_slop;
    if (pitch < -M_PI / 2 + pitch_slop) pitch = -M_PI / 2 + pitch_slop;

    // round yaw to between 0 and 2PI
    yaw = fmod(yaw, 2 * M_PI);
    if (yaw < 0.0) yaw += 2 * M_PI;

    // always use this for up
    gfx.renderer->up = vec3(0, 1, 0);

    // compute the forward direction (i.e. +Z axis), by first rotating (0,0,1) (+Z world coord) by pitch along the X axis,
    //   then yaw around the Y axis
    gfx.renderer->forward = (glm::rotate(yaw, vec3(0, 1, 0)) * glm::rotate(pitch, vec3(1, 0, 0))) * vec4(0, 0, 1, 0);

    // normalize it just to make sure
    gfx.renderer->forward = glm::normalize(gfx.renderer->forward);

    // tell the renderer to begin accepting render commands
    gfx.renderer->render_start();

    // get the current
    ChunkID rendid = { (int)(floor(gfx.renderer->pos.x / CHUNK_SIZE_Z)), (int)(floor(gfx.renderer->pos.z / CHUNK_SIZE_Z)) };

    // view distance in chunks
    int N = 8;
    // render all these chunks
    for (int X = -N; X <= N; ++X) {
        for (int Z = -N; Z <= N; ++Z) {

            if (X * X + Z * Z > N * N + 5) continue;
            
            // get the current local chunk ID
            ChunkID cid = {rendid.X + X, rendid.Z + Z};
            //ChunkID cid = {X, Z};

            // get our chunk
            //Chunk* chunk = server->loadChunk(server->worlds["world"], cid);
            Chunk* chunk = server->getChunk(cid);
            // now, render it
            if (chunk != NULL) gfx.renderer->renderChunk(cid, chunk);

        }
    }


    Render::Mesh* suz = Render::Mesh::loadConst("assets/obj/Suzanne.obj");

    gfx.renderer->renderMesh(suz, glm::translate(vec3(16, 40, 16)) * glm::scale(vec3(10.0)));

    // tell it we are done
    gfx.renderer->render_end();

    // clear input
    for (int i = 0; i < GLFW_KEY_LAST; ++i) {
        input.lastKeys[i] = input.keys[i];
        //input.keys[i] = false;
    }

    // update the frame
    glfwSwapBuffers(gfx.window);
    glfwPollEvents();

    // now, get the cursor position
    input.lastMouse = input.mouse;

    double _x, _y;
    glfwGetCursorPos(gfx.window, &_x, &_y);
    input.mouse = glm::vec2((float)_x, (float)_y);

    gfx.wasFocused = gfx.isFocused;
    gfx.isFocused = glfwGetWindowAttrib(gfx.window, GLFW_FOCUSED);

    // calculate the mouse movement, setting to 0 if it was flicked to it
    // this is to prevent large spikes in delta
    input.mouseDelta = (N_frames < 3 || gfx.isFocused == false || gfx.wasFocused == false) ? glm::vec2(0.0f, 0.0f) : input.mouse - input.lastMouse;

    // count the current frame
    N_frames++;

    // check any opengl errors
    check_GL();

    double ctime = getTime();
    dt = ctime - lastTime;
    lastTime = ctime;

    // see if the app should close or not
    if (glfwWindowShouldClose(gfx.window)) {
        return false;
    }


    return true;

}

bool Client::getFullscreen() {
    // the window is full screen if it has a dedicated monitor
    return glfwGetWindowMonitor(gfx.window) != NULL;
}

void Client::setFullscreen(bool toFullscreen) {
    // do nothing
    if (toFullscreen == getFullscreen()) return;

    if (toFullscreen) {
        // make it full screen

        // first, save size & position so we can restore
        glfwGetWindowPos(gfx.window, &gfx.windowPos[0], &gfx.windowPos[1] );
        glfwGetWindowSize(gfx.window, &gfx.windowSize[0], &gfx.windowSize[1] );

        gfx.monitor = glfwGetPrimaryMonitor();

        // get resolution of monitor
        const GLFWvidmode * mode = glfwGetVideoMode(gfx.monitor);

        // switch to full screen
        glfwSetWindowMonitor(gfx.window, gfx.monitor, 0, 0, mode->width, mode->height, 0 );


    } else {
        // else we are coming from full screen, so restore window state
        gfx.monitor = NULL;
        
        glfwSetWindowMonitor(gfx.window, NULL, gfx.windowPos[0], gfx.windowPos[1], gfx.windowSize[0], gfx.windowSize[1], 0);
    }

}




};
