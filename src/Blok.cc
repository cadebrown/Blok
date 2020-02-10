/* Blok.cc - main executable file */

#include "Blok.hh"
#include "Blok-Entity.hh"
#include "Blok-Render.hh"
#include "Blok-Server.hh"


namespace Blok {

bool operator<(ChunkID A, ChunkID B) {
    return A.X < B.X && A.Z < B.Z;
}

const char* opengl_error_string(GLenum const err) {
    switch (err) {
        // opengl 2 errors (8)
        case GL_NO_ERROR: return "GL_NO_ERROR";

        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";

        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";

        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";

        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";

        // opengl 3 errors (1)
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";

        // gles 2, 3 and gl 4 error are handled by the switch above
        default: return "UNKNOWN ERROR";
    }
}

void glfw_errorcallback(int err, const char* description) {
    b_warn("GLFW Error[%i]: %s", err, description);
}

void opengl_error_check() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        b_warn("OpenGL Error[%i]: %s", (int)err, opengl_error_string(err));
    }
}

}

using namespace Blok;

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

    glfwSetErrorCallback(glfw_errorcallback);

    // request version 3.3 CORE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create the window
    auto window = glfwCreateWindow(640, 480, "Blok", nullptr, nullptr);
    if (!window) {
        b_error("Failed to create window with GLFW!");
        return -1;
    }

    // set it to focused
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 1 = vsync, 0 = as fast as possible
    
    // various initializations
    glfwSetTime(0.0);
    opengl_error_check();

    b_info("Initialized with: OpenGL: %s, GLSL: %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    // create a server
    Server* server = new Server();

    // create a world to render
    World* world = new World();
    server->addWorld("world", world);

    server->loadChunk(world, {0, 0});

    Render::Renderer* rend = new Render::Renderer(640, 480);

    while (true) {
        // calculate the frame
        rend->render();

        // update the frame
        glfwSwapBuffers(window);
        glfwPollEvents();

        // perform error checks
        opengl_error_check();

        // see if the app should close or not
        if (glfwWindowShouldClose(window)) {
            return 0;
        }


    }


    return 0;
}

