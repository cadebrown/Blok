/* Texture.cc - Blok's Texture class implementation */

#include <Blok/Render.hh>

// use this file for a useful image loader
#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

namespace Blok::Render {

// initialize the cache for shared texture loads
Map<String, Texture*> Texture::cache;

// load a new copy of the texture
Texture* Texture::loadCopy(const String& path) {
    return new Texture(path);
}

// load a constant copy of the texture
Texture* Texture::loadConst(const String& path) {
    if (cache.find(path) == cache.end()) {
        return cache[path] = new Texture(path);
    } else {
        // we already loaded it, so just return a ref to it
        return cache[path];
    }
}


// read a texture from an image file
Texture::Texture(const String& fname) {
    int _w, _h, _channels;

    bool found = false;
    for (const String& path : paths) {

        String newpath = path + "/" + fname;

        pixels = (pixel *)stbi_load(newpath.c_str(), &_w, &_h, &_channels, 4); 
        width = _w, height = _h;
        if (pixels == NULL) {
            blok_trace("Failed to load Texture '%s'", newpath.c_str());
            continue;
        }

        found = true;
        blok_trace("Loaded Texture '%s'", newpath.c_str());
        break;
    }


    if (!found) {
        blok_error("Failed to load Texture '%s'", fname.c_str());
        return;
    }

    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    /*
    GLfloat largest_supported_anisotropy; 
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &largest_supported_anisotropy); 
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, largest_supported_anisotropy);
    */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D); 

    check_GL();
}


// free resources from the texture
Texture::~Texture() {
    // free the pixels array
    free(pixels);
    // free our OpenGL object
    glDeleteTextures(1, &glTex);
}


}
