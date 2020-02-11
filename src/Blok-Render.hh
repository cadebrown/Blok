/* Blok-Render.hh - Implementation of the rendering engine */

#pragma once
#ifndef BLOCC_RENDER_HH__
#define BLOCC_RENDER_HH__

#include <Blok.hh>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace Blok::Render {

    // use the GLM library 
    using namespace glm;

    // for graphics
    using pixel = vec<4, uint8_t>;

    // a single vertex data
    struct Vertex {
        // position of the vertex (x, y, z)
        vec3 pos;

        // the texture coordinates (u, v)
        vec2 uv;

        // the tangent direction
        vec3 T;
        // the bitangent direction
        vec3 B;
        // the normal direction
        vec3 N;

    };

    // a face, which is a reference to vertexes, implicitly linking them
    //   as a triangle
    struct Face {
        
        // the index of the first vertex
        int i;

        // index of the second vertex
        int j;

        // index of the 3rd vertex
        int k;

    };

    // Mesh: a 3D polygon. Implementation found in `render/Mesh.cc`
    class Mesh {

        private:

        // init the mesh
        void setup();

        public:

        // load from a file
        static Mesh* load(const String& fname);
        
        // openGL handles
        uint glVAO, glVBO, glEBO;

        // list of vertexes
        List<Vertex> vertices;

        // list of all the faces
        List<Face> faces;

        // construct a mesh
        Mesh(const List<Vertex>& vertices, const List<Face>& faces) {
            this->vertices = vertices;
            this->faces = faces;

            setup();
        }

    };

    // Texture: a 2D image. Implementation found in `render/Texture.cc`
    class Texture {
        private:

        // cache of already existing textures
        static Map<String, Texture*> cache;

        public:

        static Texture* get(String path) {
            if (cache.find(path) == cache.end()) {
                return cache[path] = new Texture(path);
            } else {
                return cache[path];
            }
        }

        // the openGL handle
        uint glTex;

        // the dimensions
        int width, height;

        // whether not something has been updated
        bool _flag;

        // the array of _pixel
        pixel * _pixels;

        // don't use this; use Texture::get
        Texture(String path);

        pixel operator()(int row, int col) const {
            return _pixels[col + row * width];
        }

        void operator()(int row, int col, pixel px) {
            // keep track if there's been modifications
            _flag = true;
            _pixels[col + row * width] = px;
        }

    };


    // Shader: wrapper over the OpenGL shader program, see render/Shader.cc for more
    class Shader {
        
        // the cache of shaders that already exist, keyed on <vs_file, fs_file>
        static Map<Pair<String, String>, Shader*> cache;

        // list of paths to look for shaders
        static List<String> paths;

        public:

        // get a shader
        static Shader* get(const String& vsFile, const String& fsFile);

        // opengl program ID
        uint glID;

        // construct a shader given a file path for the vertex shader & fragment shader
        // NOTE: Don't use this, use the Shader::get() method instead
        Shader(const String& vsFile, const String& fsFile);

        // use this shader as the one for rendering
        void use();

        // returns the uniform location of a given name
        int getUL(const String& name);


        /* setting uniform values in the shader */

        void setBool  (const String& name, bool value);
        void setInt   (const String& name, int value);
        void setFloat (const String& name, float value);

        void setVec2  (const String& name, const vec2& value);
        void setVec2  (const String& name, float x, float y);
        void setVec3  (const String& name, const vec3& value);
        void setVec3  (const String& name, float x, float y, float z);
        void setVec4  (const String& name, const vec4& value);
        void setVec4  (const String& name, float x, float y, float z, float w);

        void setMat2  (const String& name, const mat2& mat);
        void setMat3  (const String& name, const mat3& mat);
        void setMat4  (const String& name, const mat4& mat);
    };

    // a render target, i.e. a texture that can be rendered to. See render/Target.cc for more
    class Target {
        public:

        // opengl handles
        uint glFBO, glDepth;

        // openGL textures
        List<uint> glTex;

        // attachments to render to
        List<GLenum> glColorAttachments;

        // width/height of the render target
        int width, height;

        // create a render target with a given width/height and optional number of textures
        Target(int width, int height, int numTex=1);

    };


    // actual class that does the rendering
    class Renderer {
        public:

        // the output width & height
        int width, height;
        Mesh* mymesh;

        // various render targets, for different stages in processing
        Map<String, Target*> targets;

        // various shaders
        Map<String, Shader*> shaders;

        // the default background color
        vec3 clearColor;

        // construct a new Renderer
        Renderer(int width, int height) {
            this->width = width;
            this->height = height;

            // add a nice default color
            clearColor = vec3(0.1f, 0.1f, 0.1f);

            // construct our geometry pass
            targets["geometry"] = new Target(width, height, 1);

            // construct the main geometry pass
            shaders["geometry"] = Shader::get("resources/geom.vs", "resources/geom.fs");

            // construct basic mesh
            /*mymesh = new Mesh({
                { vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f) },
                { vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f) },
                { vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f) }
            }, {
                {0, 1, 2}
            });*/
            mymesh = Mesh::load("../resources/suzanne.obj");

        }
        

        ~Renderer() {
            // remove all created rendertargets
            for (auto keyval : targets) {
                delete keyval.second;
            }
        }

        void renderObj(mat4 gT);

        void render();

    };


}


#endif




