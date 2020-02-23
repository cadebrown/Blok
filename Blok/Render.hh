/* Blok/Render.hh - Header file describing the rendering portion of the Blok library 
 *
 * Most objects here are just thin wrappers over OpenGL objects, with a much more friendly and intuitive interface
 * They are also (mostly) designed to be drop-ins for your project! So, just copy the class definition, as well
 *   as the `.cc` file located in `./render`, and maybe fix a few headers, and you have a good, portable implementation!
 * 
 */

#pragma once
#ifndef BLOK_RENDER_HH__
#define BLOK_RENDER_HH__

/* main Blok library */
#include <Blok/Blok.hh>

/* std libraries */
#include <algorithm>
#include <thread>
#include <mutex>

/* additional graphics library from GLM */
#include <Blok/glm/gtx/transform.hpp>

namespace Blok::Render {

    /* TEXTURE/IMAGES */

    // define a datatype for storing an RGBA pixel value,
    // i.e. the main type used in image storage
    using pixel = glm::vec<4, uint8_t>;

    // define a datatype for storing an RGBA pixel value, in floating point
    // this is only really used for higher quality renderings/textures
    using pixelf = glm::vec<4, float>;

    // Texture - a 2D bitmap image. Currently, no support for 3D. Maybe I will implement seperate classes,
    //   Texture1D, Texture2D, etc...
    // NOTE: see file `render/Texture.cc` for the implementation
    // TODO: make textures updatable
    class Texture {
        public:

        // a cache of constant textures that have been loaded
        static Map<String, Texture*> cache;

        // load a new copy of the texture
        // NOTE: the caller is responsible for deleting the texture after it is done
        //         , but it is allowed to modify the pixels array
        static Texture* loadCopy(const String& fname);

        // load a constant, shared reference of the texture
        // NOTE: the caller should NOT free this texture, and it should also not
        //   modify any pixels
        static Texture* loadConst(const String& fname);


        /* MEMBER VARS */

        // the size of the image, in pixels
        int width, height;

        // array of pixels, in row major order
        pixel* pixels;

        // the OpenGL handle for the texture object
        uint glTex;


        // constructs a texture from a given image file
        // don't use this constructor, please use Texture::loadCopy if you need your own copy
        //   of the texture, or Texture::loadConst if you will not be modifying the texture
        Texture(const String& fname);

        // deconstruct a texture, freeing its resources
        ~Texture();

        // return the index into the linear array 'pixels', given a row and column from the 
        //   top left of the image. Defaults to the first element, i.e. the top-left most pixel
        int getIndex(int row=0, int col=0) const {
            return width * row + col;
        }

        // get the pixel at the indicated coordinates, starting at the top left
        pixel get(int row=0, int col=0) const {
            int idx = getIndex(row, col);
            return pixels[idx];
        }

        // set the pixel at the given location to a value, defaulting to black (i.e. clears it)
        void set(int row, int col, pixel pix=pixel(0, 0, 0, 0)) {
            int idx = getIndex(row, col);
            pixels[idx] = pix;
        }

    };


    // FontTexture - an abstraction representing a renderable font's atlas of characters,
    //   which each have allocated 2D regions on a larger texture map
    class FontTexture {
        public:

        // cache of already existing fonts
        static Map<String, FontTexture*> cache;    
    
        // load a constant, shared reference of the font-texture
        // NOTE: the caller should NOT free this texture, and it should also not
        //   modify any pixels
        static FontTexture* loadConst(const String& path);

        /* MEMBER VARS */

        // the name of the font
        String fontName;

        // the size of the image, in pixels
        int width, height;

        // array of pixels, in row major order
        pixel* pixels;


        // the OpenGL handle for the texture object
        uint glTex;

        // the FreeType handle for the face object
        FT_Face ftFace;


        // define a structure describing the character info in the font
        struct CharInfo {
            
            // the start and stop in the texture atlas (glTex), in pixels
            // image is found in: [start, stop)
            vec2i texStart, texStop;

            // offset to the top left of the character from the 'real' glyph,
            // for rendering methods to align it properly
            vec2i bearing;

            // how far to advance after drawing the character, in horizontal units
            // you will want to multiply by 64 for actual pixels
            int advance;
        };

        // list of start, stop positions for given characters in the main texture
        Map<char, CharInfo > charInfos;

        // add a character to the bitmap,
        //   which will modify the 'charInfos' map
        void addChar(char c);

        // constructs a texture from a given path
        // don't use this constructor, please use Texture::loadCopy if you need your own copy
        //   of the texture, or Texture::loadConst if you will not be modifying the texture
        FontTexture(const String& fname);

