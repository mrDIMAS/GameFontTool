/*
GameFontTool 

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

#include "gamefonttool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ft2build.h>
#include <freetype.h>
#include <math.h>

#define GFT_SIGNATURE "GFT0"

#pragma pack(push, 1)
typedef struct gft_header_t {
    char signature[4];
    int version;
    int glyphCount;
    int atlasWidth;
    int atlasHeight;
    int fontSize;
} gft_header_t;
#pragma pack(pop)

typedef struct gft_rect_t {
    int x;
    int y;
    int w;
    int h;
} gft_rect_t;

struct gft_glyph_t {
    int code; /* unicode position */
    int width;
	int height;
	gft_rgba_pixel_t * pixels;
	int advanceX;
	int advanceY;
	int bitmapTop;
	int bitmapLeft;
	gft_texcoord_t texCoords[4];
};

typedef struct gft_packer_node_t {
	gft_glyph_t * glyph;
	char split;
	gft_rect_t rect;
	struct gft_packer_node_t * children[2];
} gft_packer_node_t;

struct gft_font_t {
    int refCounter;
    float size;
    int glyphCount;
    gft_glyph_t * glyphs;
    gft_rgba_pixel_t * pixels;
    int atlasWidth;
    int atlasHeight;
    int options;
};

#define MASKBITS 0x3F
#define MASKBYTE 0x80
#define MASK2BYTES 0xC0
#define MASK3BYTES 0xE0
#define MASK4BYTES 0xF0
#define MASK5BYTES 0xF8
#define MASK6BYTES 0xFC

