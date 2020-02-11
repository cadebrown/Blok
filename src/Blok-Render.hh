/* Blok-Render.hh - Implementation of the rendering engine */

#pragma once
#ifndef BLOK_RENDER_HH__
#define BLOK_RENDER_HH__

#include <Blok.hh>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace Blok::Render {

    // use the GLM library 
    using namespace glm;

    // for graphics
    using pixel = vec<4, uint8_t>;

    // a single vertex, including all neccessary data
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

    // a face is a set of 3 indices into an array of verteices. The 3 indicated
    //   by the indices form a triangle
    using Face = vec<3, uint>;

    // Mesh: a 3D polygon. Implementation found in `render/Mesh.cc`
    class Mesh {
        public:

        // load from a file
        static Mesh* load(const String& fname);

        // init the mesh
        void setup();
        
        // OpenGL handles to the Vertex Array Object, Vertex Buffer Object, and EBO
        // for rendering, you only care about VAO, and then drawing triangles from it,
        // which should have the data from 'vertices' and 'faces' in it
        uint glVAO, glVBO, glEBO;

        // list of verteices, in no particular order. They are indexed by 'faces' list
        List<Vertex> vertices;

        // list of all the faces, as triplets referring to indices in the 'vertices' list
        List<Face> faces;

        // construct a mesh from a list of vertices and faces.
        //   each face is a list of indexes into the vertices array, making up
        //   triangles
        Mesh(const List<Vertex>& vertices, const List<Face>& faces);
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


        // the field of view, in degrees
        float FOV;

        // position of the renderer in world space
        vec3 pos;

        // the 'up' direction of the renderer in world space
        vec3 up;

        // the direction the renderer is looking
        vec3 forward;


        // the current matrices cache (projection, view)
        mat4 gP, gV;

        // construct a new Renderer
        Renderer(int width, int height) {
            this->width = width;
            this->height = height;

            // position
            pos = vec3(0, 0, 0);

            // look forward in Z direction
            forward = vec3(0, 0, 1);

            // be looking directly up
            up = vec3(0, 1, 0);

            FOV = 120.0f;

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
            mymesh = Mesh::load("../resources/DefaultCube.obj");

        }
        

        ~Renderer() {
            // remove all created rendertargets
            for (auto keyval : targets) {
                delete keyval.second;
            }
        }

        // render a single entity
        void renderEntity(Entity* entity);

        // render an entire chunk
        void renderChunk(ChunkID id, Chunk* chunk);

        void renderObj(mat4 gT);

        // begin the rendering sequence
        void render_start();

        // finalize the rendering sequence
        void render_end();

    };


}


#endif




