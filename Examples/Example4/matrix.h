/*
*--------------------------------------------------------------------------
* Copyright (c) 2016-2017 Rene Lindsay
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Author: Rene Lindsay <rjklindsay@gmail.com>
*
*--------------------------------------------------------------------------
*
* This unit provides basic types for 3d graphics:
*
* vec2    : 2 element vector of floats
* vec3    : 3 element vector of floats
* vec4    : 4 element vector of floats (for homogenous computations)
* mat4    : 4x4 matrix in column major order
* euler   : represents an orientation as (yaw, pitch, roll)
* quat    : quaternion (WIP)
*
* The types were named to match equivalent types in GLSL.
*
*--------------------------------------------------------------------------
*/

#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

//#include <intrin.h>    //for avx on windows
//include <x86intrin.h>  //for avx on linux

const float pi = 3.141592653f;
const float toDeg = 180.0f/pi;
const float toRad = pi/180.0f;
typedef unsigned int uint;

#define Y_UP    // remove this for Z_UP

#define FASTER  // but less accurate

static float invSqrt(float x) {   // fast inverse sqrt
    //assert(x>0);
    float xhalf = 0.5f * x;
    int i = *(int*)&x;
    i = 0x5f3759df - (i >> 1);
    x = *(float*)&i;           // initial guess
    x = x*(1.5f - xhalf*x*x);  // refine with newtons method
    //x = x*(1.5f - xhalf*x*x);  // refine again for more accuracy (optional)
    return x;
}
/*
inline float sqrt_fast(const float x) {
    static union {
        int i;
        float x;
    } u;
    u.x = x;
    u.i = (1<<29) + (u.i >> 1) - (1<<22); 
    u.x = 0.5f * (u.x + x/u.x);
    return u.x;
}
*/

//----------------vec2---------------
struct vec2i {
    int x, y;
    vec2i() {}
    vec2i(int x, int y) : x(x), y(y) {}
    //vec2i(float x, float y) : x((int)x), y((int)y) {}
};

struct vec2 {
    float x, y;
    vec2() {}
    vec2(float* f) { x=f[0]; y=f[1]; }
    vec2(float x, float y) : x(x), y(y) {}
    vec2& operator += (const vec2& v) { x+=v.x;  y+=v.y;  return *this; }
    vec2& operator -= (const vec2& v) { x-=v.x;  y-=v.y;  return *this; }
    vec2& operator *= (const vec2& v) { x*=v.x;  y*=v.y;  return *this; }
    vec2& operator /= (const vec2& v) { x/=v.x;  y/=v.y;  return *this; }
    vec2 operator + (const vec2& v) const { return {x+v.x, y+v.y}; }
    vec2 operator - (const vec2& v) const { return {x-v.x, y-v.y}; }
    vec2 operator * (const vec2& v) const { return {x*v.x, y*v.y}; }
    vec2 operator / (const vec2& v) const { return {x/v.x, y/v.y}; }
    vec2 operator * (const float f) const { return {x*f, y*f}; }
    vec2 operator / (const float f) const { return {x/f, y/f}; }
    operator vec2i () const { return {int(x), int(y)}; }
    vec2 lerp(vec2 v, float ratio) { return *this + ((v-*this)*ratio); }
};
//-----------------------------------

