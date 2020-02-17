/* Client.cc - implementation of the client side code */

#include <Blok/Client.hh>

namespace Blok {

Client* dirtyClient = NULL;

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

// callback to handle results from GLFW
static void glfw_mousebutton_callback(GLFWwindow* window, int key, int action, int mods) {
    void* usr_ptr = glfwGetWindowUserPointer(window);
    if (usr_ptr != NULL) {
        // then we have a client object, so set the current mouse button
        Client* client = (Client*)usr_ptr;
        client->input.mouseButtons[key] = action == GLFW_PRESS || action == GLFW_REPEAT;
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

    dirtyClient = this;
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
    glfwSetMouseButtonCallback(gfx.window, glfw_mousebutton_callback);

    // hide the cursor
    glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // raw mouse (i.e. no acceleration)
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(gfx.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    //glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize history things/state
    N_frames = 0;

    lastTime = getTime();

    dt = 0.01;
    smoothFPS = 60.0;

    yaw = 0.0f;
    pitch = 0.0f;

    this->gfx.renderer = new Render::Renderer(w, h);

    // just check the errors
    check_GL();

}

// destroy a client
Client::~Client() {
    if (dirtyClient == this) dirtyClient = NULL;
    // this was constructed for the client
    delete gfx.renderer;

    // as was this
    glfwDestroyWindow(gfx.window);
}


// run a frame on the client
bool Client::frame() {
    dirtyClient = this;

    if (dt < .0001) dt = .0001;

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
    int N = 5;
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

    // capture information about the hit
    RayHit hit;
    
    // perform a raycastBlock
    if (server->raycastBlock(Ray(gfx.renderer->pos, gfx.renderer->forward), 20.0f, hit)) {
        Render::Mesh* outline = Render::Mesh::loadConst("assets/obj/UnitCubeOutline.obj");
        gfx.renderer->renderMesh(outline, glm::translate(vec3(hit.blockPos)));



        // test debug line
        //gfx.renderer->renderDebugLine(hit.pos + vec3(0.5) + 0.5f * hit.normal, hit.pos + vec3(0.5) + 1.0f * hit.normal);
        //gfx.renderer->renderDebugLine(hit.pos, hit.pos + 0.5f * hit.normal);
        //printf("%f,%f,%f\n", hit.normal.x, hit.normal.y, hit.normal.z);


        if (input.mouseButtons[GLFW_MOUSE_BUTTON_RIGHT] && !input.lastMouseButtons[GLFW_MOUSE_BUTTON_RIGHT]) {
            // place block

            // TODO: call a server->setBlock() method

            // compute one block off
            vec3i targetPos = vec3i(glm::floor(vec3(hit.blockPos) + hit.normal));
            if (targetPos.y >= 0 && targetPos.y < CHUNK_SIZE_Y) {
                // set it
                Chunk* cur = server->getChunkIfLoaded(ChunkID::fromPos(targetPos));
                vec3i localPos = targetPos - cur->getWorldPos();

                cur->set(localPos.x, localPos.y, localPos.z, {ID::STONE});
            }


        } else if (input.mouseButtons[GLFW_MOUSE_BUTTON_LEFT] && !input.lastMouseButtons[GLFW_MOUSE_BUTTON_LEFT]) {
            // delete block
            Chunk* cur = server->getChunkIfLoaded(ChunkID::fromPos(hit.blockPos));
            vec3i localPos = hit.blockPos - cur->getWorldPos();

            cur->set(localPos.x, localPos.y, localPos.z, {ID::AIR});
        }

    } else {
        hit.blockData = {ID::AIR};
    }

    // draw coordinate axes
    /*
    vec3 infront = gfx.renderer->pos + gfx.renderer->forward;
    gfx.renderer->renderDebugLine(infront, infront + vec3(0.1f, 0.0f, 0.0f), {1, 0, 0});
    gfx.renderer->renderDebugLine(infront, infront + vec3(0.0f, 0.1f, 0.0f), {0, 1, 0});
    gfx.renderer->renderDebugLine(infront, infront + vec3(0.0f, 0.0f, 0.1f), {0, 0, 1});
    */

    //Render::Mesh* sph = Render::Mesh::loadConst("assets/obj/Sphere.obj");

    //gfx.renderer->renderMesh(sph, glm::translate(vec3(vec3i(hittarget)) + vec3(0.5, 0.5, 0.5)) * glm::scale(vec3(0.8)));

    static Render::UIText* uit = new Render::UIText(Render::FontTexture::loadConst("assets/fonts/ForcedSquare.ttf"));
    // smooth it out over time
    smoothFPS = (1 - dt) * smoothFPS + 1;

    // info screen
    char tmp[2048];
    snprintf(tmp, sizeof(tmp)-1, "Blok v%i.%i.%i %s\npos: %.1f, %.1f, %.1f\nchunk: %+i,%+i\nfps: %.1lf\nhit: %s\nlooking_at: %.1f,%.1f,%.1f\nlooking_nrm: %.1f,%.1f,%.1f", 
        BUILD_MAJOR, BUILD_MINOR, BUILD_PATCH, BUILD_DEV ? "(dev)" : "(release)",
        gfx.renderer->pos.x, gfx.renderer->pos.y, gfx.renderer->pos.z,
        rendid.X, rendid.Z,
        smoothFPS,
        BlockProperties::all[hit.blockData.id]->name.c_str(),
        hit.pos.x, hit.pos.y, hit.pos.z,
        hit.normal.x, hit.normal.y, hit.normal.z
    );


    uit->text = tmp;
    gfx.renderer->renderText({10, gfx.renderer->height-10}, uit);

    // tell it we are done
    gfx.renderer->render_end();

    // clear input
    for (int i = 0; i < GLFW_KEY_LAST; ++i) {
        input.lastKeys[i] = input.keys[i];
    }

    // clear input
    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i) {
        input.lastMouseButtons[i] = input.mouseButtons[i];
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
        glfwSetWindowMonitor(gfx.window, gfx.monitor, 0, 0, mode->width, mode->height, mode->refreshRate );


    } else {
        // else we are coming from full screen, so restore window state
        gfx.monitor = NULL;
        
        glfwSetWindowMonitor(gfx.window, NULL, gfx.windowPos[0], gfx.windowPos[1], gfx.windowSize[0], gfx.windowSize[1], 0);
    }

}




};
