/* Blocc.cc - main executable file */

#include "Blocc.hh"
#include "Blocc-Entity.hh"
#include "Blocc-Render.hh"
#include "Blocc-Server.hh"


namespace Blocc {

bool operator<(ChunkID A, ChunkID B) {
    return A.X < B.X && A.Z < B.Z;
}

}

using namespace Blocc;

int main(int argc, char** argv) {

    b_log_level_set(LOG_TRACE);

    // initialize openGL stuffs
    gl3wInit();

    // make sure our target version is supported
    if (gl3wIsSupported(3, 3) != 0) {
        b_error("Failed to init OpenGL v3.3");
        return -1;
    }

    // initialize GLFW, for windows/etc
    if (!glfwInit()) {
        b_error("Failed to initialize GLFW!");
        return -1;
    }

    // request version 3.3 CORE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create the window
    auto window = glfwCreateWindow(640, 480, "Blocc", nullptr, nullptr);
    if (!window) {
        b_error("Failed to create window with GLFW!");
        return -1;
    }

    // set it to focused
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 1 = vsync, 0 = as fast as possible
    
    // various initializations
    glfwSetTime(0.0);

    b_info("Initialized with: OpenGL: %s, GLSL: %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));



    // create a server
    Server* server = new Server();

    // create a world to render
    World* world = new World();
    server->addWorld("world", world);

    server->loadChunk(world, {0, 0});

    Render::Target* target = new Render::Target(640, 640);


    return 0;
}