//----------------vec3---------------
struct vec3 {
    float x, y, z;
    vec3() {}
    vec3(float* f) { x=f[0]; y=f[1]; z=f[2]; }
    vec3(float _x, float _y, float _z) { x = _x;  y = _y; z = _z; }
    void set(float _x, float _y, float _z) { x = _x;  y = _y; z = _z; }
    vec3& operator += (const vec3& v) { x+=v.x;  y+=v.y;  z+=v.z;  return *this; }
    vec3& operator -= (const vec3& v) { x-=v.x;  y-=v.y;  z-=v.z;  return *this; }
    vec3& operator *= (const vec3& v) { x*=v.x;  y*=v.y;  z*=v.z;  return *this; }
    vec3& operator /= (const vec3& v) { x/=v.x;  y/=v.y;  z/=v.z;  return *this; }
    vec3& operator *= (const float f) { x*=f;   y*=f;   z*=f;   return *this; }
    vec3& operator /= (const float f) { x/=f;   y/=f;   z/=f;   return *this; }
    vec3& operator =  (const vec3& v) { x=v.x;  y=v.y;  z=v.z;  return *this; }
    vec3 operator + (const vec3& v) const { return {x+v.x, y+v.y, z+v.z}; }
    vec3 operator - (const vec3& v) const { return {x-v.x, y-v.y, z-v.z}; }
    vec3 operator * (const vec3& v) const { return {x*v.x, y*v.y, z*v.z}; }
    vec3 operator / (const vec3& v) const { return {x/v.x, y/v.y, z/v.z}; }
    vec3 operator * (const float f) const { return {x*f, y*f, z*f}; }
    vec3 operator / (const float f) const { float rf=1.f/f;  return *this*rf; }
    vec3 operator - () const { return {-x,-y,-z}; }
    //bool operator ==(const vec3& v) const { return (x==v.x && y==v.y && z==v.z); }
    //vec3 operator ^ (const vec3& v) const { return{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }  // cross product
    //float operator |(const vec3& v) const { return x*v.x + y*v.y + z*v.z; }                     // dot product
    vec3  cross(const vec3& v) const { return{ y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x }; }  // cross product
    float dot  (const vec3& v) const { return x*v.x + y*v.y + z*v.z; }                     // dot product

    void normalize_fast() { *this *= invSqrt(x*x + y*y + z*z); }
#ifdef FASTER
    void normalize() { *this *= invSqrt(x*x + y*y + z*z); }
#else
    void normalize() { *this *= 1.f / length(); }
#endif
    float length_squared() const { return (x*x + y*y + z*z); }
    float length() const { return sqrtf(x*x + y*y + z*z); }
    vec3 lerp(vec3 v, float ratio) { return *this + ((v-*this)*ratio); }
    vec3 reflect(const vec3& norm) const { return *this - (norm * dot(norm)) * 2.f; }  // reflect vector around suface normal
    //TODO: refract...

    inline void _rotate(const float ang, float &X, float &Y) {
        const float a = -ang * toRad;
        const float cs = cosf(a);
        const float sn = sinf(a);
        const float Z = X;
        X = Z*cs - Y*sn;
        Y = Y*cs + Z*sn;
    }

    void RotateX(const float angle) { _rotate( angle, z, y); }  // rotate vector around X-axis (clockwise)
    void RotateY(const float angle) { _rotate( angle, x, z); }  // rotate vector around Y-axis (clockwise)
    void RotateZ(const float angle) { _rotate(-angle, x, y); }  // rotate vector around Z-axis (clockwise)
    float AngleX(){ return  (float)atan2(z, y) * toDeg; }       // get vector's angle around X-axis  (PITCH  : positive is up       )  // y-axis=0
    float AngleY(){ return  (float)atan2(x, z) * toDeg; }       // get vector's angle around Y-axis  (ROLL   : clockwise from top   )  // z-axis=0
    float AngleZ(){ return -(float)atan2(x, y) * toDeg; }       // get vector's angle around Z-axis  (HEADING: clockwise from north )  // x=axis-0

    void Print() const { printf("(%+6.3f,%+6.3f,%+6.3f)\n", x ,y ,z); }
};

static vec3 normalize(const vec3& v) { return v / v.length(); }
//-----------------------------------

//----------------vec4---------------
struct alignas(16) vec4 {
    float x, y, z, w;
    vec4() {}
    vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    vec4(const vec3& v3) : x(v3.x), y(v3.y), z(v3.z), w(1) {}
    vec4(const vec3& v3, float w) : x(v3.x), y(v3.y), z(v3.z), w(w) {}
    operator vec3 () const { return vec3(x/w, y/w, z/w); }
};
//-----------------------------------

//--------------euler----------------
struct euler {
    float yaw, pitch, roll;
    euler() {}

#ifdef Y_UP
    euler(vec3 fwd) {         //Get a vector's yaw and pitch. (no roll)
        vec3 v=fwd;
        yaw =v.AngleY();  v.RotateY(-yaw);   // get and eliminate yaw
        pitch=v.AngleX();                    // get pitch
        roll =0;                             // no roll
    }

