/* Blok-Render.cc - rendering code */

#include <Blok-Render.hh>

// for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#include <fstream>
#include <sstream>

namespace Blok::Render {

void Mesh::setup() {
    // create buffers/arrays
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);

    glBindVertexArray(glVAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(faces[0]), &faces[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers

    // vertex Positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    // vertex texture coords
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    // vertex tangent
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, T));
    // vertex bitangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, B));
    // vertex normals
    glEnableVertexAttribArray(4);	
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, N));

    glBindVertexArray(0);
}



Target::Target(int width, int height, int numTex) {
    this->width = width;
    this->height = height;

    // create a frame buffer object to render to
    glGenFramebuffers(1, &glFBO); 

    glTex.resize(numTex);
    glColorAttachments.resize(numTex);

    // Create the gbuffer textures
    glGenTextures(numTex, &glTex[0]);

    for (int i = 0 ; i < numTex; i++) {
        glBindTexture(GL_TEXTURE_2D, glTex[i]);

        // default to RGBA in float format, to avoid any potential issues
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);


        // Poor filtering. Needed !

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glFBO);

        // and just allocate color attachments starting at 0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, glTex[i], 0);
        // record it
        glColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;

    }

    // create depth buffer
    glGenTextures(1, &glDepth);

    // depth buffer render object
    glBindTexture(GL_TEXTURE_2D, glDepth);
    // initialize it
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // set it as the depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glDepth, 0);

    // draw on all these bufers
    glDrawBuffers(numTex, &glColorAttachments[0]);

    // make sure we were sucecssful
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        b_error("While calling glCheckFramebufferStatus(), got 0x%x", Status);
    }

    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


Map<String, Texture*> Texture::cache;


Texture::Texture(String path) {
    int _w, _h, _channels;
    _pixels = (pixel *)stbi_load(path.c_str(), &_w, &_h, &_channels, 4); 
    width = _w, height = _h;
    if (_pixels == NULL) {
        b_error("Failed to read image file '%s'", path.c_str());
    }
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _pixels);
    glGenerateMipmap(GL_TEXTURE_2D); 

}

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
            b_error("Failed to compile shader '%s' (of type '%s'), error was: %s", name.c_str(), type.c_str(), infoLog);
            return false;
        }
    } else {
        // check program Status
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            b_error("Failed to link program '%s', error was: %s", name.c_str(), infoLog);
            return false;

        }
    }

    // there was no problem
    return true;
}

// declare the cache of existing shaders
Map<Pair<String, String>, Shader*> Shader::cache;

// map of paths to try
List<String> Shader::paths = { ".", ".." };


Shader* Shader::get(const String& vsFile, const String& fsFile) {
    Pair<String, String> key(vsFile, fsFile);
    if (cache.find(key) == cache.end()) {
        // construct it
        return cache[key] = new Shader(vsFile, fsFile);
    } else {
        return cache[key];
    }

}

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
            b_trace("Found specific shader shader with vs: %s, fs: %s", vsNew.c_str(), fsNew.c_str());
            break;

        } catch (std::ifstream::failure e) {
            // catch any error that came about
            b_trace("Failed to read specific shader shader with vs: %s, fs: %s (err: %s)", vsNew.c_str(), fsNew.c_str(), e.what());
            continue;
        }
    }

    if (!found) {
        b_error("Failed to read shader with vs: %s, fs: %s", vsFile.c_str(), fsFile.c_str());
        return;
    }


    // get the source code as C strings for the OpenGL library
    const char* vsCode = vsSrc.c_str();
    const char* fsCode = fsSrc.c_str();

    // now, compile the opengl shaders
    uint vsProg, fsProg;

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
    glID = glCreateProgram();
    // attach vertex -> fragment
    glAttachShader(glID, vsProg);
    glAttachShader(glID, fsProg);

    //if (geometryPath != nullptr) glAttachShader(glID, geometry);

    // attempt to link the program
    glLinkProgram(glID);
    if (!checkCompileErrors(glID, vsFile + "+" + fsFile, "PROGRAM")) {
        // error linking
    }

    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vsProg);
    glDeleteShader(fsProg);
    //if(geometryPath != nullptr) glDeleteShader(geometry);

}
void Shader::use()  {
    glUseProgram(glID); 
}

int Shader::getUL(const String& name) {
    return glGetUniformLocation(glID, name.c_str());
}

