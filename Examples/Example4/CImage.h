#ifndef CIMAGE_H
#define CIMAGE_H

#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <fstream> // for file_exists

typedef uint32_t uint;


#ifdef WIN32
    static void* align_alloc(size_t alignment, size_t size) {return _aligned_malloc(size, alignment);}
    static void  align_free(void* memblock) {_aligned_free(memblock);}
#elif LINUX
    static void* align_alloc(size_t alignment, size_t size) {return aligned_alloc(alignment, size)}
    static void  align_free(void* memblock) {free(memblock);}
#elif ANDROID
    static void* align_alloc(size_t alignment, size_t size) {return malloc(size);}
    static void  align_free(void* memblock) {free(memblock);}
#endif

//--------------------------------------------------
// 8 bit per channel sRGBA (gamma)
struct RGBA { 
    uint8_t R; uint8_t G; uint8_t B; uint8_t A; 
    RGBA();
    RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
    void set(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);
    RGBA lerp(RGBA c, float f);
};
//--------------------------------------------------

//---------------------CImage-----------------------
class CImage {
public:
    int width  = 0;
    int height = 0;
    RGBA* buf  = nullptr;
	
	//MagFilter mag_filter = NEAREST;
    //WrapMode  wrap_mode_U = CLAMP;
    //WrapMode  wrap_mode_V = CLAMP;
	
    //RGBA UV_bilinear(float u, float v);

    //-- disable copy --
    CImage(const CImage&);
    CImage& operator=(const CImage&);
    //------------------
    void move_ctor(CImage& other);
public:
    CImage(){}
    CImage(const char* filename){ Load(filename); }
    CImage(const int width, const int height){ SetSize(width, height); }
    ~CImage(){ if(!!buf) align_free(buf); buf=nullptr; }

    //-- Enable move --
    CImage(CImage&& other) { move_ctor(other); }
    CImage& operator=(CImage&& other);
    //-----------------	

    void SetSize(const int width, const int height);
    int Width()  { return width;  }
    int Height() { return height; }
    //RGBA& Pixel(int x, int y);       // return pixel at x,y coordinate (range 0->w by 0->h ) Read / Write
    //RGBA UV(float u, float v);       // return color at u,v coordinate (range 0->1 by 0->1 ) Read-only
    //MagFilter& magFilter = mag_filter;
    //WrapMode& wrapMode_U   = wrap_mode_U;
    //WrapMode& wrapMode_V   = wrap_mode_V;

    void Clear();
    void Clear(RGBA& color);
    bool Load(const char* filename, bool flip = false);                // Load from file (flip = vertically)
    //bool Load_from_mem(const void* addr, int len, bool flip = false);  // Load from memory
    //void Save(const char* filename);                                   // Save to file

    void FillPattern(bool gradient = true, int checkers = 64);  // fill texture with test-pattern.
};
//--------------------------------------------------

#endif
