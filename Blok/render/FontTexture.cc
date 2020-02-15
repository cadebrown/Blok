/* FontTexture.cc - Blok's FontTexture class implementation */

#include <Blok/Render.hh>

#include <Blok/Random.hh>

#include "./stb_image.h"


namespace Blok::Render {
    
Map<String, FontTexture*> FontTexture::cache;    


// load a new copy of the texture
// load a constant copy of the texture
FontTexture* FontTexture::loadConst(const String& path) {
    if (cache.find(path) == cache.end()) {
        return cache[path] = new FontTexture(path);
    } else {
        // we already loaded it, so just return a ref to it
        return cache[path];
    }
}

void FontTexture::addChar(char c) {

    if (charXYs.find(c) != charXYs.end()) {
        // it has already been added, so stop
        return;
    }

    // get the current slot of the glyph
    FT_GlyphSlot slot = ftFace->glyph;
    FT_UInt glyph_index;

    // ensure we can load the given character as a bitmap
    if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
        blok_error("Failed to load char '%c' from font '%s'", c, fontName.c_str());
        return;
    }

    // keep a reference to the bitmap
    FT_Bitmap* bitmap = &slot->bitmap;

    // output x, y coords
    int o_px = 0, o_py = 0;

    // for simplicity, we assume that `bitmap->pixel_mode'
    // is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)

    // iterate through the bitmap for this character
    for (int py = 0; py < bitmap->rows; ++py) {
        for (int px = 0; px < bitmap->width; ++px) {
            pixel cur = pixel(bitmap->buffer[py * bitmap->width + px]);
            pixels[(o_py + py) * width + (o_px + px)] = cur;
        }
    }

    // now, add the coordinates to the map
    charXYs[c] = { vec2i(o_px, o_py), vec2i(o_px + bitmap->width, o_py + bitmap->rows) };

}



// read a texture from a given path
FontTexture::FontTexture(const String& path) {

    width = 64;
    height = 64;

    fontName = path;

    pixels = (pixel*)malloc(sizeof(*pixels) * width * height);
    if (pixels == NULL) {
        blok_error("Internal malloc() error", path.c_str());
        return;
    }

    // first, zero out the memory
    for (int i = 0; i < width * height; ++i) pixels[i] = pixel(0);

    // try constructing a FreeType font-face from the given face
    if (FT_New_Face(ftlib, path.c_str(), 0, &ftFace)) {
        blok_error("Failed to load FreeType font '%s'!", path.c_str());
        return;
    }

    // font size in pixels
    int font_size = 32;

    // dpi resolution
    int font_dpi = 160;

    // set the font size
    // the '0's mean that they are calculated by the font face,
    // so we just set the height of the font, which is what we care about most of the time
    if (FT_Set_Char_Size(ftFace, font_size * 64, 0, font_dpi, 0)) {
        blok_error("Failed to set char size!");
        return;
    }


    // add a character
    addChar('C');
   // addChar('A');


    /*for ( n = 0; n < num_chars; n++ )
    {

    my_draw_bitmap( &slot->bitmap,
                    pen_x + slot->bitmap_left,
                    pen_y - slot->bitmap_top );
    }*/
/*
    int px = 0, py = height-1;
    char* t = bmp;
    while (*t != '\0') {
        if (*t == '\n') {
            py--;
            px = 0;
        } else {
            if (*t == '*') {
                pixels[py * width + px] = pixel(255, 255, 255, 255);
            }
            px++;
        }

        t++;
    }
*/

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

    blok_debug("Loaded font-texture '%s'", path.c_str());
}


// free resources from the texture
FontTexture::~FontTexture() {
    // free the pixels array
    free(pixels);
    // free our OpenGL object
    glDeleteTextures(1, &glTex);
}


}