void Shader::setBool(const String& name, bool value) {         
    glUniform1i(glGetUniformLocation(glID, name.c_str()), (int)value); 
}
void Shader::setInt(const String& name, int value) { 
    glUniform1i(glGetUniformLocation(glID, name.c_str()), value); 
}
void Shader::setFloat(const String& name, float value) { 
    glUniform1f(glGetUniformLocation(glID, name.c_str()), value); 
}
void Shader::setVec2(const String& name, const vec2& value) { 
    glUniform2fv(glGetUniformLocation(glID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec2(const String& name, float x, float y) { 
    glUniform2f(glGetUniformLocation(glID, name.c_str()), x, y); 
}
void Shader::setVec3(const String& name, const vec3& value) { 
    glUniform3fv(glGetUniformLocation(glID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec3(const String& name, float x, float y, float z) { 
    glUniform3f(glGetUniformLocation(glID, name.c_str()), x, y, z); 
}
void Shader::setVec4(const String& name, const vec4& value) { 
    glUniform4fv(glGetUniformLocation(glID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec4(const String& name, float x, float y, float z, float w) { 
    glUniform4f(glGetUniformLocation(glID, name.c_str()), x, y, z, w); 
}
void Shader::setMat2(const String& name, const mat2& mat) {
    glUniformMatrix2fv(glGetUniformLocation(glID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const String& name, const mat3& mat) {
    glUniformMatrix3fv(glGetUniformLocation(glID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const String& name, const mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(glID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}



void Renderer::renderObj(mat4 gT) {
        // draw mesh
    glm::mat4 gPVM = gT;
    //glActiveTexture(GL_TEXTURE7); // activate the texture unit first before binding texture
    //glBindTexture(GL_TEXTURE_2D, mrc->tex->glTexture);
    //cout << shaders["geometry"]->getUL("texDiffuse") << endl;
    //shaders["geometry"]->setInt("texDiffuse", 7);
    //shaders["geometry"]->setMat4("gM", Tmat);
    //shaders["geometry"]->setMat4("gPVM", gPVM);
    //printf("RENDER\n");

    shaders["geometry"]->setMat4("gPVM", gPVM);

    printf("%d,%d\n", mymesh->faces.size() * 3, mymesh->vertices.size());

    //for (CDGE::Mesh mesh : mrc->model->meshes) {
        glBindVertexArray(mymesh->glVAO); 
        glDrawElements(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0);
    //}
}

void Renderer::render() {

    // perspective and view matrices
    mat4 gT = glm::perspective(glm::radians(120.0f / 2.0f), (float)width / height, 0.1f, 100.0f) ;
    gT[2] *= -1.0f;
    //gT = gT * glm::inverse(camera->gameObject->T.getMat());

    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);
    
    // FBO of our rendertarget
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // loop to draw geometry
    glUseProgram(shaders["geometry"]->glID);

    // renders each 
    //scene.traverse<WireframeRenderer, &WireframeRenderer::travRenderGameObject>(this);
    renderObj(gT);

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

/*
    // FBO of our rendertarget
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, targets["geometry"]->glFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // initialize our depth testing
    glEnable(GL_DEPTH_TEST); 
    //glDepthFunc(GL_LESS);
    

    clearColor[2] = glfwGetTime() / 10.0f;
    // set the background
    glClearBufferfv(GL_COLOR, 0, &clearColor[0]);

    // draw mesh
    //glm::mat4 gPVM = gT * Tmat;
    //glActiveTexture(GL_TEXTURE7); // activate the texture unit first before binding texture
    //glBindTexture(GL_TEXTURE_2D, mrc->tex->glTexture);

    //printf("%d\n", (int)mymesh.faces.size() * 3);

    Texture* mytex = Texture::get("../resources/grass.jpg");


    // start using the geometry shader
    shaders["geometry"]->use();

    glDrawBuffers(targets["geometry"]->glColorAttachments.size(), &targets["geometry"]->glColorAttachments[0]);

    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    //glBindTexture(GL_TEXTURE_2D, mrc->tex->glTexture);
    glBindTexture(GL_TEXTURE_2D, mytex->glTex);

    glBindVertexArray(mymesh->glVAO);
    glDrawElements(GL_TRIANGLES, mymesh->faces.size() * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    // loop to draw geometry
    //glUseProgram(shaders["geometry"]->glID);

    // renders each 
    //scene.traverse<WireframeRenderer, &WireframeRenderer::travRenderGameObject>(this);

    // draw to actual screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, targets["geometry"]->glFBO);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);*/


}



};
