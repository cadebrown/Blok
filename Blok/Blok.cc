/* Blok.cc - the implementation for the general library API */

#include "Blok/Blok.hh"

// for vararg parsing
#include <stdarg.h>

// use the standard time library it for the getTime() method
#include <chrono>

// for getopt
#include <unistd.h>
#include <getopt.h>

// ASCII format codes
#define BOLD   "\033[1m"
#define RESET  "\033[0m"
#define WHITE  "\033[37m"
#define RED    "\033[31m"
#define YELLOW "\033[33m"

/* MISC */


namespace Blok {


/* consants */


// global freetype library
FT_Library ftlib;

Map<ID, BlockProperties*> BlockProperties::all;


/* oeprator overloads */

bool operator<(ChunkID A, ChunkID B) {
    if (A.X == B.X) return A.Z > B.Z;
    else return A.X > B.X;
}

ChunkID operator+(ChunkID A, ChunkID B) {
    return ChunkID(A.X+B.X, A.Z+B.Z);
}
ChunkID operator-(ChunkID A, ChunkID B) {
    return ChunkID(A.X-B.X, A.Z-B.Z);

}

/* LOGGING */

// current logging level
static LogLevel curLevel = LogLevel::DEBUG;

// level names to prevent
static const char* levelNames[] = {
    WHITE  "TRACE",
    WHITE  "DEBUG",
    WHITE  "INFO ",
    YELLOW "WARN ",
    RED    "ERROR"
};

// set the log level
void setLogLevel(LogLevel level) {
    // first clamp it
    if (level > LogLevel::ERROR) level = LogLevel::ERROR;
    else if (level < LogLevel::TRACE) level = LogLevel::TRACE;

    // set the level    
    curLevel = level;
}

// get the current log level
LogLevel getLogLevel() {
    return curLevel;
}

// logs with a leevl. use the macros `blok_info`, etc
void log_internal(LogLevel level, const char *file, int line, const char* fmt, ...) {
    if (level < curLevel) {
        // not important enough to print currently
        return;
    }

    // print a header with the level name
    fprintf(stderr, BOLD "%s" RESET ": ", levelNames[level]);

    // call the vfprintf
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);

    va_end(args);

    // warnings & errors should print out the source of the error as well,
    //   to help with debugging
    if (level == LogLevel::WARN || level == LogLevel::ERROR) {
        // find the file name
        const char* fname = strrchr(file, '/');
        // if it didn't exist default back
        if (fname == NULL) fname = file;
        else {
            // try and go up another directory
            const char* upone = strrchr(file, '/');
            fname = upone ? upone + 1 : fname + 1;
        }

        fprintf(stderr, " (@%s:%i)", fname, line);
    }


    // always end with a newline for this function
    fprintf(stderr, "\n");

    // flush the output
    fflush(stderr);
}

/* MISC FUNCTIONS */

// get the current time (in seconds) since Blok has been initialized,
//   i.e. the relative wall time
double getTime() {
    // get the start time as a static variable
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();

    // get a duration as a difference 
    std::chrono::duration<double> elapsed = current_time - start_time;

    return elapsed.count();
}

/* LIBRARY CALLBACKS */

// the GLFW error callback function, called when GLFW encounters an error
static void glfw_errcb(int err, const char* description) {
    blok_error("GLFW: %s (%i)", description, err);
}

// return a string representation of an error string
static String GL_getErrorString(GLenum err) {
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


// check/handle any OpenGL errors, returning true if there were errors,
//   false if there were none
bool check_GL() {
    bool hadErr = false;

    // iterate through any errors on the error stack
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        hadErr = true;
        blok_error("OpenGL: %s (%i)", GL_getErrorString(err).c_str(), (int)err);
    }

    // error out if there was one
    if (hadErr) {
        exit(-1);
    }

    return hadErr;
}

/* ERROR HANDLING */


/* INITIALIZATION ROUTINES */

// initialize the place to look for everything:
List<String> paths = { ".", ".." };

