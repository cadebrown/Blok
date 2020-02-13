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
    }
}


// construct a new client
Client::Client(Server* server, int w, int h) {
    this->server = server;
    // create a window to render to
    gfx.window = glfwCreateWindow(w, h, "Blok", nullptr, nullptr);
    if (!gfx.window) {
        blok_error("Failed to create GLFW Window!");
    }

    // set it to focused
    glfwMakeContextCurrent(gfx.window);
    glfwSwapInterval(1); // 1 = vsync, 0 = as fast as possible

    // set it to focused
    glfwMakeContextCurrent(gfx.window);
    gfx.isFocused = true;
    // 1 = vsync, 0 = as fast as possible
    glfwSwapInterval(1); 

    // set the window user pointer to this class so we can get it later
    glfwSetWindowUserPointer(gfx.window, this);

    // // set up input
    glfwSetKeyCallback(gfx.window, glfw_key_callback);
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(gfx.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetInputMode(gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initialize history things/state
    N_frames = 0;

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

    const float pitch_slop = .1f;
    // sanitize some input
    if (pitch > M_PI / 2 - pitch_slop) pitch = M_PI / 2 - pitch_slop;
    if (pitch < -M_PI / 2 + pitch_slop) pitch = -M_PI / 2 + pitch_slop;
    //pitch = 0;

    yaw = fmod(yaw, 2 * M_PI) + 2 * M_PI;
    if (yaw >= 2 * M_PI) yaw -= 2 * M_PI;

    // use the internal renderer to update
    //renderer->render();
    
    gfx.renderer->up = vec3(0, 1, 0);
    //renderer->forward = vec3(0, 0, 1);
    //renderer->up = (glm::rotate((float)(pitch - M_PI / 2), vec3(1, 0, 0)) * glm::rotate(yaw, vec3(0, 1, 0))) * vec4(0, 0, 1, 0);
    gfx.renderer->forward = (glm::rotate(yaw, vec3(0, 1, 0)) * glm::rotate(pitch, vec3(1, 0, 0))) * vec4(0, 0, 1, 0);

    gfx.renderer->forward = glm::normalize(gfx.renderer->forward);

    // tell our renderer what to do
    gfx.renderer->render_start();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    ChunkID rendid = { floor(gfx.renderer->pos.x / CHUNK_SIZE_Z), floor(gfx.renderer->pos.z / CHUNK_SIZE_Z) };

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


    Render::Mesh* suz = Render::Mesh::loadConst("../resources/suzanne.obj");

    gfx.renderer->renderMesh(suz, glm::translate(vec3(0, 100, 0)));


    // tell it we are done
    gfx.renderer->render_end();

    // clear input
    for (int i = 0; i < GLFW_KEY_LAST; ++i) {
        input.lastKeys[i] = input.keys[i];
        input.keys[i] = false;
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

    // see if the app should close or not
    if (glfwWindowShouldClose(gfx.window)) {
        return false;
    }

    return true;

}


};