       euler(vec3 fwd, vec3 up) {    // Convert up and forward vectors to yaw/pitch/roll
        yaw  =180+fwd.AngleY(); fwd.RotateY(-yaw);   up.RotateY(-yaw);
        pitch= 90+fwd.AngleX(); fwd.RotateX(-pitch); up.RotateX(-pitch);
        roll =up.AngleZ();

        if(yaw   > 180) yaw  -=360;
        if(pitch > 180) pitch-=360;
        if(roll  > 180) roll -=360;
    }
#else
    euler(vec3 fwd) {         //Get a vector's yaw and pitch. (no roll)
        vec3 v=fwd;
        yaw =v.AngleZ();  v.RotateZ(-yaw);   // get and eliminate yaw
        pitch=v.AngleX();                    // get pitch
        roll =0;                             // no roll
    }

    euler(vec3 fwd, vec3 up) {    // Convert up and forward vectors to yaw/pitch/roll
        yaw  = fwd.AngleZ(); fwd.RotateZ(-yaw);   up.RotateZ(-yaw);
        pitch= fwd.AngleX(); fwd.RotateX(-pitch); up.RotateX(-pitch);
        roll =  up.AngleY();
    }
#endif

    void Print(){ printf("Roll=%f  Pitch=%f  Yaw=%f\n", roll, pitch, yaw); }
    //void Print(){ printf("Yaw=%f  Pitch=%f  Roll=%f\n", yaw, pitch, roll); }
};
//-----------------------------------


struct quat {  // TODO
    float w, x, y, z;
    quat() : w(1), x(0), y(0), z(0) {}  // identity
    quat(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}
};



//--------------matrix---------------
class alignas(16) mat4 {
public:
    union{
        float m[16];
        struct{
            float m00, m10, m20, m30;  // x axis       ,0
            float m01, m11, m21, m31;  // y axis       ,0
            float m02, m12, m22, m32;  // z axis       ,0
            float m03, m13, m23, m33;  // translation  ,ScaleInverse
        };
        struct{
            vec3 xAxis;    float _0;
            vec3 yAxis;    float _1;
            vec3 zAxis;    float _2;
            vec3 position; float _3;
        };
        struct{
            vec4 xAxis4;
            vec4 yAxis4;
            vec4 zAxis4;
            vec4 position4;
        };
    };

    void Set(const float* f){ memcpy(m,f,sizeof(m)); }

    inline void Clear() { 
        const float Identity4x4[16] {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}; // Reset this matrix to Identity.
        Set(Identity4x4);
    }

    inline void ClearRot() { vec3 pos = position; Clear(); position = pos; }

    float& operator [] (uint i) { return m[i]; }                         // array operator
    mat4& operator = (const mat4& other){ Set(other.m); return *this; }  // copy assignment operator
    mat4(float* f) { Set(&(*f)); }                                       // convert float[16] to mat4
    mat4(quat q) { from_quat(q);}                                        // convert quaternion to mat4
    mat4(){ Clear(); }                                                   // default constructor

#ifdef Y_UP
    euler Euler() { return euler(-zAxis, yAxis); }                     // Get Euler angles from matrix (y-up)

    void SetRotation(float roll, float pitch, float yaw) {             // Apply euler rotations.
        vec3 pos = position;
        Clear();
        RotateY(yaw);
        RotateX(pitch);
        RotateZ(roll);
        position = pos;
    }
#else
    euler Euler() { return euler(yAxis, zAxis); }                     // Get Euler angles from matrix (z-up)

    void SetRotation(float roll, float pitch, float yaw) {             // Apply euler rotations.
        vec3 pos = position;
        Clear();
        RotateZ(yaw);
        RotateX(pitch);
        RotateY(roll);
        position = pos;
    }
#endif



#ifdef _WIN32
    inline void sincosf(float ang, float* s, float* c){
        *s = sinf(ang);
        *c = cosf(ang);
    }
#endif

    inline void Translate(const float x, const float y, const float z) {
        position += {x, y, z};
    }

