/* Texture.cc - Blok's Texture class */

#include <Blok-Render.hh>

// for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

namespace Blok::Render {

// initialize the cache
Map<String, Texture*> Texture::cache;

// read a texture from a given path
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

    b_debug("Loaded texture '%s'", path.c_str());


}


}
