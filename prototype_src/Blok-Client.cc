/* Blok-Client.cc - implementation of the client side code */

#include <Blok-Client.hh>

namespace Blok {


static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    void* usr_ptr = glfwGetWindowUserPointer(window);
    if (usr_ptr != NULL) {
        // then we have a client object, so set the current key
        Client* client = (Client*)usr_ptr;
        client->keysPressed[key] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
    }
}


// construct a new client
Client::Client(Server* server, int w, int h) {
    this->server = server;
    // create a window to render to
    this->window = glfwCreateWindow(w, h, "Blok", nullptr, nullptr);
    if (!this->window) {
        b_error("Failed to create GLFW Window!");
    }

    // set it to focused
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 1 = vsync, 0 = as fast as possible

    // set it to focused
    glfwMakeContextCurrent(window);
    isFocused = true;
    // 1 = vsync, 0 = as fast as possible
    glfwSwapInterval(1); 

    // set the window user pointer to this class so we can get it later
    glfwSetWindowUserPointer(window, this);

    // // set up input
    glfwSetKeyCallback(window, glfw_key_callback);
    if (glfwRawMouseMotionSupported()) glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // just check the errors
    opengl_error_check();

    // initialize history things/state
    N_frames = 0;

    yaw = 0.0f;
    pitch = 0.0f;


    this->renderer = new Render::Renderer(w, h);

}

// destroy a client
Client::~Client() {
    // this was constructed for the client
    delete renderer;

    // as was this
    glfwDestroyWindow(window);
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
    
    renderer->up = vec3(0, 1, 0);
    //renderer->forward = vec3(0, 0, 1);
    //renderer->up = (glm::rotate((float)(pitch - M_PI / 2), vec3(1, 0, 0)) * glm::rotate(yaw, vec3(0, 1, 0))) * vec4(0, 0, 1, 0);
    renderer->forward = (glm::rotate(yaw, vec3(0, 1, 0)) * glm::rotate(pitch, vec3(1, 0, 0))) * vec4(0, 0, 1, 0);

    renderer->forward = normalize(renderer->forward);

    // tell our renderer what to do
    renderer->render_start();
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    ChunkID rendid = { renderer->pos.x / 16, renderer->pos.z / 16 };

    // view distance in chunks
    int N = 4;
    // render all these chunks
    for (int X = -N; X <= N; ++X) {
        for (int Z = -N; Z <= N; ++Z) {

            if (X * X + Z * Z > N * N + 3) continue;
            
            // get the current local chunk ID
            ChunkID cid = {rendid.X + X, rendid.Z + Z};
            //ChunkID cid = {X, Z};

            // get our chunk
            Chunk* chunk = server->loadChunk(server->worlds["world"], cid);

            // now, render it
            renderer->renderChunk(cid, chunk);

        }
    }

    // tell it we are done
    renderer->render_end();

    opengl_error_check();

    // clear input
    for (int i = 0; i < GLFW_KEY_LAST; ++i) {
        lastKeysPressed[i] = keysPressed[i];
        keysPressed[i] = false;
    }

    // update the frame
    glfwSwapBuffers(window);
    glfwPollEvents();

    // now, get the cursor position
    lastMousePos = mousePos;

    double _x, _y;
    glfwGetCursorPos(window, &_x, &_y);
    mousePos = glm::vec2((float)_x, (float)_y);

    wasFocused = isFocused;
    isFocused = glfwGetWindowAttrib(window, GLFW_FOCUSED);

    // calculate the mouse movement, setting to 0 if it was flicked to it
    // this is to prevent large spikes in delta
    mouseDelta = (N_frames < 3 || isFocused == 0 || wasFocused == 0) ? glm::vec2(0.0f, 0.0f) : mousePos - lastMousePos;

    // count the current frame
    N_frames++;

    // check any opengl errors
    opengl_error_check();

    // see if the app should close or not
    if (glfwWindowShouldClose(window)) {
        return false;
    }

    return true;

}


};