    inline void RotateX(const float angle){  // Rotate around x-axis. (Pitch)
        float c, s; 
        sincosf(angle * toRad, &s, &c);
        mat4 M(*this);
        m01=c*M.m01+s*M.m02;  m02=-s*M.m01+c*M.m02;  // 1  0  0  0
        m11=c*M.m11+s*M.m12;  m12=-s*M.m11+c*M.m12;  // 0  c -s  0
        m21=c*M.m21+s*M.m22;  m22=-s*M.m21+c*M.m22;  // 0  s  c  0
        m31=c*M.m31+s*M.m32;  m32=-s*M.m31+c*M.m32;  // 0  0  0  1
    }

    inline void RotateY(const float angle){  // Rotate clockwise around y-axis. (Roll)
        float c, s; 
        sincosf(angle * toRad, &s, &c);
        mat4 M(*this);
        m00=c*M.m00-s*M.m02;  m02=s*M.m00+c*M.m02;  // c  0  s  0
        m10=c*M.m10-s*M.m12;  m12=s*M.m10+c*M.m12;  // 0  1  0  0
        m20=c*M.m20-s*M.m22;  m22=s*M.m20+c*M.m22;  //-s  0  c  0
        m30=c*M.m30-s*M.m32;  m32=s*M.m30+c*M.m32;  // 0  0  0  1
    }

    inline void RotateZ(const float angle){  // Rotate clockwise around z-axis. (-Heading)
        float c, s; 
        sincosf(angle * toRad, &s, &c);
        mat4 M(*this);
        m00=c*M.m00+s*M.m01;  m01=-s*M.m00+c*M.m01;  // c -s  0  0
        m10=c*M.m10+s*M.m11;  m11=-s*M.m10+c*M.m11;  // s  c  0  0
        m20=c*M.m20+s*M.m21;  m21=-s*M.m20+c*M.m21;  // 0  0  1  0
        m30=c*M.m30+s*M.m31;  m31=-s*M.m30+c*M.m31;  // 0  0  0  1
    }

    //======Quaternion-derived rotation======
    void Rotate(const float angle,const vec3 axis){Rotate(angle,axis.x,axis.y,axis.z);} // http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToMatrix/index.htm
    void Rotate(const float angle, float x, float y, float z) {                // Rotate clockwise around a vector. (see 'glRotatef')
#ifdef FASTER
        const float a = invSqrt(x*x + y*y + z*z); x *= a; y *= a; z *= a;  // Normalize the axis vector
#else
        const float a = 1.f / sqrtf(x*x + y*y + z*z); x *= a; y *= a; z *= a;  // Normalize the axis vector
#endif
        const float Rad = angle*toRad;
        const float c = cosf(Rad);
        const float s = sinf(Rad);
        const float t = 1 - c;
        mat4 M;
        M.m00 = x*x*t + c;    M.m10 = x*y*t - z*s;   M.m20 = x*z*t + y*s;
        M.m01 = y*x*t + z*s;  M.m11 = y*y*t + c;     M.m21 = y*z*t - x*s;
        M.m02 = z*x*t - y*s;  M.m12 = z*y*t + x*s;   M.m22 = z*z*t + c;
        Rotate(M);
    }

    //http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToAngle/index.htm
    void toAxisAngle(vec3 &axis, float &angle){                              // Convert a rotation matrix to Axis/angle form. (Eigenvector)
        angle = (float)acos((m00 + m11 + m22 - 1) / 2.f) * toDeg;
        vec3 a(m21 - m12, m02 - m20, m10 - m01); a *= a;
        const float s = 1 / (a.x + a.y + a.z);
        axis.x = (m21 - m12) * s;
        axis.y = (m02 - m20) * s;
        axis.z = (m10 - m01) * s;
    }
    //========================================

    inline void Scale(const float s) { Scale(s, s, s); }
    inline void Scale(const float x, const float y, const float z) {
        m00*=x;  m01*=y;  m02*=z;                 // x 0 0 0
        m10*=x;  m11*=y;  m12*=z;                 // 0 y 0 0
        m20*=x;  m21*=y;  m22*=z;                 // 0 0 z 0
    }

    inline void Normalize() {
        xAxis.normalize();
        yAxis.normalize();
        zAxis.normalize();
        position = {0,0,0};
    }

    inline mat4 Transpose() {
        float f[16] = { m00,m01,m02,m03,  m10,m11,m12,m13,  m20,m21,m22,m23,  m30,m31,m32,m33 };
        return (mat4)f;
    }

