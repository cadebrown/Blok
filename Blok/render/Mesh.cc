/* Mesh.cc - implementation of the Mesh class */

// include rendering library
#include <Blok/Render.hh>

// assimp libraries, for asset importing
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace Blok::Render {

// internal method to iterate through Assimp's structure
static Mesh* processMesh(aiMesh *mesh/*, const aiScene *scene*/) {

    List<Vertex> vertices;
    List<Face> faces;

    for(uint i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        vertex.N = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        //std::cout << glm::to_string(vertex.N) << std::endl;
        
        // UVs
        if (mesh->mTextureCoords[0]) {
            vertex.uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertex.T = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        vertex.B = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

        vertices.push_back(vertex);
    }

    // should work, since we triangulate all faces
    for(uint i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        faces.push_back((Face){face.mIndices[0], face.mIndices[1], face.mIndices[2] });
        //for (uint j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
    }

    // process materials
    //aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN
    /*
    // 1. diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    */
    
    // return a mesh object created from the extracted mesh data
    return new Mesh(vertices, faces);
}

// internal method to iterate through assimp's scenes
static void processNode(List<Mesh*>& meshes, aiNode *node, const aiScene *scene) {

    for(uint i = 0; i < node->mNumMeshes; i++) {
        meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]]/*, scene*/));
    }

    for(uint i = 0; i < node->mNumChildren; i++) {
        processNode(meshes, node->mChildren[i], scene);
    }

}


// load a mesh from a given file
Mesh* Mesh::loadCopy(const String& fname) {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fname, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        blok_error("Failed to load model '%s' (err: %s)", fname.c_str(), importer.GetErrorString());
        return NULL;
    }
    // retrieve the directory path of the filepath
    //directory = path.substr(0, path.find_last_of('/'));

    //int mesh_idx = scene->mRootNode->mMeshes[0];
    //printf("%i\n", mesh_idx);
    //return processMesh(scene->mMeshes[mesh_idx], scene);

    // process ASSIMP's root node recursively
    //processNode(scene->mRootNode, scene);

    List<Mesh*> meshes;
    processNode(meshes, scene->mRootNode, scene);

    // delete all meshes except for the first one, because that's the one we'll use
    for (uint i = 1; i < meshes.size(); ++i) {
        delete meshes[i];
    }

    blok_debug("Loaded model '%s'", fname.c_str());

    // return the first mesh found
    return meshes[0];
}



// construct a mesh from a list of vertices and faces.
//   each face is a list of indexes into the vertices array, making up
//   triangles
Mesh::Mesh(const List<Vertex>& vertices, const List<Face>& faces) {
    // just set them equal
    this->vertices = vertices;
    this->faces = faces;

    // create OpenGL handles for everything
    glGenVertexArrays(1, &glVAO);
    glGenBuffers(1, &glVBO);
    glGenBuffers(1, &glEBO);


    // now, innitialize the OpenGL information and send the vertex data to the GPU
    glBindVertexArray(glVAO);
    
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, glVBO);

    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * 3 * sizeof(int), &faces[0], GL_STATIC_DRAW);

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

// deconstruct the mesh
Mesh::~Mesh() {
    // just delete our OpenGL handles's resourceses
    glDeleteVertexArrays(1, &glVAO);
    glDeleteBuffers(1, &glVBO);
    glDeleteBuffers(1, &glEBO);
}



}