        // deconstruct a texture, freeing its resources
        ~FontTexture();

        // return the index into the linear array 'pixels', given a row and column from the 
        //   top left of the image
        int getIndex(int row=0, int col=0) const {
            return width * row + col;
        }

        // get the pixel at the indicated coordinates
        pixel get(int row=0, int col=0) const {
            int idx = getIndex(row, col);
            return pixels[idx];
        }

        // set the pixel at the given location to a value, defaulting to black
        void set(int row, int col, pixel pix=pixel(0, 0, 0, 0)) {
            int idx = getIndex(row, col);
            pixels[idx] = pix;
        }

    };


    // UIText - class to render 2D text on the screen
    // TODO: implement effects/outlines
    // HERE: https://stackoverflow.com/questions/52086217/outline-fonts-with-true-type-and-opengl-core-3-3
    class UIText {
        public:

        // what is the font to render
        FontTexture* font;

        // the OpenGL handles to the VAO and VBO objects for the array of screen quads to render
        uint glVAO, glVBO;

        // number of triangles to render
        int tris;

        // current text being rendered
        String text;

        // the cache of previous values
        struct {
            
            // the text that was last rendered
            String lastText;

            // last value of max width
            float lastMaxWidth;

        } cache;

        // the maximum width (in pixels) for the UIText to render to
        float maxWidth;

        // construct a UIText object from a given font
        // NOTE: set the text after this to actually see something
        UIText(FontTexture* font);

        // recalculate the VBO object
        void calcVBO();
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
    // TODO: make meshes updateable
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
        
        // return the constant screen space quad for OpenGL, i.e. that will take up the entire screen by default
        static Mesh* getConstSSQ() {
            return loadConst("assets/obj/SSQ.obj");
        }

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

    // the vertex data for a chunk mesh
    // We can store things like ambient occlusion here, block ID, etc to help with rendering
    struct ChunkMeshVertex {
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

        // the block ID at this vertex
        float blockID;

        // ambient occlusion
        float ao;

        // ambient occlusion?
        ChunkMeshVertex(vec3 pos, vec2 uv, vec3 N, int blockID, float ao) {
            this->pos = pos;
            this->uv = uv;
            this->N = N;
            this->blockID = blockID;
            this->ao = ao;
        }

    };

    // ChunkMesh - generate a mesh from a chunk, with fast bindings to update it when it changes
    //   by recalculating the visual geometry
    // See implementation in `render/ChunkMesh.cc`
    class ChunkMesh {
        public:

        // OpenGL handles to the Vertex Array Object, Vertex Buffer Object, and EBO
        // for rendering, you only care about VAO, and then drawing triangles from it,
        // which should have the data from 'vertices' and 'faces' in it
        uint glVAO, glVBO, glEBO;

        // list of verteices, in no particular order. They are indexed by 'faces' list
        List<ChunkMeshVertex> vertices;

        // list of all the faces, as triplets referring to indices in the 'vertices' list
        List<Face> faces;

        // recalculate the mesh, do not update OpenGL though
        void update(Chunk* chunk);

        // construct a new chunk mesh, with nothing in it.
        // call `update(chunk)` to cause a recalculation
        ChunkMesh();

        // deconstruct/delete resources associated with a mesh
        ~ChunkMesh();

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

        // OpenGL handle to the shader's program object
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

        /* setting uniform values in the shader, note that you should first call `shader->use()`, so only call
             these when it is the active shader */

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
    //   output to the screen or some other texture or other target
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

        // resize a rendering target to a given size
        void resize(int w, int h);

    };


    // Renderer : a construct for rendering the entire game state, including chunks, entities, 
    //   GUIs, markup, etc
    // This should be the primary object calling OpenGL rendering commands
    // Internally, it uses a rendering pipeline that is abstracted from the client. So, you should just
    //   call the `renderX()` methods, and trust that the internal rendering process makes it efficient (which it does ;) )
    class Renderer {
        public:

        // the width/height (in pixels) of the actual screen output
        int width, height;

        // various render targets, for different stages in processing
        Map<String, Target*> targets;

        // various shaders that are used
        Map<String, Shader*> shaders;


        // array of freely allocated ChunkMeshes
        List<ChunkMesh*> chunkMeshPool;

        // chunk mesh requests
        Set<Chunk*> chunkMeshRequests;
        
        // chunk mesh objects
        Map<Chunk*, ChunkMesh*> chunkMeshes;


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


