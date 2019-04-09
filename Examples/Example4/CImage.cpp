#pragma warning(disable: 4996)

#include "CImage.h"
#include "Validation.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#define repeat(COUNT) for(int i = 0; i < COUNT; ++i)

static bool file_exists(const char *fileName) {
#ifdef ANDROID
    return true;
#endif

    std::ifstream infile(fileName);
    return infile.good();
}

//-------------------RGBA----------------------
RGBA::RGBA() : R(0), G(0), B(0), A(255) {}
RGBA::RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) {}
void RGBA::set(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { R=r; G=g; B=b; A=a;}

RGBA RGBA::lerp(RGBA c, float f) { 
    return RGBA( (uint8_t)(f*(c.R-R)+R),
                 (uint8_t)(f*(c.G-G)+G),
                 (uint8_t)(f*(c.B-B)+B),
                 (uint8_t)(f*(c.A-A)+A) );
}
//---------------------------------------------

//------------------CImage---------------------

// move symantics
void CImage::move_ctor(CImage& other) {
    buf = other.buf;
    width = other.width;
    height = other.height;
//    mag_filter = other.mag_filter;
    other.buf = nullptr;
}

// move assignment operator
CImage& CImage::operator=(CImage&& other) {  // not used?
    if(this == &other) return *this;
    if(buf) free(buf);
    move_ctor(other);
    return *this;
}

void CImage::SetSize(const int w, const int h) {
    width  = w;
    height = h;
    if (!!buf) free(buf);
    buf = (RGBA*) malloc((uint)w * (uint)h * sizeof(RGBA));
}
/*
RGBA& CImage::Pixel(int x, int y) {
    // texture CLAMP vs WRAP 
    if(wrapMode_U == REPEAT) x = WRAP_INT(x, width);
    if(wrapMode_V == REPEAT) y = WRAP_INT(y, height);
    x = CLAMP(x, 0, width -1);
    y = CLAMP(y, 0, height-1);
    return buf[x + ((height-y-1) * width)];
}

RGBA CImage::UV(float u, float v) {  // Nearest filtering. TODO: Biinear / Trilinear / Anisotropic
    //return Pixel( (int)(u * width), (int)(v * height));
    if(mag_filter == NEAREST) return Pixel((int)(u * width), (int)(v * height));
    else                      return UV_bilinear(u, v);
}

RGBA CImage::UV_bilinear(float u, float v) {  // untested, and ignores gamma
    float fx = u * width;
    float fy = v * height;
    float ix = floor(fx);
    float iy = floor(fy);
    fx -= ix;
    fy -= iy;
    int x = (int)ix;
    int y = (int)iy;
    RGBA a = Pixel(x + 0, y + 0);
    RGBA b = Pixel(x + 1, y + 0);
    RGBA c = Pixel(x + 0, y + 1);
    RGBA d = Pixel(x + 1, y + 1);
    RGBA ab  = a.lerp(b, fx);
    RGBA cd  = c.lerp(d, fx);
    return ab.lerp(cd, fy);
}
*/
void CImage::Clear() {
    memset(buf, 0, (uint)width * (uint)height * sizeof(RGBA));
}

void CImage::Clear(RGBA& color) {
    repeat(width * height) buf[i] = color;
}

bool CImage::Load(const char* filename, bool flip) {
    if(!file_exists(filename)) { printf("CImage: File not found: %s\n", filename);  return false; }
    int w, h, n;
    stbi_set_flip_vertically_on_load(flip);
    char* tmp=(char*)stbi_load(filename, &w, &h, &n, sizeof(RGBA));
    if(tmp) {
        printf("Load image: %s (%dx%d)\n", filename, w, h);
        SetSize(w, h);
        memcpy(buf,tmp, (uint)w * (uint)h * sizeof(RGBA));
        stbi_image_free(tmp);
    } else printf("CImage: Failed to load texture.");
    return !!tmp;
}
/*
bool CImage::Load_from_mem(const void* addr, int len, bool flip) {
    //int w, h, n;
    int w = 0, h = 0, n = 0;
    stbi_set_flip_vertically_on_load(flip);
    char* tmp=(char*)stbi_load_from_memory((const stbi_uc*)addr, len, &w, &h, &n, sizeof(RGBA));
    if(tmp) {
        SetSize(w, h);
        memcpy(buf, tmp, (uint)w * (uint)h * sizeof(RGBA));
        stbi_image_free(tmp);
    } else printf("CImage: Failed to load texture.");
    return !!tmp;
}
*/
/*
void CImage::Save(const char* filename) {
    printf("Saving file: %s\n", filename);
    const char* ext = strrchr(filename, '.');
    //int stride = width * height * sizeof(RGBA);
    if(!strcmp(ext, ".jpg")) stbi_write_jpg(filename, width, height, sizeof(RGBA), buf, 90);
    if(!strcmp(ext, ".png")) stbi_write_png(filename, width, height, sizeof(RGBA), buf, 0);
    //stbi_write_png(filename, width, height, sizeof(RGBA), buf, 0);
}
*/
void CImage::FillPattern(bool gradient, int checkers) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            RGBA* pix = &buf[x + y * width];
            *pix = { 0, 0, 0, 255 };
            if(gradient) *pix = { (uint8_t)x, (uint8_t)y, 0, 255 };
            if(checkers) if((x/(width/checkers))%2 == (y/(height/checkers))%2) pix->B = 128;
        }
    }
}


//-------------------------------------------------------------