    inline mat4 Transpose3x3() const {  // Transpose 3x3 part of 4x4 matrix
        mat4 R;
        R.m00= m00;  R.m10= m01;  R.m20= m02;
        R.m01= m10;  R.m11= m11;  R.m21= m12;
        R.m02= m20;  R.m12= m21;  R.m22= m22;
        return R;
    }

    inline void Rotate(const mat4& M) {  // Matrix * Matrix_3x3  :Rotate this matrix by the rotation part of matrix M.
        const mat4 t(*this);
        m00 = t.m00*M.m00 + t.m01*M.m10 + t.m02*M.m20;
        m01 = t.m00*M.m01 + t.m01*M.m11 + t.m02*M.m21;
        m02 = t.m00*M.m02 + t.m01*M.m12 + t.m02*M.m22;

        m10 = t.m10*M.m00 + t.m11*M.m10 + t.m12*M.m20;
        m11 = t.m10*M.m01 + t.m11*M.m11 + t.m12*M.m21;
        m12 = t.m10*M.m02 + t.m11*M.m12 + t.m12*M.m22;

        m20 = t.m20*M.m00 + t.m21*M.m10 + t.m22*M.m20;
        m21 = t.m20*M.m01 + t.m21*M.m11 + t.m22*M.m21;
        m22 = t.m20*M.m02 + t.m21*M.m12 + t.m22*M.m22;
    }
 
    mat4 operator * (const mat4& M) const {  // Matrix * Matrix
        float f[16]{
            m00 * M.m00 + m01 * M.m10 + m02 * M.m20 + m03 * M.m30,  // m00
            m10 * M.m00 + m11 * M.m10 + m12 * M.m20 + m13 * M.m30,  // m10
            m20 * M.m00 + m21 * M.m10 + m22 * M.m20 + m23 * M.m30,  // m20
            m30 * M.m00 + m31 * M.m10 + m32 * M.m20 + m33 * M.m30,  // m30

            m00 * M.m01 + m01 * M.m11 + m02 * M.m21 + m03 * M.m31,  // m01
            m10 * M.m01 + m11 * M.m11 + m12 * M.m21 + m13 * M.m31,  // m11
            m20 * M.m01 + m21 * M.m11 + m22 * M.m21 + m23 * M.m31,  // m21
            m30 * M.m01 + m31 * M.m11 + m32 * M.m21 + m33 * M.m31,  // m31

            m00 * M.m02 + m01 * M.m12 + m02 * M.m22 + m03 * M.m32,  // m02
            m10 * M.m02 + m11 * M.m12 + m12 * M.m22 + m13 * M.m32,  // m12
            m20 * M.m02 + m21 * M.m12 + m22 * M.m22 + m23 * M.m32,  // m22
            m30 * M.m02 + m31 * M.m12 + m32 * M.m22 + m33 * M.m32,  // m32

            m00 * M.m03 + m01 * M.m13 + m02 * M.m23 + m03 * M.m33,  // m03
            m10 * M.m03 + m11 * M.m13 + m12 * M.m23 + m13 * M.m33,  // m13
            m20 * M.m03 + m21 * M.m13 + m22 * M.m23 + m23 * M.m33,  // m23
            m30 * M.m03 + m31 * M.m13 + m32 * M.m23 + m33 * M.m33   // m33
        };
        return *((mat4*) f);
    }

/*
    //-----------------Matrix x Matrix using SSE -------------------
    mat4 operator * ( const mat4& M) {  // Matrix * Matrix  USING SSE2
        float f[16] alignas(32);
        __m128 row1 = _mm_load_ps(&m[0]);
        __m128 row2 = _mm_load_ps(&m[4]);
        __m128 row3 = _mm_load_ps(&m[8]);
        __m128 row4 = _mm_load_ps(&m[12]);

        for(int i=0; i<4; ++i){
            __m128 brod1 = _mm_set1_ps(M.m[4*i + 0]);
            __m128 brod2 = _mm_set1_ps(M.m[4*i + 1]);
            __m128 brod3 = _mm_set1_ps(M.m[4*i + 2]);
            __m128 brod4 = _mm_set1_ps(M.m[4*i + 3]);
            __m128 row = _mm_add_ps(
                         _mm_add_ps(
                             _mm_mul_ps(brod1, row1),
                             _mm_mul_ps(brod2, row2)),
                         _mm_add_ps(
                             _mm_mul_ps(brod3, row3),
                             _mm_mul_ps(brod4, row4)
                         ));
            _mm_store_ps(&f[4*i], row);
        }
        return *((mat4*) f);
    }
    //--------------------------------------------------------------
*/
/*
    //-----------------Matrix x Matrix using AVX -------------------
    // dual linear combination using AVX instructions on YMM regs
    static inline __m256 twolincomb_AVX_8(__m256 A01, const mat4 &B) {
        __m256 result;
        result = _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x00), _mm256_broadcast_ps((__m128*)&B.m00));
        result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0x55), _mm256_broadcast_ps((__m128*)&B.m01)));
        result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xaa), _mm256_broadcast_ps((__m128*)&B.m02)));
        result = _mm256_add_ps(result, _mm256_mul_ps(_mm256_shuffle_ps(A01, A01, 0xff), _mm256_broadcast_ps((__m128*)&B.m03)));
        return result;
    }

    mat4 operator * ( const mat4& M) {  // Matrix * Matrix  USING AVX
        float f[16] alignas(32);
        __m256 A01 = _mm256_loadu_ps(&M.m00);
        __m256 A23 = _mm256_loadu_ps(&M.m02);
        __m256 out01x = twolincomb_AVX_8(A01, m);
        __m256 out23x = twolincomb_AVX_8(A23, m);
        _mm256_storeu_ps(&f[0], out01x);
        _mm256_storeu_ps(&f[8], out23x);
        return *((mat4*) f);
    }
    //--------------------------------------------------------------
*/