            // the list of misc. meshes to render
            // See here: https://computergraphics.stackexchange.com/questions/37/what-is-the-cost-of-changing-state/46#46
            Map<Mesh*, List<mat4> > meshes;

            // list of strings to render on screen along with their positions
            Map<FontTexture*, List< Pair<vec2, UIText*> > > texts;

            // list of lines to debug, in:
            // <start, color, end, color>
            List< std::array<vec3, 4> > lines;

        } queue;

        // struct describing debug drawing operations
        struct {

            // VAO/VBO for the debug lines
            uint glLinesVAO, glLinesVBO;

        } debug;


        // keep stats about rendering
        struct Stats {
            
            // time spent processing chunks
            double t_chunks;

            // number of chunks processed
            int n_chunks;

            // number of chunk recalculations
            int n_chunk_recalcs;

            // number of triangles (total) send to OpenGL
            int n_tris;

            Stats() {
                // reset all statistics by default
                t_chunks = 0.0;
                n_chunks = 0;
                n_chunk_recalcs = 0;
                n_tris = 0;
            }

        } stats;

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

            // default to a wide POV
            FOV = 90.0f;

            // add a nice default color
            clearColor = vec3(0.1f, 0.1f, 0.1f);

            // construct our geometry pass
            targets["GEOM"] = new Target(width, height, 5);
            
            // construct a basic lighting pass
            targets["LBASIC"] = new Target(width, height, 1);

            //targets["ssq"] = new Target(width, height, 1);

            // get shaders for the geometry scene pass
            shaders["GEOM_ChunkMesh"] = Shader::load("assets/shaders/GEOM_ChunkMesh.vert", "assets/shaders/GEOM_ChunkMesh.frag");
            shaders["GEOM_Mesh"] = Shader::load("assets/shaders/GEOM_Mesh.vert", "assets/shaders/GEOM_Mesh.frag");

            // get shaders for the LBASIC pass
            shaders["LBASIC"] = Shader::load("assets/shaders/LBASIC.vert", "assets/shaders/LBASIC.frag");

            // the shader for the reticle
            shaders["Reticle"] = Shader::load("assets/shaders/Reticle.vert", "assets/shaders/Reticle.frag");

            // shader for the debugging thin lines
            shaders["DebugLine"] = Shader::load("assets/shaders/DebugLine.vert", "assets/shaders/DebugLine.frag");

            // shader for rendering text
            shaders["TextQuad"] = Shader::load("assets/shaders/TextQuad.vert", "assets/shaders/TextQuad.frag");
            
            // allocate the debug lines
            glGenVertexArrays(1, &debug.glLinesVAO);
            glGenBuffers(1, &debug.glLinesVBO);

            // now, set them up
            glBindVertexArray(debug.glLinesVAO);
            glBindBuffer(GL_ARRAY_BUFFER, debug.glLinesVBO);

            // add position vertex attribute
            glEnableVertexAttribArray(0);	
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3), (void*)0);

            // add color vertex attribute
            glEnableVertexAttribArray(1);	
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3), (void*)sizeof(vec3));

            glBindVertexArray(0);

            check_GL();
        }


        // deconstruct the renderer
        ~Renderer() {
            // remove all created rendertargets
            for (auto keyval : targets) {
                delete keyval.second;
            }

            // free our kept pools
            for (auto cmesh : chunkMeshPool) {
                delete cmesh;
            }

            /*for (auto keyval : chunkMeshes) {
                // clear any existing chunk meshes
                delete keyval.second;
            }*/
            chunkMeshPool.clear();

            // remove/delete the debug lines variables
            glDeleteVertexArrays(1, &debug.glLinesVAO);
            glDeleteBuffers(1, &debug.glLinesVBO);
        }

        // get the final output target of the renderer
        Target* getOutputTarget() {
            // for now, just output the geometry pass
            return targets["GEOM"];
        }

        // resize the rendering engine to a new output size
        void resize(int w, int h);

        
        // render a mesh with a given transform
        void renderMesh(Mesh* mesh, mat4 T);

        // render a 'debug' line, i.e. a 1px line with a given start & end point, with a color
        void renderDebugLine(vec3 start, vec3 end, vec3 col={1, .4, .3});

        // render a string of text at a given screen location `pxy`
        void renderText(vec2 pxy, UIText* text, vec2 scalexy={1,1});

        // request for the renderer to render a chunk of the world
        void renderChunk(ChunkID id, Chunk* chunk);


        // finalize, and render out the entire queue
        void renderFrame();

    };


}

#endif /* BLOK_RENDER_HH__ */
