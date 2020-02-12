/* Blok.cc - main executable file */

#include "Blok.hh"
#include "Blok-Entity.hh"
#include "Blok-Render.hh"
#include "Blok-Server.hh"
#include "Blok-Client.hh"

#include <chrono>

namespace Blok {

// initialize the place to look for everything:
List<String> paths = { ".", ".." };

bool operator<(ChunkID A, ChunkID B) {
    if (A.X == B.X) return A.Z > B.Z;
    else return A.X > B.X;
    //return A.X < B.X && A.Z < B.Z;
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
        exit(-1);
    }
}


double getTime() {
    // get the start time as a static var
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();


    std::chrono::duration<double> elapsed = current_time - start_time;

    return elapsed.count();
}

// initialize everyting in Blok
bool initAll() {


    b_log_level_set(LOG_TRACE);

    // initialize openGL stuffs
    gl3wInit();

    // make sure our target version is supported
    if (gl3wIsSupported(3, 3) != 0) {
        b_error("Failed to init OpenGL v3.3");
        return false;
    }

    // initialize GLFW, for windows/etc
    if (!glfwInit()) {
        b_error("Failed to initialize GLFW!");
        return false;
    }

    glfwSetErrorCallback(glfw_errorcallback);

    // request version 3.3 CORE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
/*
    // create the window
    auto window = glfwCreateWindow(640, 480, "Blok", nullptr, nullptr);
    if (!window) {
        b_error("Failed to create window with GLFW!");
        return false;
    }

    // set it to focused
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 1 = vsync, 0 = as fast as possible
    */

   
    // various initializations
    glfwSetTime(0.0);
    opengl_error_check();

    return true;
}

}

using namespace Blok;

int main(int argc, char** argv) {

    // initialize blok
    initAll();

    // create a server
    Server* server = new Server();

    // create a client attached to that server
    Client* client = new Client(server, 640, 480);

    // create a world to render
    World* world = new World();
    server->addWorld("world", world);

    //server->loadChunk(world, {0, 0});
    // put some info
    b_info("Initialized with: OpenGL: %s, GLSL: %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));


    // just update
    client->renderer->pos = vec3(0, 14, -10);

    int n = 0;
    double ltime = getTime();

    float speed = 100;

    do {
    
        double ctime = getTime();
        double dt = ctime - ltime;
        ltime = ctime;

        if (client->keysPressed[GLFW_KEY_W]) {
            client->renderer->pos += speed * (float)dt * client->renderer->forward;
        }
        if (client->keysPressed[GLFW_KEY_S]) {
            client->renderer->pos -= speed * (float)dt * client->renderer->forward;
        }
        client->yaw += dt * 0.4f * client->mouseDelta.x;
        client->pitch += dt * 0.4f * client->mouseDelta.y;

        //client->renderer->forward = glm::rotate((float)dt * 0.4f * -client->mouseDelta.x, vec3(0, 1, 0)) * vec4(client->renderer->forward, 0);
        //client->renderer->forward = glm::rotate((float)dt * 0.4f * client->mouseDelta.y, vec3(0, 0, 1)) * vec4(client->renderer->forward, 0);

        if (client->N_frames % 20 == 0) {

            printf("fps: %lf\n", 1 / dt);
        }

    } while (client->frame());

    return 0;
}

