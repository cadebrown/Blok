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

namespace Blok {

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
            fname = upone ? upone : fname;
        }

        fprintf(stderr, "(@%s:%i)", fname, line);
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

    setLogLevel(LogLevel::DEBUG);

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
    
    // output some information
    blok_info("Blok initialized successfully!");


    return true;
}

}



/* COMMANDLINE APP */

#include <Blok/Random.hh>

using namespace Blok;

int main(int argc, char** argv) {

    // disable getopt's error messages
    opterr = 0;

    // our option
    int opt;

    // parse arguments 
    while ((opt = getopt(argc, argv, "o:vh")) != -1) {
        if (opt == 'h') {
            // print help
            printf("Usage: %s [-h]\n\n", argv[0]);
            printf("  -h           Prints this help/usage message\n");
            printf("\nBlok v%i.%i.%i %s\n", BUILD_MAJOR, BUILD_MINOR, BUILD_PATCH, BUILD_DEV ? "(dev)" : "");
            printf("Cade Brown <brown.cade@gmail.com>\n");
            return 0;
        } else if (opt == 'v') {
            // increate verbosity
            setLogLevel((LogLevel)(getLogLevel()+1));
            continue;
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


    // now, actually run the game
    Random::XorShift gen = Random::XorShift();

    int i;
    for (i = 0; i < 32; ++i) {
        printf("%f ", gen.getF());
    }


    printf("\n");



    return 0;
}