// initialize everyting in Blok
bool initAll() {

    // initialize openGL stuffs
    gl3wInit();

    // make sure our target version is supported
    if (gl3wIsSupported(3, 3) != 0) {
        blok_error("Failed to init OpenGL v3.3");
        return false;
    }

    // initialize GLFW, for windows/etc
    if (!glfwInit()) {
        blok_error("Failed to initialize GLFW!");
        return false;
    }

    // set callback for any reports from OpenGL
    glfwSetErrorCallback(glfw_errcb);

    // request version 3.3 CORE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   
    // various initializations
    // reset the GLFW time
    glfwSetTime(0.0);

    // do some error checks
    if (check_GL()) return false;


    if (FT_Init_FreeType(&ftlib)) {
        blok_error("Failed to initialize FreeType!");
        return false;
    }

    
    // output some information
    blok_info("Blok initialized successfully!");

    // now, set up block info

    #define ADDBLOCK(_id, _idname, _name) { \
        BlockProperties::all[_id] = new BlockProperties(_id); \
        BlockProperties::all[_id]->id_name = _idname; \
        BlockProperties::all[_id]->name = _name; \
    }

    ADDBLOCK(ID::AIR, "AIR", "Air");
    ADDBLOCK(ID::DIRT, "DIRT", "Dirt");
    ADDBLOCK(ID::DIRT_GRASS, "DIRT_GRASS", "Dirt (Grass)");
    ADDBLOCK(ID::STONE, "STONE", "Stone");

    blok_trace("sizeof(BlockData)==%ib", (int)sizeof(BlockData));
    blok_trace("sizeof(ID)==%ib", (int)sizeof(ID));
    int cb = CHUNK_NUM_BLOCKS * sizeof(BlockData);
    blok_trace("Chunk size: %ix%ix%i (%i blocks) (%ib, %ikb)", CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z, CHUNK_NUM_BLOCKS, cb, cb / 1024);





    return true;
}

}



/* COMMANDLINE APP */

#include <Blok/Random.hh>
#include <Blok/Server.hh>
#include <Blok/Client.hh>

using namespace Blok;


// run a check of algorithms
void runTests() {
    int N = 1000000, B = 4;
    uint32_t tmp = 0;

    printf(" -*- Blok v%i.%i.%i %s Tests -*-\n", BUILD_MAJOR, BUILD_MINOR, BUILD_PATCH, BUILD_DEV ? "(dev)" : "");
    printf(" -*- 1: Random::XorShift::getU32 (N=%i) -*-\n", N);
    Random::XorShift rnd = Random::XorShift(0);

    double st = getTime();
    for (int i = 0; i < N; ++i) {
        uint32_t r = rnd.getU32();
        tmp += r;

        if (i == N - B) printf("... ");
        if (i < B || N - i <= B) printf("0x%x ", r);
    }
    printf("\n");
    st = getTime() - st;

    printf("Speed %lfMsmp/sec\n", 1e-6 * N / st);

    printf(" -*- 2: Random::Perlin::noise1d (N=%i) -*-\n", N);

    Random::Perlin prnd = Random::Perlin(0);

    st = getTime();
    for (int i = 0; i < N; ++i) {
        double r = prnd.noise1d(0.123456 * i);
        tmp += r;

        if (i == N - B) printf("... ");
        if (i < B || N - i <= B) printf("%.3lf ", r);
    }
    printf("\n");
    st = getTime() - st;

    printf("Speed %lfMsmp/sec\n", 1e-6 * N / st);


    printf(" -*- 3: Random::Perlin::noise2d (N=%i,M=%i) -*-\n", N / 1000, 1000);

    st = getTime();
    int ct = 0;
    for (int i = 0; i < N / 1000; ++i) {
        for (int j = 0; j < 1000; ++j) {

            double r = prnd.noise2d(0.123456 * i, 0.0372983 * j);
            tmp += r;

            if (ct == N - B) printf("... ");
            if (ct < B || N - ct <= B) printf("%.3lf ", r);
            ct++;
        }

    }
    printf("\n");
    st = getTime() - st;

    printf("Speed %lfMsmp/sec\n", 1e-6 * N / st);

}