    inline void operator *= (const mat4& b){ *this = *this * b; }  // Matrix *= Matrix

    inline vec4 operator * (const vec4& v) const {  // Matrix * vec4
        return vec4{ m00*v.x + m01*v.y + m02*v.z + m03,
                     m10*v.x + m11*v.y + m12*v.z + m13,
                     m20*v.x + m21*v.y + m22*v.z + m23,
                     m30*v.x + m31*v.y + m32*v.z + m33 };
    }

    void SetProjection(float aspect, float fovy, float nearplane, float farplane) {
        float f = 1.f / tanf((fovy * toRad) / 2.f);
        float d = nearplane - farplane;
        m00 = f/aspect;  m01 = 0;        m02 = 0;                        m03 = 0;
        m10 = 0;         m11 = f;        m12 = 0;                        m13 = 0;
        m20 = 0;         m21 = 0;        m22 = (farplane+nearplane)/d;   m23 = (2*farplane*nearplane)/d;
        m30 = 0;         m31 = 0;        m32 = -1;                       m33 = 0;
    }

    void SetOrtho(float left, float right, float bottom, float top, float nearplane, float farplane){
        float w  = right - left;
        float h  = top - bottom;
        float d  = farplane - nearplane;
        float tx = (right + left) / w;
        float ty = (top + bottom) / h;
        float tz = (farplane + nearplane) / d;
        m00=2/w;    m01=0;      m02=0;      m03=-tx;
        m10=0;      m11=2/h;    m12=0;      m13=-ty;
        m20=0;      m21=0;      m22=-2/d;   m23=-tz;
        m30=0;      m31=0;      m32=0;      m33=1;
    }

//    void LookAt(vec3 target, vec3 up = { 0, 0, 1 }) {  // z-up
    void LookAt(vec3 target, vec3 up = { 0, 1, 0 }) {  // y-up
        zAxis = position - target;
        if(zAxis.x == 0 && zAxis.y == 0 && zAxis.z == 0) return;  // prevent NAN error
        zAxis.normalize();
        if(fabs(zAxis.y) > 0.999f)                                // prevent NAN if fwd==up/down
        xAxis = up.cross(zAxis);    xAxis.normalize();
        yAxis = zAxis.cross(xAxis); yAxis.normalize();
    }

    // Look at direction (direction must be normailzed)
    void Look(vec3 direction, vec3 up = { 0, 1, 0 }) {        // y-up
        zAxis = - direction;        //zAxis.normalize();      // assume direction is normalized
        if(fabs(zAxis.y) < 0.99f)                             // prevent NAN if fwd==up/down
          {xAxis = up.cross(zAxis); xAxis.normalize_fast();}
        yAxis = zAxis.cross(xAxis); yAxis.normalize_fast();
    }

