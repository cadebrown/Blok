/* Shader.cc - implementation of the shader class */

// rendering library
#include <Blok/Render.hh>

// standard library for reading files
#include <fstream>
#include <sstream>


namespace Blok::Render {

// declare the cache of existing shaders
Map<Pair<String, String>, Shader*> Shader::cache;

// internal method to check whether there were any problems with the shader, and if so,
// print and error and return false. Retrun true if there was no program
// type can be "VERTEX", "FRAGMENT", "GEOMETRY", or "PROGRAM"
static bool checkCompileErrors(GLuint shader, const String& name, const String& type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        //check Status
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            // output error
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            blok_error("Failed to compile shader '%s' (of type '%s'), error was: %s", name.c_str(), type.c_str(), infoLog);
            return false;
        }
    } else {
        // check program Status
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            blok_error("Failed to link program '%s', error was: %s", name.c_str(), infoLog);
            return false;

        }
    }

    // there was no problem
    return true;
}

// load a shader from a given vertex shader & fragment shader
Shader* Shader::load(const String& vsFile, const String& fsFile) {
    Pair<String, String> key(vsFile, fsFile);
    if (cache.find(key) == cache.end()) {
        // construct it
        return cache[key] = new Shader(vsFile, fsFile);
    } else {
        return cache[key];
    }

}

// construct a shader from files
Shader::Shader(const String& vsFile, const String& fsFile) {
    // read in the source code
    String vsSrc, fsSrc, vsNew, fsNew;

    bool found = false;

    for (String path : paths) {
        try {
            // try these paths
            vsNew = path + "/" + vsFile, fsNew = path + "/" + fsFile;

            // file sources
            std::ifstream vsIn, fsIn;

            // set exception flags
            vsIn.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fsIn.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            // try and open the files
            vsIn.open(vsNew);
            fsIn.open(fsNew);

            // read in the entire file
            std::stringstream vsStr, fsStr;
            vsStr << vsIn.rdbuf();
            fsStr << fsIn.rdbuf();

            // close both files
            vsIn.close();
            fsIn.close();

            // get the string that was read
            vsSrc = vsStr.str();
            fsSrc = fsStr.str();
            found = true;
            blok_trace("Found specific shader shader with vs: %s, fs: %s", vsNew.c_str(), fsNew.c_str());
            break;

        } catch (std::ifstream::failure e) {
            // catch any error that came about
            blok_trace("Failed to read specific shader shader with vs: %s, fs: %s (err: %s)", vsNew.c_str(), fsNew.c_str(), e.what());
            continue;
        }
    }

    if (!found) {
        blok_error("Failed to read shader with vs: %s, fs: %s", vsFile.c_str(), fsFile.c_str());
        return;
    }

    // get the source code as C strings for the OpenGL library
    const char* vsCode = vsSrc.c_str();
    const char* fsCode = fsSrc.c_str();

    // now, compile the opengl shaders
    GLuint vsProg, fsProg;

    // first, try compiling the vertex shader
    vsProg = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsProg, 1, &vsCode, NULL);
    glCompileShader(vsProg);
    if (!checkCompileErrors(vsProg, vsFile, "VERTEX")) {
        // handle error?
    }

    // then, compile the fragment shader
    fsProg = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsProg, 1, &fsCode, NULL);
    glCompileShader(fsProg);
    if (!checkCompileErrors(fsProg, fsFile, "FRAGMENT")) {
        // handle error?
    }

    // construct the entire program
    glProgram = glCreateProgram();
    // attach vertex -> fragment
    glAttachShader(glProgram, vsProg);
    glAttachShader(glProgram, fsProg);

    //if (geometryPath != nullptr) glAttachShader(glProgram, geometry);

    // attempt to link the program
    glLinkProgram(glProgram);
    if (!checkCompileErrors(glProgram, vsFile + "+" + fsFile, "PROGRAM")) {
        // error linking
    }
    blok_debug("Loaded shader '%s'+'%s'", vsFile.c_str(), fsFile.c_str());

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vsProg);
    glDeleteShader(fsProg);
    //if(geometryPath != nullptr) glDeleteShader(geometry);

}

Shader::~Shader() {
    // just free the shader resource
    glDeleteShader(glProgram);
}

/* misc util */

void Shader::use()  {
    glUseProgram(glProgram); 
}

int Shader::getUL(const String& name) {
    int res = glGetUniformLocation(glProgram, name.c_str());
    /*if (res == -1) {
        static Map<String, double> timesToPrint;
        if (timesToPrint.find(name) == timesToPrint.end()) {
            timesToPrint[name] = getTime() - 1;
        }
        if (getTime() > timesToPrint[name]) {
            blok_error("Unknown uniform '%s'", name.c_str());
            timesToPrint[name] = getTime() + 1;
        }
    }*/
    return res;
}

/* uniform setting */

void Shader::setBool(const String& name, bool value) {         
    glUniform1i(getUL(name), (int)value); 
}
void Shader::setInt(const String& name, int value) { 
    glUniform1i(getUL(name), value); 
}
void Shader::setFloat(const String& name, float value) { 
    glUniform1f(getUL(name), value); 
}
void Shader::setVec2(const String& name, const vec2& value) { 
    glUniform2fv(getUL(name), 1, &value[0]); 
}
void Shader::setVec2(const String& name, float x, float y) { 
    glUniform2f(getUL(name), x, y); 
}
void Shader::setVec3(const String& name, const vec3& value) { 
    glUniform3fv(getUL(name), 1, &value[0]); 
}
void Shader::setVec3(const String& name, float x, float y, float z) { 
    glUniform3f(getUL(name), x, y, z); 
}
void Shader::setVec4(const String& name, const vec4& value) { 
    glUniform4fv(getUL(name), 1, &value[0]); 
}
void Shader::setVec4(const String& name, float x, float y, float z, float w) { 
    glUniform4f(getUL(name), x, y, z, w); 
}
void Shader::setMat2(const String& name, const mat2& mat) {
    glUniformMatrix2fv(getUL(name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const String& name, const mat3& mat) {
    glUniformMatrix3fv(getUL(name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const String& name, const mat4& mat) {
    glUniformMatrix4fv(getUL(name), 1, GL_FALSE, &mat[0][0]);
}


}