/* rewritten http://www.codeguru.com/cpp/misc/misc/multi-lingualsupport/article.php/c10451/The-Basics-of-UTF8.htm */
gft_error_t gft_utf8_to_utf32(const char * inString, gft_symbol_t * out) {
    int i = 0;
    int n = 0;
    unsigned char * in = (unsigned char *)inString;
    int size = strlen(inString);  
 
    for(i = 0; i < size; ) {
        unsigned int ch = 0;
        
        if((in[i] & MASK6BYTES) == MASK6BYTES) {
            /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
            ch = ((in[i] & 0x01) << 30) | ((in[i+1] & MASKBITS) << 24) | ((in[i+2] & MASKBITS) << 18) | ((in[i+3] & MASKBITS) << 12) | ((in[i+4] & MASKBITS) << 6) | (in[i+5] & MASKBITS);
            i += 6;
        } else if((in[i] & MASK5BYTES) == MASK5BYTES) {
            /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
            ch = ((in[i] & 0x03) << 24) | ((in[i+1] & MASKBITS) << 18) | ((in[i+2] & MASKBITS) << 12) | ((in[i+3] & MASKBITS) << 6) | (in[i+4] & MASKBITS);
            i += 5;
        } else if((in[i] & MASK4BYTES) == MASK4BYTES) {
            /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
            ch = ((in[i] & 0x07) << 18) | ((in[i+1] & MASKBITS) << 12) | ((in[i+2] & MASKBITS) << 6) | (in[i+3] & MASKBITS);
            i += 4;
        } else if((in[i] & MASK3BYTES) == MASK3BYTES) {
            /* 1110xxxx 10xxxxxx 10xxxxxx */
            ch = ((in[i] & 0x0F) << 12) | ((in[i+1] & MASKBITS) << 6)
            | (in[i+2] & MASKBITS);
            i += 3;
        } else if((in[i] & MASK2BYTES) == MASK2BYTES) { 
            /* 110xxxxx 10xxxxxx */
            ch = ((in[i] & 0x1F) << 6) | (in[i+1] & MASKBITS);
            i += 2;
        } else if(in[i] < MASKBYTE) {
            /* 0xxxxxxx */
            ch = in[i];
            i += 1;
        }

        out[n++] = ch;
    }
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_utf32_strlen
====================================
*/
gft_error_t gft_utf32_strlen(gft_symbol_t * utf32str, int * out) {
    gft_symbol_t * s = NULL;
    
    if(!out || !utf32str) {
        return GFT_BAD_PARAMETER;
    }
    
    s = utf32str;
    
    *out = 0;
    while(*s) {
        ++(*out);
        ++s;
    }
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_packer_find_place_to_insert
====================================
*/
static gft_packer_node_t * gft_packer_find_place_to_insert(gft_packer_node_t * node, gft_glyph_t * glyph) {
    float dw = 0, dh = 0;
	if (node->split) {
		gft_packer_node_t * newNode = gft_packer_find_place_to_insert(node->children[0], glyph);
		if (newNode) {
			return newNode;
		} else {
			return gft_packer_find_place_to_insert(node->children[1], glyph);
		}
	} else {
		if (node->glyph) { /* node already filled */
			return NULL;
		} else {
			if (node->rect.w < glyph->width || node->rect.h < glyph->height) { /* too small to fit */
				return NULL;
			} else {
				if (node->rect.w == glyph->width && node->rect.h == glyph->height) { /* fits perfectly */
					node->glyph = glyph;
					return node;
				} else { /* break node in two parts */
					node->children[0] = calloc(sizeof(*node->children[0]), 1);
					node->children[1] = calloc(sizeof(*node->children[1]), 1);

					dw = node->rect.w - glyph->width;
					dh = node->rect.h - glyph->height;

					if (dw > dh) {
						node->children[0]->rect.x = node->rect.x;
						node->children[0]->rect.y = node->rect.y;
						node->children[0]->rect.w = glyph->width;
						node->children[0]->rect.h = node->rect.h;

						node->children[1]->rect.x = node->rect.x + glyph->width;
						node->children[1]->rect.y = node->rect.y;
						node->children[1]->rect.w = node->rect.w - glyph->width;
						node->children[1]->rect.h = node->rect.h;
					} else {
						node->children[0]->rect.x = node->rect.x;
						node->children[0]->rect.y = node->rect.y;
						node->children[0]->rect.w = node->rect.w;
						node->children[0]->rect.h = glyph->height;

						node->children[1]->rect.x = node->rect.x;
						node->children[1]->rect.y = node->rect.y + glyph->height;
						node->children[1]->rect.w = node->rect.w;
						node->children[1]->rect.h = node->rect.h - glyph->height;
					}

					node->split = 1;

					return gft_packer_find_place_to_insert(node->children[0], glyph);
				}
			}
		}
	}

	return NULL;
}

/*
====================================
gft_glyph_packer_free
====================================
*/
static void gft_glyph_packer_free(gft_packer_node_t * node) {
    int i;
	if (node) {
		if (node->split) {
			for (i = 0; i < 2; ++i) {
				gft_glyph_packer_free(node->children[i]);
			}
		}
		free(node);
	}
}

/*
====================================
gft_ceil_pow2
====================================
*/
static unsigned int gft_ceil_pow2(unsigned int v) {
	unsigned int power = 1;
	while (v >>= 1) {
		power <<= 1;
	}
	power <<= 1;
	return power;
}

/*
====================================
gft_font_compute_atlas_size
    - computes atlas size (side length of rectangle)
====================================
*/
static int gft_font_compute_atlas_size(gft_font_t * font) {
    int totalArea = 0;
    int i = 0;
    int size = 0;
    
    /* compute total area of all glyphs */
    for(i = 0; i < font->glyphCount; ++i) {
        totalArea += font->glyphs[i].width * font->glyphs[i].height;
    }
    
    /* 
    size of atlas is the square root of totalArea + 10% 
    by the way, 10% is empirical value and this value must be tested on 
    various count of fonts to be sure, that is good value for all cases
    */
    size = sqrt(totalArea) * 1.1f;
    
    /* for old gpu's adjust size to be power of two */
    if(font->options & GFT_ATLAS_STRICT_POW2_SIZE) {
        size = gft_ceil_pow2(size);
    }

    return size;
}

/*
====================================
gft_font_pack
====================================
*/
static gft_error_t gft_font_pack(gft_font_t * font) {
    int i = 0;
    int k = 0;
    int row = 0;
    int col = 0;
    int srcRow = 0;
    int srcCol = 0;
    int atlasSize = gft_font_compute_atlas_size(font);
    float uScale = 0, vScale = 0;
    float uOffset = 0, vOffset = 0;
    int rowEnd = 0;
    int colEnd = 0;
	gft_packer_node_t * root = NULL;
    
    /* alloc root packer node */
    root = calloc(sizeof(*root), 1);
	root->rect.w = atlasSize;
    root->rect.h = atlasSize;

    /* prepare atlas */
	font->pixels = malloc(atlasSize * atlasSize * sizeof(*font->pixels));
	for (i = 0; i < atlasSize * atlasSize; ++i) {
		font->pixels[i].a = 0;
        font->pixels[i].r = 255;
        font->pixels[i].g = 255;
        font->pixels[i].b = 255;
	}
    
    font->atlasWidth = atlasSize;
    font->atlasHeight = atlasSize;

    /* pack each glyph into atlas and copy it's pixels to atlas */
	for (i = 0; i < font->glyphCount; ++i) {
		gft_glyph_t * glyph = &font->glyphs[i];
		gft_packer_node_t * node = gft_packer_find_place_to_insert(root, glyph);
		if (node) {
            /* set texcoords as unit rectangle */
			glyph->texCoords[0].x = 0.0f;
            glyph->texCoords[0].y = 0.0f;
            
			glyph->texCoords[1].x = 1.0f;
            glyph->texCoords[1].y = 0.0f;
            
			glyph->texCoords[2].x = 1.0f;
            glyph->texCoords[2].y = 1.0f;
            
			glyph->texCoords[3].x = 0.0f;
            glyph->texCoords[3].y = 1.0f;

			/* remap unit texture coordinates to atlas */
			uScale = (float)node->rect.w / (float)atlasSize;
			vScale = (float)node->rect.h / (float)atlasSize;
			uOffset = (float)node->rect.x / (float)atlasSize;
			vOffset = (float)node->rect.y / (float)atlasSize;

			for (k = 0; k < 4; ++k) {
				glyph->texCoords[k].x = glyph->texCoords[k].x * uScale + uOffset;
				glyph->texCoords[k].y = glyph->texCoords[k].y * vScale + vOffset;
			}

			rowEnd = node->rect.y + node->rect.h;
			colEnd = node->rect.x + node->rect.w;

            /* copy glyph pixels to atlas pixels */
			for (row = node->rect.y, srcRow = 0; row < rowEnd; row++, srcRow++) {
				for (col = node->rect.x, srcCol = 0; col < colEnd; col++, srcCol++) {
					font->pixels[row * atlasSize + col].a = glyph->pixels[srcRow * node->rect.w + srcCol].a;
				}
			}
		} else {
			return GFT_UNABLE_TO_PACK_FONT;
		}
	}

	gft_glyph_packer_free(root);

    /* free pixels for each glyph */
	for (i = 0; i < font->glyphCount; ++i) {
		free(font->glyphs[i].pixels);
        font->glyphs[i].pixels = NULL;
	}
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_font_create
====================================
*/
gft_error_t gft_font_create(const char * filename, float size, int options, const char * symbolSet, gft_font_t ** fontPtr) {
    FT_Library ftLibrary = NULL;
    FT_Face face = NULL;    
    gft_font_t * font = NULL;
    int i = 0;
    int k = 0;
    int col = 0;
    int row = 0;
    FT_Bitmap * bitmap = NULL;
    const int border = 4;    
    int realWidth = 0;
    int realHeight = 0;
    int halfBorder = border / 2;
    gft_symbol_t code;
    gft_symbol_t symbols[4096] = {0}; /* this size must be controlled <<<--- FIX THIS */
    
    /* convert symbol set to utf32 */
    gft_utf8_to_utf32(symbolSet, symbols);
    
    /* create font */
    font = calloc(sizeof(*font), 1);
	font->size = size;
    font->refCounter = 1;    
    gft_utf32_strlen(symbols, &font->glyphCount);
    font->glyphs = calloc(font->glyphCount, sizeof(*font->glyphs));
    font->options = options;   
    
    /* init freetype */
	if(FT_Init_FreeType(&ftLibrary)) {
        goto error_cleanup;
    }
    
    /* try to load file */
	if(FT_New_Face(ftLibrary, filename, 0, &face)) {
        goto error_cleanup;
    }
    
    /* set size and charmap */
	if(FT_Set_Pixel_Sizes(face, 0, font->size)) {
        goto error_cleanup;
    }
	if(FT_Select_Charmap(face, FT_ENCODING_UNICODE)) {
        goto error_cleanup;
    }
	
	for (i = 0; i < font->glyphCount; i++) {        
        code = symbols[i];
        
		if(FT_Load_Glyph(face, FT_Get_Char_Index(face, code), FT_LOAD_DEFAULT)) {
            goto error_cleanup;
        }
		if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
            goto error_cleanup;
        }

		bitmap = &face->glyph->bitmap;

		realWidth = bitmap->width + border;
		realHeight = bitmap->rows + border;
		
        font->glyphs[i].code = code;
		font->glyphs[i].advanceX = face->glyph->advance.x >> 6;
		font->glyphs[i].advanceY = face->glyph->advance.y >> 6;
		font->glyphs[i].bitmapTop = face->glyph->bitmap_top;
		font->glyphs[i].bitmapLeft = face->glyph->bitmap_left;
		font->glyphs[i].pixels = malloc(realWidth * realHeight * sizeof(*font->glyphs[i].pixels));
		font->glyphs[i].width = realWidth;
		font->glyphs[i].height = realHeight;
		
        /* clear glyph pixels into (a = 0; rgb = 255) */
		for (k = 0; k < realWidth * realHeight; ++k) {
			font->glyphs[i].pixels[k].a = 0;
            font->glyphs[i].pixels[k].r = 255;
            font->glyphs[i].pixels[k].g = 255;
            font->glyphs[i].pixels[k].b = 255;
		}
        
        /* copy glyph pixels from freetype data with border, to remove seams due to bilinear filtration on GPU */
		for (row = halfBorder; row < realHeight; ++row) {
			for (col = halfBorder; col < realWidth; ++col) {
				int r = row - halfBorder;
				int c = col - halfBorder;
				if (r < bitmap->rows && c < bitmap->width) {
					font->glyphs[i].pixels[row * realWidth + col].a = bitmap->buffer[r * bitmap->width + c];
				}
			}
		}
	}
    
	FT_Done_Face(face);
	FT_Done_FreeType(ftLibrary);    
    
	gft_font_pack(font);   
 
    *fontPtr = font;
    
    /* everything succeeded */
	return GFT_NO_ERROR;
    
/* something went wrong - clean up*/
error_cleanup:
    if(font) {
        free(font->pixels);
        free(font);
    }
    if(face) {
        FT_Done_Face(face);    
    }
    if(ftLibrary) {
        FT_Done_FreeType(ftLibrary);
    }
    return GFT_FREETYPE_ERROR;
}

/*
====================================
gft_font_load
====================================
*/
gft_error_t gft_font_load(const char * filename, gft_font_t ** fontPtr) {
    int i = 0;
    int k = 0;
    int byteCount = 0;
    gft_header_t header = { {0} };
    gft_font_t * font = NULL;
    FILE * file = fopen(filename, "rb");
    
    if(!file) {
        return GFT_UNABLE_TO_LOAD_FONT;
    }

    /* read and check signature - it must be GFT_SIGNATURE */
    fread(header.signature, 4, 1, file);    
    if(strcmp(header.signature, GFT_SIGNATURE) != 0) {        
        return GFT_UNKNOWN_FORMAT;
    }
    
    /* read version */
    fread(&header.version, sizeof(header.version), 1, file);
    
    /* read glyph count */
    fread(&header.glyphCount, sizeof(header.glyphCount), 1, file);
    
    /* read atlas metrics */
    fread(&header.atlasWidth, sizeof(header.atlasWidth), 1, file);
    fread(&header.atlasHeight, sizeof(header.atlasHeight), 1, file);
    
    /* read size */
    fread(&header.fontSize, sizeof(header.fontSize), 1, file); 
    
    /* create empty font */
    font = calloc(sizeof(*font), 1);    
    
    if(!font) {
        fclose(file);
        return GFT_NOT_ENOUGH_MEMORY;
    }
    
    font->refCounter = 1;
    font->atlasWidth = header.atlasWidth;
    font->atlasHeight = header.atlasHeight;
    font->size = header.fontSize;
    font->glyphCount = header.glyphCount;
    font->glyphs = calloc(font->glyphCount, sizeof(*font->glyphs));
    
    /* read glyphs */
    for(i = 0; i < header.glyphCount; ++i) {
        fread(&font->glyphs[i].code, sizeof(font->glyphs[i].code), 1, file);
        fread(&font->glyphs[i].advanceX, sizeof(font->glyphs[i].advanceX), 1, file);
        fread(&font->glyphs[i].advanceY, sizeof(font->glyphs[i].advanceY), 1, file);
        fread(&font->glyphs[i].bitmapLeft, sizeof(font->glyphs[i].bitmapLeft), 1, file);
        fread(&font->glyphs[i].bitmapTop, sizeof(font->glyphs[i].bitmapTop), 1, file);
        fread(&font->glyphs[i].width, sizeof(font->glyphs[i].width), 1, file);
        fread(&font->glyphs[i].height, sizeof(font->glyphs[i].height), 1, file);
        for(k = 0; k < 4; ++k) {
            fread(&font->glyphs[i].texCoords[k], sizeof(font->glyphs[i].texCoords[k]), 1, file);
        }
    }
    
    /* read atlas bitmap */
    byteCount = font->atlasWidth * font->atlasHeight * sizeof(gft_rgba_pixel_t);
    font->pixels = malloc(byteCount);
    
    if(!font->pixels) {
        fclose(file);
        return GFT_NOT_ENOUGH_MEMORY;
    }
    
    if(fread(font->pixels, 1, byteCount, file) != byteCount) {
        fclose(file);
        return GFT_UNEXPECTED_END_OF_FILE;
    }
    
    fclose(file);
    
    *fontPtr = font;
    
    return GFT_NO_ERROR;
}

/* 
===============
gft_font_save
===============
*/
gft_error_t gft_font_save(gft_font_t * font, const char * filename) {
    int i = 0;
    int k = 0;
    gft_header_t header = { {0} };
    FILE * file = fopen(filename, "wb");
    
    if(!file) {
        return GFT_UNABLE_TO_SAVE_FONT;
    }
    
    /* fill header */
    strcpy(&header.signature[0], "GFT0");
    header.glyphCount = font->glyphCount;
    header.atlasHeight = font->atlasHeight;
    header.atlasWidth = font->atlasWidth;
    header.version = 1; 
    header.fontSize = font->size;
    
    /* write header */
    fwrite(&header, sizeof(header), 1, file);
    
    /* write glyphs */
    for(i = 0; i < header.glyphCount; ++i) {
        fwrite(&font->glyphs[i].code, sizeof(font->glyphs[i].code), 1, file);
        fwrite(&font->glyphs[i].advanceX, sizeof(font->glyphs[i].advanceX), 1, file);
        fwrite(&font->glyphs[i].advanceY, sizeof(font->glyphs[i].advanceY), 1, file);
        fwrite(&font->glyphs[i].bitmapLeft, sizeof(font->glyphs[i].bitmapLeft), 1, file);
        fwrite(&font->glyphs[i].bitmapTop, sizeof(font->glyphs[i].bitmapTop), 1, file);
        fwrite(&font->glyphs[i].width, sizeof(font->glyphs[i].width), 1, file);
        fwrite(&font->glyphs[i].height, sizeof(font->glyphs[i].height), 1, file);
        for(k = 0; k < 4; ++k) {
            fwrite(&font->glyphs[i].texCoords[k], sizeof(font->glyphs[i].texCoords[k]), 1, file);
        }
    }
    
    /* write bitmap atlas */
    fwrite(font->pixels, font->atlasWidth * font->atlasHeight * sizeof(*font->pixels), 1, file);
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_free
====================================
*/
gft_error_t gft_font_add_reference(gft_font_t * font) {
    if(!font) {
        return GFT_BAD_FONT;
    }
    
    ++font->refCounter;
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_free
====================================
*/
gft_error_t gft_font_free(gft_font_t * font) {
    if(!font)  {
        return GFT_BAD_FONT;
    }
    
    --font->refCounter;
    if(font->refCounter == 0) {
        free(font->glyphs);
        free(font->pixels);
        free(font);
    }
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_font_get_width
====================================
*/
gft_error_t gft_font_get_width(gft_font_t * font, int * width) {
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!width) {
        return GFT_BAD_PARAMETER;
    }
    
    *width = font->atlasWidth;
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_font_get_height
====================================
*/
gft_error_t gft_font_get_height(gft_font_t * font, int * height) {
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!height) {
        return GFT_BAD_PARAMETER;
    }
    
    *height = font->atlasHeight;
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_font_get_atlas_size
====================================
*/
gft_error_t gft_font_get_atlas_size(gft_font_t * font, int * size) {
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!size) {
        return GFT_BAD_PARAMETER;
    }
    
    *size = font->atlasWidth * font->atlasHeight * sizeof(*font->pixels);
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_glyph_find
    - finds glyph by unicode symbol
====================================
*/
static gft_glyph_t * gft_glyph_find(gft_font_t * font, gft_symbol_t symbol) {
    int i = 0;
    
    /* linear search for now (shitty and slow) - replace this with binary tree <<<--- FIX THIS */
    for(i = 0; i < font->glyphCount; ++i) {
        if(font->glyphs[i].code == symbol) {
            return &font->glyphs[i];
        }
    }
    
    return NULL;
}

/*
====================================
gft_font_get_atlas_pixels
====================================
*/
gft_error_t gft_font_get_atlas_pixels(gft_font_t * font, gft_rgba_pixel_t ** pixels) {
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!pixels) {
        return GFT_BAD_PARAMETER;
    }
    
    *pixels = font->pixels;
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_glyph_get_caret_offset_x
====================================
*/
gft_error_t gft_glyph_get_caret_offset_x(gft_font_t * font, gft_symbol_t symbol, int * offset) {
    gft_glyph_t * glyph;
    
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!offset) {
        return GFT_BAD_PARAMETER;
    }
    
    glyph = gft_glyph_find(font, symbol);
    
    if(glyph) {
        *offset = glyph->bitmapLeft;
    } else {
        return GFT_BAD_CODE;
    }
                            
    return GFT_NO_ERROR;
}

/*
====================================
gft_glyph_get_caret_offset_y
====================================
*/
gft_error_t gft_glyph_get_caret_offset_y(gft_font_t * font, gft_symbol_t symbol, int * offset) {
    gft_glyph_t * glyph;
    
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!offset) {
        return GFT_BAD_PARAMETER;
    }
    
    glyph = gft_glyph_find(font, symbol);
    
    if(glyph) {
        *offset = font->size - glyph->bitmapTop;
    } else {
        return GFT_BAD_CODE;
    }
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_glyph_get_caret_step_x
====================================
*/
gft_error_t gft_glyph_get_caret_step_x(gft_font_t * font, gft_symbol_t symbol, int * step) {
    gft_glyph_t * glyph;
    
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!step) {
        return GFT_BAD_PARAMETER;
    }
    
    glyph = gft_glyph_find(font, symbol);
    
    if(glyph) {
        *step = glyph->advanceX;
    } else {
        return GFT_BAD_CODE;
    }
    
    return GFT_NO_ERROR;
}

/*
====================================
gft_glyph_get_caret_step_y
====================================
*/
gft_error_t gft_glyph_get_caret_step_y(gft_font_t * font, gft_symbol_t symbol, int * step) {
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!step) {
        return GFT_BAD_PARAMETER;
    }
    
    *step = font->size;
    
    return GFT_NO_ERROR;    
}

/*
====================================
gft_glyph_get_texcoords
====================================
*/
gft_error_t gft_glyph_get_texcoords(gft_font_t * font, gft_symbol_t symbol, gft_texcoord_t * texcoords) {
    int i;
    gft_glyph_t * glyph;
    
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!texcoords) {
        return GFT_BAD_PARAMETER;
    }
   
    glyph = gft_glyph_find(font, symbol);
    
    if(glyph) {
        for(i = 0; i < 4; ++i) {
            texcoords[i] = glyph->texCoords[i];
        }
    } else {
        return GFT_BAD_CODE;
    }
    
    return GFT_NO_ERROR;    
}

/*
====================================
gft_glyph_get_width
====================================
*/
gft_error_t gft_glyph_get_width(gft_font_t * font, gft_symbol_t symbol, int * width) {
    gft_glyph_t * glyph;
     
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!width) {
        return GFT_BAD_PARAMETER;
    }
    
    *width = 0;
    
    glyph = gft_glyph_find(font, symbol);
    
    if(glyph) {
        *width = glyph->width;
    } else {
        return GFT_BAD_CODE;
    }
    
    return GFT_NO_ERROR;    
}

/*
====================================
gft_glyph_get_height
====================================
*/
gft_error_t gft_glyph_get_height(gft_font_t * font, gft_symbol_t symbol, int * height) {
    gft_glyph_t * glyph;
    
    if(!font) {
        return GFT_BAD_FONT;
    }
    if(!height) {
        return GFT_BAD_PARAMETER;
    }
    
    *height = 0;
    
    glyph = gft_glyph_find(font, symbol);
    
    if(glyph) {
        *height = glyph->height;
    } else {
        return GFT_BAD_CODE;
    }
    
    return GFT_NO_ERROR;  
}