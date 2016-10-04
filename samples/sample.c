#include <stdio.h>
#include <stdlib.h>

#ifdef _TEST

#include <windows.h>
/* include gl headers */
#include <gl/gl.h>
#include <gl/glu.h>

#include "../src/gamefonttool.h"



HGLRC glContext;
HDC deviceContext;
HWND window;
char running;

LRESULT CALLBACK window_proc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
        running = 0;
        return 1;
		break;
	}
	case WM_CLOSE: {
        running = 0;
		return 1;
		break;
	}
	}
	return DefWindowProc(wnd, msg, wParam, lParam);
}

/* 
===============
init_opengl
    - creates window and inits opengl
===============
*/
void init_opengl() {
    PIXELFORMATDESCRIPTOR pfd = { 0 };    
    WNDCLASSEX wcx = { 0 };
    DWORD style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    RECT wRect = { 0 };
    const char * className = "GFTTest";
    
    wcx.cbSize = sizeof(wcx);
    wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcx.hInstance = GetModuleHandle(NULL);
    wcx.lpfnWndProc = window_proc;
    wcx.lpszClassName = className;
    wcx.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClassEx(&wcx);
    
    wRect.right = 800;
    wRect.bottom = 600;

	AdjustWindowRect(&wRect, style, 0);
	window = CreateWindow(className, className, style, 0, 0, wRect.right - wRect.left, wRect.bottom - wRect.top, 0, 0, wcx.hInstance, 0);
	ShowWindow(window, SW_SHOW);
	UpdateWindow(window);
	SetActiveWindow(window);
	SetForegroundWindow(window);

	deviceContext = GetDC(window);	
    
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SWAP_COPY | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

	SetPixelFormat(deviceContext, ChoosePixelFormat(deviceContext, &pfd), &pfd);
    
	glContext = wglCreateContext(deviceContext);
	wglMakeCurrent(deviceContext, glContext);
	glEnable(GL_TEXTURE_2D);
    
    running = 1;
}

/* 
===============
atlas_to_texture
    - loads font atlas to GPU as texture and returns texture identifier
===============
*/
GLuint atlas_to_texture(gft_font_t * font) {
    GLuint texture;
    int width;
    int height;
    gft_rgba_pixel_t * pixels;
    
    gft_font_get_width(font, &width);
    gft_font_get_height(font, &height);
    gft_font_get_atlas_pixels(font, &pixels);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
    return texture;
}

/* 
===============
render_string
    - renders string using glyphs info from a font
===============
*/
void render_string(gft_font_t * font, int originX, int originY, const char * str) {
    int i = 0;
    int len = 0;
    int caretX = originX, caretY = originY;
    gft_symbol_t symbol = 0;
    int offsetX, offsetY;
    int stepX, stepY;
    int x, y;
    int width, height;
    gft_texcoord_t texCoords[4];
    gft_symbol_t utf32str[2048];
        
    /* convert string to utf32 */
    gft_utf8_to_utf32(str, utf32str, 2048);
    
    /* compute length */
    gft_utf32_strlen(utf32str, &len);
    
    /* render */
    glBegin(GL_QUADS);
    
    for(i = 0; i < len; ++i) {
        symbol = utf32str[i];
        
        gft_glyph_get_caret_offset_x(font, symbol, &offsetX);
        gft_glyph_get_caret_offset_y(font, symbol, &offsetY);
        gft_glyph_get_caret_step_x(font, symbol, &stepX);
        gft_glyph_get_caret_step_y(font, symbol, &stepY);
        gft_glyph_get_width(font, symbol, &width);
        gft_glyph_get_height(font, symbol, &height);
        gft_glyph_get_texcoords(font, symbol, texCoords);
            
        x = caretX + offsetX;
        y = caretY + offsetY; 
        
        /* feed GPU */
        glTexCoord2f(texCoords[0].x, texCoords[0].y);
        glVertex2f(x, y);
        
        glTexCoord2f(texCoords[1].x, texCoords[1].y);      
        glVertex2f(x + width, y); 
        
        glTexCoord2f(texCoords[2].x, texCoords[2].y);
        glVertex2f(x + width, y + height);
        
        glTexCoord2f(texCoords[3].x, texCoords[3].y);
        glVertex2f(x, y + height);  
        
        /* move caret */
        caretX += stepX;
        
        if(symbol == '\n') {
            caretY += stepY;
            caretX = originX;
        }
    }  
    
    glEnd();
}

/* 
===============
render_atlas
===============
*/
void render_atlas(gft_font_t * font, int originX, int originY) {
    int width, height;
    
    gft_font_get_width(font, &width);
    gft_font_get_height(font, &height);
    
    glBegin(GL_QUADS);
    
    glTexCoord2f(0, 0);
    glVertex2f(originX, originY);
    
    glTexCoord2f(0, 1);
    glVertex2f(originX, originY + height);
    
    glTexCoord2f(1, 1);
    glVertex2f(originX + width, originY + height);
    
    glTexCoord2f(1, 0);
    glVertex2f(originX + width, originY);   

    glEnd();
}

int main(int argc, char **argv) {
    gft_font_t * font = NULL;
    GLuint texture;
    
    const char * symbolSet = 
        "1234567890-=!@#$%^&*()_+\\|/><,.?~`';: "
        "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЬЪЫЭЮЯ"
        "абвгдеёжзийклмнопрстуфхцчшщьъыэюя"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    const char * gft_text = 
        "Съешь еще этих мягких французских булок да выпей чаю.\n"
        "The quick brown fox jumps over a lazy dog. 1234567890. !@#$%^&*()_+<>?\n"
        "New line in the same string.\n"
        "Below you can see glyph atlas\n";
        
    init_opengl();

#if 0    
    gft_font_create("data/test.ttf", 20, GFT_DEFAULT, symbolSet, &font);
#endif

    /* try to load font from cache */
    if(gft_font_load("test.gft", &font)) {
        /* cached font does not exists, so create it */
        if(gft_font_create("data/test.ttf", 20, GFT_DEFAULT, symbolSet, &font)) {
            printf("failed to load test.ttf\n");
        } else {
            /* save font to cache */
            gft_font_save(font, "test.gft");
            printf("created from test.ttf");
        }
    } else {
        printf("loaded from cache");
    }


    texture = atlas_to_texture(font);
    
    while(running) {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        
        glClearColor(0.0, 0.5, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, 800, 600, 0);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
                
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
        render_string(font, 10, 10, gft_text);
        render_atlas(font, 10, 100);

        SwapBuffers(deviceContext);
    }
    
    gft_font_free(font);
    
	return 0;
}

#endif