/* Texture.cc - Blok's Texture class */

#include <Blok-Render.hh>

// for image loading
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




// read a texture from a given path
Texture::Texture(String path) {
    int _w, _h, _channels;
    pixels = (pixel *)stbi_load(path.c_str(), &_w, &_h, &_channels, 4); 
    width = _w, height = _h;
    if (pixels == NULL) {
        b_error("Failed to read image file '%s'", path.c_str());
    }
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D); 

    b_debug("Loaded texture '%s'", path.c_str());


}


}
