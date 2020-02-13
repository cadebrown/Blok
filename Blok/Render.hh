/* Blok/Render.hh - Header file describing the rendering portion of the Blok library */

#pragma once
#ifndef BLOK_RENDER_HH__
#define BLOK_RENDER_HH__

/* main Blok library */
#include <Blok/Blok.hh>

/* additional graphics library from GLM */
#include <Blok/glm/gtx/transform.hpp>

namespace Blok::Render {


    /* TEXTURE/IMAGES */

    // define a datatype for storing an RGBA pixel value,
    // i.e. the main type used in image storage
    using pixel = glm::vec<4, uint8_t>;


    // Texture - a 2D bitmap image
    // NOTE: see file `Texture.cc` for the implementation
    class Texture {
        public:


        // a cache of constant textures that have been loaded
        static Map<String, Texture*> cache;

        // load a new copy of the texture
        // NOTE: the caller is responsible for deleting the texture after it is done
        //         , but it is allowed to modify the _pixels
        static Texture* loadCopy(const String& path);

        // load a constant, shared reference of the texture
        // NOTE: the caller should NOT free this texture, and it should also not
        //   modify any pixels
        static Texture* loadConst(const String& path);


        /* MEMBER VARS */

        // the size of the image, in pixels
        int width, height;

        // array of pixels, in row major order
        pixel* pixels;

        // the OpenGL handle for the texture object
        uint glTex;


        // constructs a texture from a given path
        // don't use this constructor, please use Texture::loadCopy if you need your own copy
        //   of the texture, or Texture::loadConst if you will not be modifying the texture
        Texture(const String& path);

        // deconstruct a texture, freeing its resources
        ~Texture();

        // return the index into the linear array 'pixels', given a row and column from the 
        //   top left of the image
        int getIndex(int row=0, int col=0) const {
            return width * row + col;
        }

        // get the pixel at the indicated coordinates
        pixel get(int row, int col) const {
            int idx = getIndex(row, col);
            return pixels[idx];
        }

        // set the pixel at the given location to a value, defaulting to black
        void set(int row, int col, pixel pix=pixel(0, 0, 0, 0)) {
            int idx = getIndex(row, col);
            pixels[idx] = pix;
        }

    };

    /* MESH/GEOMETRY */

    // Face - a collection of 3 indices into a vertex array, describing a triangular
    //   face for a mesh
    using Face = glm::vec<3, uint>;

    // Vertex - the most generic vertex, which supports all sorts of data send with it
    // NOTE: this is somewhat inefficient, as for most rendering tasks, you can get away with
    //   a smaller vertex format
    struct Vertex {
        // position of the vertex in model space (x, y, z)
        vec3 pos;

        // the texture coordinates (u, v)
        vec2 uv;

        // the Tangent direction
        vec3 T;
        // the Bitangent direction
        vec3 B;
        // the Normal direction
        vec3 N;

    };

    // Mesh: a 3D polygon, of the most generic variety. Implementation found in `render/Mesh.cc`
    class Mesh {
        public:

        // a cache of constant, shared meshes
        static Map<String, Mesh*> cache;

        // load a new copy of the mesh
        // NOTE: the caller is responsible for deleting the mesh after it is done
        //         , but it is allowed to modify the values
        static Mesh* loadCopy(const String& path);

        // load a constant, shared reference of the mesh
        // NOTE: the caller should NOT free this mesh, and it should also not
        //   modify any data
        static Mesh* loadConst(const String& path);
        
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

        // deconstruct/delete resources associated with a mesh
        ~Mesh();

    };


    /* RENDERING PROGRAMS/CONSTRUCTS */

    // Shader - a shader program, for drawing input points, see Shader.cc for more
    //   basically just a wrapper over the OpenGL construct
    class Shader {

        public:

        // the cache of shaders that already exist, keyed on <vs_file, fs_file>
        static Map<Pair<String, String>, Shader*> cache;

        // load a shader from the source files (vertex & fragment shader)
        // NOTE: The caller should not free or modify this shader, except for the
        //   set* methods, and using the program normally
        static Shader* load(const String& vsFile, const String& fsFile);

        // OpenGL program resource
        uint glProgram;

        // construct a shader given a file path for the vertex shader & fragment shader
        // NOTE: Don't use this, use the Shader::load() method instead
        Shader(const String& vsFile, const String& fsFile);

        // deconstruct/free shader's resources
        ~Shader();

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


    /* RENDERING TARGET */


    // Target : a target framebuffer that can be rendered to as an intermediate result, then
    //   output to the screen or some other texture
    // This render Target can have multiple color attachments, so for example, for a deferred renderer,
    //   you can have 4 (position, color, normal, UV)
    // See more in `render/Target.cc`
    class Target {
        public:

        // OpenGL handles to the frame buffer object (FBO), and the depth buffer
        uint glFBO, glDepth;

        // List of OpenGL handle textures that are a part of the render target
        List<uint> glTex;

        // This is the list of the GL_COLOR_ATTACHMENT* enumerations, describing the respective
        //   glTex entries
        List<GLenum> glColorAttachments;

        // width/height of the render target, in pixels
        int width, height;

        // create a render target with a given width/height and optional number of textures
        Target(int width, int height, int numTex=1);

    };

}

#endif /* BLOK_RENDER_HH__ */
