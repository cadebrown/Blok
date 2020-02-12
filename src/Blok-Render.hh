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

    // a single RGBA pixel value
    using pixel = vec<4, uint8_t>;

    // a single vertex, including all neccessary data
    // NOTE: this is the most expansive definition of a vertex,
    //   most of the internal rendering uses more efficient storage
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


    // PackedBlockVertex : more efficient storage for specifically the main Packed Block format,
    // where everything is very efficient
    struct PackedBlockVertex {

        // position of this vertex (locally), packed:
        // [4: x] [4: y] [4: z] [3: xyz normal dir]
        // so pos & 0xF / 0xF gives the x position from 0-1, etc
        // (pos >> 12) & 0x7 gives three bits that tell whether it is facing forwards
        //   in the x, y, and z directions respectively
        //uint16_t pos;
        vec3 pos, normal;

        // the texture coordinates, packed:
        // [4: U] [4: V]
        // so uvp & 0xF / 0xF gives the U coordinate, etc
        vec2 uv;

        PackedBlockVertex(vec3 pos, vec3 normal, vec2 uv) {
            this->pos = pos;
            this->normal = normal;
            this->uv = uv;

/*
            pos = glm::clamp(pos, vec3(0), vec3(1));

            this->pos |= 0xFF & ((uint)pos.x * 256);
            this->pos |= (0xFF & ((uint)pos.y * 256)) << 4;
            this->pos |= (0xFF & ((uint)pos.z * 256)) << 8;

            uint8_t normalp = 0;
            if (normal.x > 0) normalp |= 1;
            if (normal.y > 0) normalp |= 2;
            if (normal.z > 0) normalp |= 4;
            this->pos |= normalp << 12;
*/
        }

    };


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



    class PackedBlockMesh {
        public:


        // construct these
        uint glVAO, glVBO, glEBO;

        List<PackedBlockVertex> vertices;

        List<Face> faces;

        void setup();

        PackedBlockMesh(const List<PackedBlockVertex>& vertices, const List<Face>& faces);

        // from a normal mesh
        PackedBlockMesh(Mesh* m);

    };

    // Texture: a 2D image, bit map style. Implementation found in `render/Texture.cc`
    class Texture {
        private:

        // cache of already existing textures
        static Map<String, Texture*> cache;

        public:

        // load a new copy of the texture
        // NOTE: the caller is responsible for deleting the texture after it is done
        //         , but it is allowed to modify the _pixels
        static Texture* loadCopy(const String& path);

        // load a constant, shared reference of the texture
        static Texture* loadConst(const String& path);


        // member variables

        // the size of the image, in pixels
        int width, height;

        // the OpenGL handle for the texture object
        uint glTex;

        // whether not something has been updated
        bool _flag;

        // the array of _pixel
        pixel* pixels;



        // don't use this constructor, please use Texture::loadCopy if you need your own copy
        //   of the texture, or Texture::loadConst if you will not be modifying the texture
        Texture(String path);



        // get a reference to a given pixel from the array
        pixel& get(int row, int col) {
            return pixels[width * row + col];
        }

    };


    // Shader: a shader program, for drawing input points, see render/Shader.cc for more
    class Shader {
        
        // the cache of shaders that already exist, keyed on <vs_file, fs_file>
        static Map<Pair<String, String>, Shader*> cache;

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


    // Renderer : the main class that does the rendering
    class Renderer {
        public:


        // the output width & height
        int width, height;
        PackedBlockMesh* mymesh;

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


        // the current queue of things that need to be rendered in the current frame
        struct RendererQueue {
            
            // all requested chunks that need to be rendered
            Map<ChunkID, Chunk*> chunks;

        } queue;

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
            //shaders["geometry"] = Shader::get("resources/geom.vs", "resources/geom.fs");
            shaders["geometry"] = Shader::get("resources/pmgeom.vs", "resources/pmgeom.fs");

            // construct basic mesh
            /*mymesh = new Mesh({
                { vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 0.0f) },
                { vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f) },
                { vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 1.0f) }
            }, {
                {0, 1, 2}
            });*/
            mymesh = new PackedBlockMesh(Mesh::load("../resources/DefaultCube.obj"));

        }

        // deconstruct the renderer
        ~Renderer() {
            // remove all created rendertargets
            for (auto keyval : targets) {
                delete keyval.second;
            }
        }



        // begin the rendering sequence
        void render_start();

        // add a chunk to be rendered
        // NOTE: must be between `render_start()` and `render_end()`!
        void renderChunk(ChunkID id, Chunk* chunk);

        // finalize the rendering sequence
        void render_end();



        // render a single entity
        //void renderEntity(Entity* entity);

        // render an entire chunk
        //void renderChunk(ChunkID id, Chunk* chunk);



    };


}


#endif