int main(int argc, char** argv) {

    // disable getopt's error messages
    opterr = 0;

    // our option
    int opt;

    // parse arguments 
    while ((opt = getopt(argc, argv, "Tvh")) != -1) {
        if (opt == 'h') {
            // print help
            printf("Usage: %s [-h]\n\n", argv[0]);
            printf("  -h           Prints this help/usage message\n");
            printf("  -T           Run some sanity checks\n");
            printf("\nBlok v%i.%i.%i %s\n", BUILD_MAJOR, BUILD_MINOR, BUILD_PATCH, BUILD_DEV ? "(dev)" : "");
            printf("Cade Brown <brown.cade@gmail.com>\n");
            return 0;
        } else if (opt == 'v') {
            // increate verbosity
            setLogLevel((LogLevel)((int)getLogLevel()-1));
        } else if (opt == 'T') {
            // run a test
            runTests();
            return 0;
        } else if (opt == '?') {
            fprintf(stderr, "Unknown option '-%c', run with '-h' to see help message\n", optopt);
            return -1;
        } else if (opt == ':') {
            fprintf(stderr, "Option '-%c' needs a value, run with '-h' to see help message\n", optopt);
            return -2;
        }
    }

    // these are all extraneous arguments, ignore for now
    // In the future, allow a world file
    while (optind < argc) {
        fprintf(stderr, "Extra arguments given! Run with '-h' for a usage message\n");
        return -3;

        optind++;
    }

    // try and initialize blok
    if (!initAll()) return -1;

    // create a local server
    LocalServer* server = new LocalServer();

    Client* client = new Client(server, 800, 600);

    blok_info("Initialized with: OpenGL: %s, GLSL: %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    // just update
    client->gfx.renderer->pos = vec3(0, 14, -10);

    float speed = 40.0f;
    client->gfx.renderer->pos = vec3(0, 100, 0);


    // create a statistics object
    Render::Renderer::Stats stats;


    // gathering statistics every so many frames
    int every = 100;

    // get time
    double everyT = getTime();

    while (client->frame()) {
        
        vec3 moveZ = client->gfx.renderer->forward;
        moveZ = normalize(moveZ);

        vec3 moveX = glm::cross(client->gfx.renderer->up, client->gfx.renderer->forward);
        moveX.y = 0;
        moveX = normalize(moveX);

        vec3 moveY = vec3(0, 0.8, 0);

        double dt = client->dt;
        
        if (client->input.keys[GLFW_KEY_W]) {
            client->gfx.renderer->pos += speed * (float)dt * moveZ;
        }
        if (client->input.keys[GLFW_KEY_S]) {
            client->gfx.renderer->pos -= speed * (float)dt * moveZ;
        }

        if (client->input.keys[GLFW_KEY_D]) {
            client->gfx.renderer->pos += speed * (float)dt * moveX;
        }
        if (client->input.keys[GLFW_KEY_A]) {
            client->gfx.renderer->pos -= speed * (float)dt * moveX;
        }

        if (client->input.keys[GLFW_KEY_SPACE]) {
            client->gfx.renderer->pos += speed * (float)dt * moveY;
        }

        if (client->input.keys[GLFW_KEY_LEFT_SHIFT]) {
            client->gfx.renderer->pos -= speed * (float)dt * moveY;
        }

        if (client->input.keys[GLFW_KEY_F] && !client->input.lastKeys[GLFW_KEY_F]) {
            client->setFullscreen(!client->getFullscreen());
        }


        //client->renderer->pos += vec3(0.1, 0.0, 0.0);
        client->yaw += dt * 0.3f * client->input.mouseDelta.x;
        client->pitch += dt * 0.3f * client->input.mouseDelta.y;

        //client->renderer->forward = glm::rotate((float)dt * 0.4f * -client->mouseDelta.x, vec3(0, 1, 0)) * vec4(client->renderer->forward, 0);
        //client->renderer->forward = glm::rotate((float)dt * 0.4f * client->mouseDelta.y, vec3(0, 0, 1)) * vec4(client->renderer->forward, 0);

        stats.t_chunks += client->gfx.renderer->stats.t_chunks;
        stats.n_chunks += client->gfx.renderer->stats.n_chunks;
        stats.n_chunk_recalcs += client->gfx.renderer->stats.n_chunk_recalcs;
        stats.n_tris += client->gfx.renderer->stats.n_tris;


        if (client->N_frames % every == 0) {
            double et = getTime();

            double dt = et - everyT;
            blok_debug("[frame%i] fps: %.1lf, kchunks/s: %.3lf, mtris/f: %.3lf", client->N_frames, every / dt, stats.n_chunk_recalcs / (1e3 * stats.t_chunks), stats.n_tris / (1e6 * every));

            everyT = et;

            // reset the statistics
            stats = Render::Renderer::Stats();
            //blok_debug("fps: %.1lf, chunks/sec: %.1lf", 1.0 / dt, server->stats.n_chunks / server->stats.t_chunks);
        }

    } 


    return 0;
}

