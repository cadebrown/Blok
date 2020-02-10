/* Blocc-Render.hh - Implementation of the rendering engine */

#pragma once
#ifndef BLOCC_RENDER_HH__
#define BLOCC_RENDER_HH__

#include <Blocc.hh>
#include <glm/glm.hpp>


namespace Blocc::Render {
    

    // a single vertex data
    struct Vertex {
        // position of the vertex (x, y, z)
        glm::vec3 pos;

        // the texture coordinates (u, v)
        glm::vec2 uv;

        // the tangent direction
        glm::vec3 T;
        // the bitangent direction
        glm::vec3 B;
        // the normal direction
        glm::vec3 N;

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

    class Mesh {

        private:

        // init the mesh
        void setup();

        public:
        
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

    // a render target, i.e. a texture that can be rendered to
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

    };


}


#endif