    void Print(){  // for debugging
        //printf(" \n");
        printf("%9f %9f %9f %9f\n", m00,m10,m20,m30);
        printf("%9f %9f %9f %9f\n", m01,m11,m21,m31);
        printf("%9f %9f %9f %9f\n", m02,m12,m22,m32);
        printf("%9f %9f %9f %9f\n", m03,m13,m23,m33);
        printf(" \n");
    }

    inline mat4 Inverse() const {
        mat4 R;
        float s[6];
        float c[6];
        s[0] = m00*m11 - m10*m01;
        s[1] = m00*m12 - m10*m02;
        s[2] = m00*m13 - m10*m03;
        s[3] = m01*m12 - m11*m02;
        s[4] = m01*m13 - m11*m03;
        s[5] = m02*m13 - m12*m03;

        c[0] = m20*m31 - m30*m21;
        c[1] = m20*m32 - m30*m22;
        c[2] = m20*m33 - m30*m23;
        c[3] = m21*m32 - m31*m22;
        c[4] = m21*m33 - m31*m23;
        c[5] = m22*m33 - m32*m23;

        float idet = 1.0f/( s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0] );

        R.m00 = ( m11 * c[5] - m12 * c[4] + m13 * c[3]) * idet;
        R.m01 = (-m01 * c[5] + m02 * c[4] - m03 * c[3]) * idet;
        R.m02 = ( m31 * s[5] - m32 * s[4] + m33 * s[3]) * idet;
        R.m03 = (-m21 * s[5] + m22 * s[4] - m23 * s[3]) * idet;

        R.m10 = (-m10 * c[5] + m12 * c[2] - m13 * c[1]) * idet;
        R.m11 = ( m00 * c[5] - m02 * c[2] + m03 * c[1]) * idet;
        R.m12 = (-m30 * s[5] + m32 * s[2] - m33 * s[1]) * idet;
        R.m13 = ( m20 * s[5] - m22 * s[2] + m23 * s[1]) * idet;

        R.m20 = ( m10 * c[4] - m11 * c[2] + m13 * c[0]) * idet;
        R.m21 = (-m00 * c[4] + m01 * c[2] - m03 * c[0]) * idet;
        R.m22 = ( m30 * s[4] - m31 * s[2] + m33 * s[0]) * idet;
        R.m23 = (-m20 * s[4] + m21 * s[2] - m23 * s[0]) * idet;

        R.m30 = (-m10 * c[3] + m11 * c[1] - m12 * c[0]) * idet;
        R.m31 = ( m00 * c[3] - m01 * c[1] + m02 * c[0]) * idet;
        R.m32 = (-m30 * s[3] + m31 * s[1] - m32 * s[0]) * idet;
        R.m33 = ( m20 * s[3] - m21 * s[1] + m22 * s[0]) * idet;

        return R;
    }

    inline mat4 Inverse_fast() const {  // Assumes matrix is orthogonal. (untested)
        mat4 R = Transpose3x3();
        vec3 pos = R * position;
        R.position = - pos;
        return R;
    }

    void from_quat(quat q) {  // convert quaternion to matrix (untested.. may need transpose)
        float w = q.w;
        float x = q.x;
        float y = q.y;
        float z = q.z;
        float w2 = w * w;
        float x2 = x * x;
        float y2 = y * y;
        float z2 = z * z;

        m00 = w2 + x2 - y2 - z2;
        m01 = 2.f * (x*y + w*z);
        m02 = 2.f * (x*z - w*y);
        m03 = 0.f;

        m10 = 2.f * (x*y - w*z);
        m11 = w2 - x2 + y2 - z2;
        m12 = 2.f * (y*z + w*x);
        m13 = 0.f;

        m20 = 2.f * (x*z + w*y);
        m21 = 2.f * (y*z - w*x);
        m22 = w2 - x2 - y2 + z2;
        m23 = 0.f;

        m30 = m31 = m32 = 0.f;
        m33 = 1.f;

        //*this = Transpose();
    }

};

#endif
