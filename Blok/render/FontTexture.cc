/* FontTexture.cc - Blok's FontTexture class implementation */

#include <Blok/Render.hh>

namespace Blok::Render {
    
Map<String, FontTexture*> FontTexture::cache;    

// load a constant copy of the font texture
FontTexture* FontTexture::loadConst(const String& path) {
    if (cache.find(path) == cache.end()) {
        return cache[path] = new FontTexture(path);
    } else {
        // we already loaded it, so just return a ref to it
        return cache[path];
    }
}

void FontTexture::addChar(char c) {
    if (charInfos.find(c) != charInfos.end()) {
        // it has already been added, so stop
        return;
    }

    // get the current slot of the glyph
    FT_GlyphSlot slot = ftFace->glyph;

    // ensure we can load the given character as a bitmap
    if (FT_Load_Char(ftFace, c, FT_LOAD_RENDER)) {
        blok_error("Failed to load char '%c' from font '%s'", c, fontName.c_str());
        return;
    }

    // keep a reference to the bitmap
    FT_Bitmap* bitmap = &slot->bitmap;

    // output size
    int o_sx = bitmap->width, o_sy = bitmap->rows;

    // output x, y coords
    int o_px = 0, o_py = 0;

    if (charInfos.size() != 0) {
        
        // candidate position to start placing at
        vec2i cand{0, 0};


        // keep a variable for when we are done
        // it is set to false if there were any changes, so it goes through until there were no changes
        bool isRefined = false;

        // only try so many times, just in case
        int maxTries = 10000;
        int ct = 0;

        // a hueristic row alignment value
        int align = 64;

        // number of pixels to buffer between gluphs
        int buff = 2;

        while (!isRefined && ct < maxTries) {
            isRefined = true;


            int maxy = 0;

            for (auto& entry : charInfos) {
                // iterate through map of all of the allocated space
                Pair<vec2i, vec2i> pos{entry.second.texStart, entry.second.texStop};

                if (pos.second.y > maxy) maxy = pos.second.y;

                // check if we are intersecting the previous bounding box
                bool overX = (cand.x >= pos.first.x && cand.x < pos.second.x + buff) ||
                    (cand.x + o_sx >= pos.first.x && cand.x + o_sx < pos.second.x + buff);
                bool overY = (cand.y >= pos.first.y && cand.y < pos.second.y + buff) ||
                            (cand.y + o_sy >= pos.first.y && cand.y + o_sy < pos.second.y + buff);

                // check for overlap
                if (overX && overY) {
                    cand.x = pos.second.x + buff;
                    isRefined = false;
                }

                if (cand.x + o_sx >= width) {
                    // overflow to next line
                    cand.x = 0;
                    cand.y = pos.second.y + buff;
                    // add alignment to simulate rows
                    // I've noticed this results in more tighly packed maps most of the time,
                    // with the ability to still allocate space in most cases
                    cand.y += align - ((cand.y) % align);
                    isRefined = false;
                }
            }
            ct++;
        }

        //    if (cand.x >= charXYs)
        o_px = cand.x;
        o_py = cand.y;

    }

    // otherwise, just use 0, 0 as the starting point

    if (o_py + o_sy >= height || o_px + o_sx >= width) {
        // out of range, cannot add
        blok_warn("Failed to allocate space in the font texture for char '%c' from font '%s'", c, fontName.c_str());
        return;
    }


    // for simplicity, we assume that `bitmap->pixel_mode'
    // is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)

    // iterate through the bitmap for this character
    for (uint py = 0; py < bitmap->rows; ++py) {
        for (uint px = 0; px < bitmap->width; ++px) {
            pixel cur = pixel(bitmap->buffer[(bitmap->rows - py - 1) * bitmap->width + px]);
            pixels[(o_py + py) * width + (o_px + px)] = cur;
        }
    }

    // now, add the coordinates to the map
    charInfos[c] = { vec2i(o_px, o_py), vec2i(o_px + o_sx, o_py + o_sy), vec2i(slot->bitmap_left, slot->bitmap_top), (int)slot->advance.x };
}


// read a texture from a given path
FontTexture::FontTexture(const String& fname) {

    width = 1024;
    height = 1024;

    fontName = fname;

    pixels = (pixel*)malloc(sizeof(*pixels) * width * height);
    if (pixels == NULL) {
        blok_error("Internal malloc() error");
        return;
    }

    // first, zero out the memory
    for (int i = 0; i < width * height; ++i) pixels[i] = pixel(0);

    bool found = false;

    for (const String& path : paths) {
        // try and construct it
        String newpath = path + "/" + fname;

        // try constructing a FreeType font-face from the given face
        if (FT_New_Face(ftlib, newpath.c_str(), 0, &ftFace)) {
            blok_debug("Failed to load FontTexture font '%s'", newpath.c_str());
            continue;
        }

        blok_debug("Loaded FontTexture '%s'", newpath.c_str());
        found = true;
        break;
    }

    if (!found) {
        blok_error("Failed to load FontTexture '%s'", fname.c_str());
        return;
    }



    // font size in pixels
    int font_size = 48;

    // dpi resolution
    int font_dpi = 200;

    // set the font size
    // the '0's mean that they are calculated by the font face,
    // so we just set the height of the font, which is what we care about most of the time
    if (FT_Set_Char_Size(ftFace, font_size * 64, 0, font_dpi, 0)) {
        blok_error("Failed to set char size!");
        return;
    }

    // add standard characters
    //for (char c : "0123456789ab") {
    for (char c : "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ !@#$%^&*()_+`-=[]\\{}|;':\",./<>?") {
    //for (char c : "F") {
        addChar(c);
    }

    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    /*
    GLfloat largest_supported_anisotropy; 
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &largest_supported_anisotropy); 
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, largest_supported_anisotropy);
*/
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    //glGenerateMipmap(GL_TEXTURE_2D); 

    check_GL();

}


// free resources from the texture
FontTexture::~FontTexture() {
    // free the pixels array
    free(pixels);
    // free our OpenGL object
    glDeleteTextures(1, &glTex);
}


}
