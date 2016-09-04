/*
GameFontTool 

MIT License

Copyright (c) 2016 Stepanov Dmitriy a.k.a mrDIMAS

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef GAME_FONT_TOOL_H
#define GAME_FONT_TOOL_H

#ifdef GFT_SHARED
    #ifdef _MSC_VER        
        #define GFT_API __declspec(dllexport)
    #elif defined(_GCC)
        #define GFT_API __attribute__((visibility("default")))
    #endif
#else 
    #define GFT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
    
/* 
gft_font_t is reference counted, so you can easily share pointer
and when you doesn't need it anymore, you call gft_font_free, 
which decreases reference counter and deletes object if refcounter == 0 
*/
typedef struct gft_font_t gft_font_t;
typedef struct gft_glyph_t gft_glyph_t;
typedef unsigned int gft_symbol_t;

typedef struct gft_texcoord_t {
    float x;
    float y;
} gft_texcoord_t;

/* atlas stored in RGBA8 format, so you can easily load it to GPU */
typedef struct gft_rgba_pixel_t {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} gft_rgba_pixel_t;

typedef enum gft_error_t {
    GFT_NO_ERROR, /* everything is ok */
    GFT_FREETYPE_ERROR, /* somethings went wrong inside freetype */
    GFT_NOT_ENOUGH_MEMORY, /* memory allocation failed */
    GFT_UNEXPECTED_END_OF_FILE, /* occures when you trying to load damaged gft file */
    GFT_BAD_FONT, /* font does not exist */
    GFT_BAD_PARAMETER, /* some parameter is wrong */
    GFT_BAD_CODE, /* symbol does not found in an atlas */
    GFT_UNKNOWN_FORMAT, /* occures when you trying to load non-gft file */
    GFT_UNABLE_TO_LOAD_FONT, /* something went wrong while loading font from gft file */
    GFT_UNABLE_TO_SAVE_FONT, /* something went wrong while saving font to gft file */
    GFT_UNABLE_TO_PACK_FONT /* something in font packing (into atlas) went wrong */
} gft_error_t;

typedef enum gft_atlas_options_t {
    GFT_DEFAULT = 0,
    GFT_ATLAS_STRICT_POW2_SIZE = 1 /* option for old GPU's that does not support non-pow2 textures */
} gft_atlas_options_t;
   
/* 
===============
gft_utf8_to_utf32
    - converts utf8 string to utf32 
    - inString is pointer to source string
===============
*/ 
GFT_API gft_error_t gft_utf8_to_utf32(const char * inString, gft_symbol_t * out);

/* 
===============
gft_utf32_strlen
    - computes utf32 string length
===============
*/ 
GFT_API gft_error_t gft_utf32_strlen(gft_symbol_t * utf32str, int * out);

/* 
===============
gft_font_create
    - loads font from *.tff, *.otf, ...
    - generates bitmap atlas with texture coordinates and metrics for each glyph
    - loaded font is ready to render    
    - loaded font can be saved to *.gft by gft_font_save
    - 'options' is combinations of something from gft_atlas_options_t
    - 'symbolSet' is an utf8 string with all symbols, that must be associated with this font, i.e. 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=+<>/.,АБВГДЕЁЖЗИКЛМНОПРСТУФХЦЧШЩЬЪЭЮЯ"!№;%:?"
===============
*/
GFT_API gft_error_t gft_font_create(const char * filename, float size, int options, const char * symbolSet, gft_font_t ** fontPtr);

/* 
===============
gft_font_load
    - load font atlas with texture coordinates and metrics for each glyph from *.gft file
    - can be used in pair with gft_font_create to create font cache on hard drive
===============
*/
GFT_API gft_error_t gft_font_load(const char * filename, gft_font_t ** fontPtr);

/* 
===============
gft_font_add_reference
    - adds reference to font
    - use this to share font between multiple objects
===============
*/
GFT_API gft_error_t gft_font_add_reference(gft_font_t * font);

/* 
===============
gft_font_save
    - saves font atlas with texture coordinates and metrics for each glyph to *.gft file
===============
*/
GFT_API gft_error_t gft_font_save(gft_font_t * font, const char * filename);

/* 
===============
gft_font_free
    - frees allocated memory for font
===============
*/
GFT_API gft_error_t gft_font_free(gft_font_t * font);

/* 
===============
gft_font_get_width
    - returns atlas width in pixels
===============
*/
GFT_API gft_error_t gft_font_get_width(gft_font_t * font, int * width);

/* 
===============
gft_font_get_height
    - returns atlas height in pixels
===============
*/
GFT_API gft_error_t gft_font_get_height(gft_font_t * font, int * height);

/* 
===============
gft_font_get_atlas_size
    - returns atlas size in bytes
===============
*/
GFT_API gft_error_t gft_font_get_atlas_size(gft_font_t * font, int * size);

/* 
===============
gft_font_get_atlas_pixels
    - returns raw pointer to atlas pixels
    - you can pass this pointer to glTexImage2D without any conversions
===============
*/
GFT_API gft_error_t gft_font_get_atlas_pixels(gft_font_t * font, gft_rgba_pixel_t ** pixels);

/* 
===============
gft_glyph_get_caret_offset_x
    - returns x caret offset for rendering
===============
*/
GFT_API gft_error_t gft_glyph_get_caret_offset_x(gft_font_t * font, gft_symbol_t symbol, int * offset);

/* 
===============
gft_glyph_get_caret_offset_y
    - returns y caret offset for rendering
===============
*/
GFT_API gft_error_t gft_glyph_get_caret_offset_y(gft_font_t * font, gft_symbol_t symbol, int * offset);

/* 
===============
gft_glyph_get_caret_step_x
    - returns x step of caret for rendering
===============
*/
GFT_API gft_error_t gft_glyph_get_caret_step_x(gft_font_t * font, gft_symbol_t symbol, int * step);

/* 
===============
gft_glyph_get_caret_step_y
    - returns y step of caret for rendering
===============
*/
GFT_API gft_error_t gft_glyph_get_caret_step_y(gft_font_t * font, gft_symbol_t symbol, int * step);

/* 
===============
gft_glyph_get_width
    - returns width of a glyph
===============
*/
GFT_API gft_error_t gft_glyph_get_width(gft_font_t * font, gft_symbol_t symbol, int * width);

/* 
===============
gft_glyph_get_height
    - returns height of a glyph
===============
*/
GFT_API gft_error_t gft_glyph_get_height(gft_font_t * font, gft_symbol_t symbol, int * height);

/*
gft_glyph_get_texcoords
    - note that returned texcoords starts in left bottom corner as shown below:
        1--->---2
        |       |
        ^       v
        |       |
        0---<---3
        
    texcoords - pointer to gft_texcoord_t[4]
*/
GFT_API gft_error_t gft_glyph_get_texcoords(gft_font_t * font, gft_symbol_t symbol, gft_texcoord_t * texcoords);

#ifdef __cplusplus
}
#endif

#endif
