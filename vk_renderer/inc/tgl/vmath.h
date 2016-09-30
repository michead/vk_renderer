#ifndef _VMATH_H_
#define _VMATH_H_

#include <cmath>
#include <cstdlib>
#include <array>

#if defined(_WIN32)
#include <numeric>
#undef near
#undef far
#endif

#include "common.h"

#define PIf 3.14159265f
#define PI 3.1415926535897932384626433832795

// pi (float)
const float pif = PIf;
// pi (double)
const double pid = PI;
// pi
const double pi = PI;

// std math brought into namespace --------
using std::abs;
using std::sqrt;
using std::pow;
using std::sin;
using std::cos;
using std::tan;
using std::asin;
using std::acos;
using std::atan;
using std::atan2;
using std::ceil;
using std::floor;
using std::round;

// math utilities --------------------------
// max
inline int max(int a, int b) { return (a > b)?a:b; }
inline float max(float a, float b) { return (a > b)?a:b; }
// min
inline int min(int a, int b) { return (a < b)?a:b; }
inline float min(float a, float b) { return (a < b)?a:b; }
// clamp (sets x between m and M)
inline int clamp(int x, int m, int M) { return min(M,max(m,x)); }
inline float clamp(float x, float m, float M) { return min(M,max(m,x)); }
// degrees to radians
inline float radians(float deg) { return deg / float(180) * float(pi); }
// radians to degrees
inline float degrees(float rad) { return rad / float(pi) * float(180); }
// integer power of 2
inline int pow2(int x) { int ret = 1; for(int i = 0; i < x; i ++) ret *= 2; return ret; }
// square
inline int sqr(int x) { return x*x; }
inline float sqr(float x) { return x*x; }
// sqrt2
inline int sqrt_int(int x) { return (int)round(sqrt((double)x)); }

// 2d vector
struct vec2f {
    float x; // X component
    float y; // Y component
    
    // Default constructor (set to zeros)
    vec2f() : x(0), y(0) { }
    // Element-setting constructor
    vec2f(float x, float y) : x(x), y(y) { }
    // Vector constructor
    template<typename T> vec2f(const vector<T>& v) { error_if_not(v.size() == size, "bad length"); std::copy(v.begin(),v.end(),begin()); }
    
    // Element access
    float& operator[](int i) { return (&x)[i]; }
    // Element access
    float operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    float* begin() { return data(); }
    float* end() { return data()+size; }
    const float* begin() const { return data(); }
    const float* end() const { return data()+size; }
    
    // type system
    typedef float value_type;
    static const int size = 2;
    float* data() { return &x; }
    const float* data() const { return &x; }
};

// 2d vector Constants -------------------------------
// zero
const vec2f zero2f = vec2f(0, 0);
// one
const vec2f one2f = vec2f(1, 1);
// x, y
const vec2f x2f = vec2f(1, 0);
const vec2f y2f = vec2f(0, 1);

// 2d component-wise equality  -----------------------
inline bool operator==(const vec2f& a, const vec2f& b) { return a.x==b.x && a.y==b.y; }
inline bool operator!=(const vec2f& a, const vec2f& b) { return !operator==(a,b); }

// 2d lexical ordering  ------------------------------
inline bool operator<(const vec2f& a, const vec2f& b) { return a.x<b.x || (a.x==b.x && a.y<b.y); }

// 2d component-wise arithmetic operators -----------
inline vec2f operator-(const vec2f& a) { return vec2f(-a.x, -a.y); }
inline vec2f operator+(const vec2f& a, const vec2f& b) { return vec2f(a.x+b.x, a.y+b.y); }
inline vec2f& operator+=(vec2f& a, const vec2f& b) { a.x+=b.x; a.y+=b.y; return a; }
inline vec2f operator-(const vec2f& a, const vec2f& b) { return vec2f(a.x-b.x, a.y-b.y); }
inline vec2f& operator-=(vec2f& a, const vec2f& b) { a.x-=b.x; a.y-=b.y; return a; }
inline vec2f operator*(const vec2f& a, const vec2f& b) { return vec2f(a.x*b.x, a.y*b.y); }
inline vec2f& operator*=(vec2f& a, const vec2f& b) { a.x*=b.x; a.y*=b.y; return a; }
inline vec2f operator/(const vec2f& a, const vec2f& b) { return vec2f(a.x/b.x, a.y/b.y); }
inline vec2f& operator/=(vec2f& a, const vec2f& b) { a.x/=b.x; a.y/=b.y; return a; }
inline vec2f operator*(const vec2f& a, float b) { return vec2f(a.x*b, a.y*b); }
inline vec2f operator*(float a, const vec2f& b) { return vec2f(a*b.x, a*b.y); }
inline vec2f& operator*=(vec2f& a, float b) { a.x*=b; a.y*=b; return a; }
template<typename T, typename R> inline vec2f operator*(const vec2f& a, const R& b) { return vec2f(a.x*b, a.y*b); }
template<typename T, typename R> inline vec2f operator*(const R& a, const vec2f& b) { return vec2f(a*b.x, a*b.y); }
template<typename T, typename R> inline vec2f& operator*=(vec2f& a, const R& b) { a.x*=b; a.y*=b; return a; }
inline vec2f operator/(const vec2f& a, float b) { return vec2f(a.x/b, a.y/b); }
inline vec2f operator/(float a, const vec2f& b) { return vec2f(a/b.x, a/b.y); }
inline vec2f& operator/=(vec2f& a, float b) { a.x/=b; a.y/=b; return a; }
template<typename T, typename R> inline vec2f operator/(const vec2f& a, const R& b) { return vec2f(a.x/b, a.y/b); }
template<typename T, typename R> inline vec2f operator/(const R& a, const vec2f& b) { return vec2f(a/b.x, a/b.y); }
template<typename T, typename R> inline vec2f& operator/=(vec2f& a, const R& b) { a.x/=b; a.y/=b; return a; }

// 2d component-wise functions ----------------------
// max, min, clamp
inline vec2f max(const vec2f& a, const vec2f& b) { return vec2f(max(a.x,b.x), max(a.y,b.y)); }
inline vec2f max(const vec2f& a, float b) { return vec2f(max(a.x,b), max(a.y,b)); }
inline vec2f max(float a, const vec2f& b) { return vec2f(max(a,b.x), max(a,b.y)); }
inline vec2f min(const vec2f& a, const vec2f& b) { return vec2f(min(a.x,b.x), min(a.y,b.y)); }
inline vec2f min(const vec2f& a, float b) { return vec2f(min(a.x,b), min(a.y,b)); }
inline vec2f min(float a, const vec2f& b) { return vec2f(min(a,b.x), min(a,b.y)); }
inline vec2f clamp(const vec2f& x, const vec2f& m, const vec2f& M) { return vec2f(clamp(x.x,m.x,M.x), clamp(x.y,m.y,M.y)); }
inline vec2f clamp(const vec2f& x, float m, float M) { return vec2f(clamp(x.x,m,M), clamp(x.y,m,M)); }
inline float mean(const vec2f& a) { return (a.x + a.y) / 2; }

// 2d vector operations -----------------------------
// dot product
inline float dot(const vec2f& a, const vec2f& b) { return a.x*b.x+a.y*b.y; }
// length and lengthSqr
inline float length(const vec2f& a) { return sqrt(dot(a,a)); }
inline float lengthSqr(const vec2f& a) { return dot(a,a); }
// normalize
inline vec2f normalize(const vec2f& a) { auto l = length(a); if (l==0) return vec2f(0,0); else return a * 1/l; }

// 3d vector
struct vec3f {
    float x; // X component
    float y; // Y component
    float z; // Z component
    
    // Default constructor (set to zeros)
    vec3f() : x(0), y(0), z(0) { }
    // Element-setting constructor
    vec3f(float x, float y, float z) : x(x), y(y), z(z) { }
    // Vector constructor
    template<typename T> vec3f(const vector<T>& v) { error_if_not(v.size() == size, "bad length"); std::copy(v.begin(),v.end(),begin()); }

    // Element access
    float& operator[](int i) { return (&x)[i]; }
    // Element access
    float operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    float* begin() { return data(); }
    float* end() { return data()+size; }
    const float* begin() const { return data(); }
    const float* end() const { return data()+size; }
    
    // type system
    typedef float value_type;
    static const int size = 3;
    float* data() { return &x; }
    const float* data() const { return &x; }
};

// 3d vector Constants -------------------------------
// zero
const vec3f zero3f = vec3f(0, 0, 0);
// one
const vec3f one3f = vec3f(1, 1, 1);
// x, y, z
const vec3f x3f = vec3f(1, 0, 0);
const vec3f y3f = vec3f(0, 1, 0);
const vec3f z3f = vec3f(0, 0, 1);

// 3d component-wise equality  -----------------------
inline bool operator==(const vec3f& a, const vec3f& b) { return a.x==b.x && a.y==b.y && a.z==b.z; }
inline bool operator!=(const vec3f& a, const vec3f& b) { return !operator==(a,b); }

// 3d lexical ordering  ------------------------------
inline bool operator<(const vec3f& a, const vec3f& b) { return a.x<b.x || (a.x==b.x && a.y<b.y) || (a.x==b.x && a.y==b.y && a.z<b.z); }

// 3d component-wise arithmetic operators -----------
inline vec3f operator-(const vec3f& a) { return vec3f(-a.x, -a.y, -a.z); }
inline vec3f operator+(const vec3f& a, const vec3f& b) { return vec3f(a.x+b.x, a.y+b.y, a.z+b.z); }
inline vec3f& operator+=(vec3f& a, const vec3f& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
inline vec3f operator-(const vec3f& a, const vec3f& b) { return vec3f(a.x-b.x, a.y-b.y, a.z-b.z); }
inline vec3f& operator-=(vec3f& a, const vec3f& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; return a; }
inline vec3f operator*(const vec3f& a, const vec3f& b) { return vec3f(a.x*b.x, a.y*b.y, a.z*b.z); }
inline vec3f& operator*=(vec3f& a, const vec3f& b) { a.x*=b.x; a.y*=b.y; a.z*=b.z; return a; }
inline vec3f operator/(const vec3f& a, const vec3f& b) { return vec3f(a.x/b.x, a.y/b.y, a.z/b.z); }
inline vec3f& operator/=(vec3f& a, const vec3f& b) { a.x/=b.x; a.y/=b.y; a.z/=b.z; return a; }
inline vec3f operator*(const vec3f& a, float b) { return vec3f(a.x*b, a.y*b, a.z*b); }
inline vec3f operator*(float a, const vec3f& b) { return vec3f(a*b.x, a*b.y, a*b.z); }
inline vec3f& operator*=(vec3f& a, float b) { a.x*=b; a.y*=b; a.z*=b; return a; }
template<typename T, typename R> inline vec3f operator*(const vec3f& a, const R& b) { return vec3f(a.x*b, a.y*b, a.z*b); }
template<typename T, typename R> inline vec3f operator*(const R& a, const vec3f& b) { return vec3f(a*b.x, a*b.y, a*b.z); }
template<typename T, typename R> inline vec3f& operator*=(vec3f& a, const R& b) { a.x*=b; a.y*=b; a.z*=b; return a; }
inline vec3f operator/(const vec3f& a, float b) { return vec3f(a.x/b, a.y/b, a.z/b); }
inline vec3f operator/(float a, const vec3f& b) { return vec3f(a/b.x, a/b.y, a/b.z); }
inline vec3f& operator/=(vec3f& a, float b) { a.x/=b; a.y/=b; a.z/=b; return a; }
template<typename T, typename R> inline vec3f operator/(const vec3f& a, const R& b) { return vec3f(a.x/b, a.y/b, a.z/b); }
template<typename T, typename R> inline vec3f operator/(const R& a, const vec3f& b) { return vec3f(a/b.x, a/b.y, a/b.z); }
template<typename T, typename R> inline vec3f& operator/=(vec3f& a, const R& b) { a.x/=b; a.y/=b; a.z/=b; return a; }

// 3d component-wise functions ----------------------
// max, min, clamp
inline vec3f max(const vec3f& a, const vec3f& b) { return vec3f(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z)); }
inline vec3f max(const vec3f& a, float b) { return vec3f(max(a.x,b), max(a.y,b), max(a.z,b)); }
inline vec3f max(float a, const vec3f& b) { return vec3f(max(a,b.x), max(a,b.y), max(a,b.z)); }
inline vec3f min(const vec3f& a, const vec3f& b) { return vec3f(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z)); }
inline vec3f min(const vec3f& a, float b) { return vec3f(min(a.x,b), min(a.y,b), min(a.z,b)); }
inline vec3f min(float a, const vec3f& b) { return vec3f(min(a,b.x), min(a,b.y), min(a,b.z)); }
inline vec3f clamp(const vec3f& x, const vec3f& m, const vec3f& M) { return vec3f(clamp(x.x,m.x,M.x), clamp(x.y,m.y,M.y), clamp(x.z,m.z,M.z)); }
inline vec3f clamp(const vec3f& x, float m, float M) { return vec3f(clamp(x.x,m,M), clamp(x.y,m,M), clamp(x.z,m,M)); }
inline float mean(const vec3f& a) { return (a.x + a.y + a.z) / 3; }
inline vec3f pow(const vec3f& a, float b) { return vec3f(pow(a.x,b),pow(a.y,b),pow(a.z,b)); }
inline vec3f sqrt(const vec3f& a) { return vec3f(sqrt(a.x),sqrt(a.y),sqrt(a.z)); }
inline vec3f exp(const vec3f& a) { return vec3f(exp(a.x),exp(a.y),exp(a.z)); }

// 3d vector operations -----------------------------
// dot product
inline float dot(const vec3f& a, const vec3f& b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
// length and lengthSqr
inline float length(const vec3f& a) { return sqrt(dot(a,a)); }
inline float lengthSqr(const vec3f& a) { return dot(a,a); }
// normnalization
inline vec3f normalize(const vec3f& a) { auto l = length(a); if (l==0) return vec3f(0,0,0); else return a * 1/l; }
// distance and diatcne squared
inline float dist(const vec3f& a, const vec3f& b) { return length(a-b); }
inline float distSqr(const vec3f& a, const vec3f& b) { return lengthSqr(a-b); }
// cross product
inline vec3f cross(const vec3f& a, const vec3f& b) { return vec3f(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x); }
// orthogonal (from: http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts)
inline vec3f orthogonal(const vec3f& v) { return abs(v.x) > abs(v.z) ? vec3f(-v.y, v.x, 0.0) : vec3f(0.0, -v.z, v.y); }
// orthonormalize a wrt b
inline vec3f orthonormalize(const vec3f& a, const vec3f& b) { return normalize(a - b*dot(a,b)); }
// direction of mirror reflection of v with normal n (v is incoming)
inline vec3f reflect(const vec3f& v, const vec3f& n) { return v - 2 * dot(n,v) * n; }
// direction of refraction of v with normal n (v is incoming --- from glsl documentation)
inline vec3f refract(const vec3f& v, const vec3f& n, float eta) {  auto k = 1 - eta * eta * (1 - dot(n, v) * dot(n, v)); return (k < 0) ? vec3f(0,0,0) : eta * v - (eta * dot(n, v) + sqrt(k)) * n; }

// 4d Vector
struct vec4f {
    float x; // X component
    float y; // Y component
    float z; // Z component
    float w; // W component
    
    // Default constructor (set to zeros)
    vec4f() : x(0), y(0), z(0), w(0) { }
    // Element-setting constructor
    vec4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) { }
    // Vector constructor
    template<typename T> vec4f(const vector<T>& v) { error_if_not(v.size() == size, "bad length"); std::copy(v.begin(),v.end(),begin()); }
    
    // Element access
    float& operator[](int i) { return (&x)[i]; }
    // Element access
    float operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    float* begin() { return data(); }
    float* end() { return data()+size; }
    const float* begin() const { return data(); }
    const float* end() const { return data()+size; }
    
    // type system
    typedef float value_type;
    static const int size = 4;
    float* data() { return &x; }
    const float* data() const { return &x; }
};

// 4d vector Constants -------------------------------
// zero
const vec4f zero4f = vec4f(0, 0, 0, 0);
// one
const vec4f one4f = vec4f(1, 1, 1, 1);
// x, y, z, w
const vec4f x4f = vec4f(1, 0, 0, 0);
const vec4f y4f = vec4f(0, 1, 0, 0);
const vec4f z4f = vec4f(0, 0, 1, 0);
const vec4f w4f = vec4f(0, 0, 0, 1);

// 4d component-wise equality  -----------------------
inline bool operator==(const vec4f& a, const vec4f& b) { return a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w; }
inline bool operator!=(const vec4f& a, const vec4f& b) { return !operator==(a,b); }

// 4d lexical ordering  ------------------------------
inline bool operator<(const vec4f& a, const vec4f& b) { return a.x<b.x || (a.x==b.x && a.y<b.y) || (a.x==b.x && a.y==b.y && a.z<b.z) || (a.x==b.x && a.y==b.y && a.z==b.z && a.w<b.w); }

// 4d component-wise arithmetic operators -----------
inline vec4f operator-(const vec4f& a) { return vec4f(-a.x, -a.y, -a.z, -a.w); }
inline vec4f operator+(const vec4f& a, const vec4f& b) { return vec4f(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w); }
inline vec4f& operator+=(vec4f& a, const vec4f& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; a.w+=b.w; return a; }
inline vec4f operator-(const vec4f& a, const vec4f& b) { return vec4f(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w); }
inline vec4f& operator-=(vec4f& a, const vec4f& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; a.w-=b.w; return a; }
inline vec4f operator*(const vec4f& a, const vec4f& b) { return vec4f(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }
inline vec4f& operator*=(vec4f& a, const vec4f& b) { a.x*=b.x; a.y*=b.y; a.z*=b.z; a.w*=b.w; return a; }
inline vec4f operator/(const vec4f& a, const vec4f& b) { return vec4f(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w); }
inline vec4f& operator/=(vec4f& a, const vec4f& b) { a.x/=b.x; a.y/=b.y; a.z/=b.z; a.w/=b.w; return a; }
inline vec4f operator*(const vec4f& a, float b) { return vec4f(a.x*b, a.y*b, a.z*b, a.w*b); }
inline vec4f operator*(float a, const vec4f& b) { return vec4f(a*b.x, a*b.y, a*b.z, a*b.w); }
inline vec4f& operator*=(vec4f& a, float b) { a.x*=b; a.y*=b; a.z*=b; a.w*=b; return a; }
template<typename T, typename R> inline vec4f operator*(const vec4f& a, const R& b) { return vec4f(a.x*b, a.y*b, a.z*b, a.w*b); }
template<typename T, typename R> inline vec4f operator*(const R& a, const vec4f& b) { return vec4f(a*b.x, a*b.y, a*b.z, a*b.w); }
template<typename T, typename R> inline vec4f& operator*=(vec4f& a, const R& b) { a.x*=b; a.y*=b; a.z*=b; a.w*=b; return a; }
inline vec4f operator/(const vec4f& a, float b) { return vec4f(a.x/b, a.y/b, a.z/b, a.w/b); }
inline vec4f operator/(float a, const vec4f& b) { return vec4f(a/b.x, a/b.y, a/b.z, a/b.w); }
inline vec4f& operator/=(vec4f& a, float b) { a.x/=b; a.y/=b; a.z/=b; a.w/=b; return a; }
template<typename T, typename R> inline vec4f operator/(const vec4f& a, const R& b) { return vec4f(a.x/b, a.y/b, a.z/b, a.w/b); }
template<typename T, typename R> inline vec4f operator/(const R& a, const vec4f& b) { return vec4f(a/b.x, a/b.y, a/b.z, a/b.w); }
template<typename T, typename R> inline vec4f& operator/=(vec4f& a, const R& b) { a.x/=b; a.y/=b; a.z/=b; a.w/=b; return a; }

// 4d component-wise functions ----------------------
// max, min, clamp
inline vec4f max(const vec4f& a, const vec4f& b) { return vec4f(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z), max(a.w,b.w)); }
inline vec4f max(const vec4f& a, float b) { return vec4f(max(a.x,b), max(a.y,b), max(a.z,b), max(a.w,b)); }
inline vec4f max(float a, const vec4f& b) { return vec4f(max(a,b.x), max(a,b.y), max(a,b.z), max(a,b.w)); }
inline vec4f min(const vec4f& a, const vec4f& b) { return vec4f(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z), min(a.w,b.w)); }
inline vec4f min(const vec4f& a, float b) { return vec4f(min(a.x,b), min(a.y,b), min(a.z,b), min(a.w,b)); }
inline vec4f min(float a, const vec4f& b) { return vec4f(min(a,b.x), min(a,b.y), min(a,b.z), min(a,b.w)); }
inline vec4f clamp(const vec4f& x, const vec4f& m, const vec4f& M) { return vec4f(clamp(x.x,m.x,M.x), clamp(x.y,m.y,M.y), clamp(x.z,m.z,M.z), clamp(x.w,m.w,M.w)); }
inline vec4f clamp(const vec4f& x, float m, float M) { return vec4f(clamp(x.x,m,M), clamp(x.y,m,M), clamp(x.z,m,M), clamp(x.w,m,M)); }
inline float mean(const vec4f& a) { return (a.x + a.y + a.z + a.w) / 4; }
inline vec4f pow(const vec4f& a, float b) { return vec4f(pow(a.x,b),pow(a.y,b),pow(a.z,b),pow(a.w,b)); }
inline vec4f sqrt(const vec4f& a) { return vec4f(sqrt(a.x),sqrt(a.y),sqrt(a.z),sqrt(a.w)); }
inline vec4f exp(const vec4f& a) { return vec4f(exp(a.x),exp(a.y),exp(a.z),exp(a.w)); }

// 4d vector operations -----------------------------
// dot product
inline float dot(const vec4f& a, const vec4f& b) { return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w; }
// length and lengthSqr
inline float length(const vec4f& a) { return sqrt(dot(a,a)); }
inline float lengthSqr(const vec4f& a) { return dot(a,a); }
// normnalization
inline vec4f normalize(const vec4f& a) { auto l = length(a); if (l==0) return vec4f(0,0,0,0); else return a * 1/l; }

// 2d integer Vector
struct vec2i {
    int x; // X component
    int y; // Y component
    
    // Default constructor (set to zeros)
    vec2i() : x(0), y(0) { }
    // Element-setting constructor
    vec2i(int x, int y) : x(x), y(y) { }

    // Element access
    int& operator[](int i) { return (&x)[i]; }
    // Element access
    int operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    int* begin() { return data(); }
    int* end() { return data()+size; }
    const int* begin() const { return data(); }
    const int* end() const { return data()+size; }
    
    // type system
    typedef int value_type;
    static const int size = 2;
    int* data() { return &x; }
    const int* data() const { return &x; }
};

// 2d vector Constants -------------------------------
// zero
const vec2i zero2i = vec2i(0, 0);
// one
const vec2i one2i = vec2i(1, 1);
// x, y
const vec2i x2i = vec2i(1, 0);
const vec2i y2i = vec2i(0, 1);

// 2d component-wise equality  -----------------------
inline bool operator==(const vec2i& a, const vec2i& b) { return a.x==b.x && a.y==b.y; }
inline bool operator!=(const vec2i& a, const vec2i& b) { return !operator==(a,b); }

// 2d lexical ordering  ------------------------------
inline bool operator<(const vec2i& a, const vec2i& b) { return a.x<b.x || (a.x==b.x && a.y<b.y); }

// 2d component-wise arithmetic operators -----------
inline vec2i operator-(const vec2i& a) { return vec2i(-a.x, -a.y); }
inline vec2i operator+(const vec2i& a, const vec2i& b) { return vec2i(a.x+b.x, a.y+b.y); }
inline vec2i& operator+=(vec2i& a, const vec2i& b) { a.x+=b.x; a.y+=b.y; return a; }
inline vec2i operator-(const vec2i& a, const vec2i& b) { return vec2i(a.x-b.x, a.y-b.y); }
inline vec2i& operator-=(vec2i& a, const vec2i& b) { a.x-=b.x; a.y-=b.y; return a; }
inline vec2i operator*(const vec2i& a, const vec2i& b) { return vec2i(a.x*b.x, a.y*b.y); }
inline vec2i& operator*=(vec2i& a, const vec2i& b) { a.x*=b.x; a.y*=b.y; return a; }
inline vec2i operator/(const vec2i& a, const vec2i& b) { return vec2i(a.x/b.x, a.y/b.y); }
inline vec2i& operator/=(vec2i& a, const vec2i& b) { a.x/=b.x; a.y/=b.y; return a; }
inline vec2i operator*(const vec2i& a, int b) { return vec2i(a.x*b, a.y*b); }
inline vec2i operator*(int a, const vec2i& b) { return vec2i(a*b.x, a*b.y); }
inline vec2i& operator*=(vec2i& a, int b) { a.x*=b; a.y*=b; return a; }
template<typename T, typename R> inline vec2i operator*(const vec2i& a, const R& b) { return vec2i(a.x*b, a.y*b); }
template<typename T, typename R> inline vec2i operator*(const R& a, const vec2i& b) { return vec2i(a*b.x, a*b.y); }
template<typename T, typename R> inline vec2i& operator*=(vec2i& a, const R& b) { a.x*=b; a.y*=b; return a; }
inline vec2i operator/(const vec2i& a, int b) { return vec2i(a.x/b, a.y/b); }
inline vec2i operator/(int a, const vec2i& b) { return vec2i(a/b.x, a/b.y); }
inline vec2i& operator/=(vec2i& a, int b) { a.x/=b; a.y/=b; return a; }
template<typename T, typename R> inline vec2i operator/(const vec2i& a, const R& b) { return vec2i(a.x/b, a.y/b); }
template<typename T, typename R> inline vec2i operator/(const R& a, const vec2i& b) { return vec2i(a/b.x, a/b.y); }
template<typename T, typename R> inline vec2i& operator/=(vec2i& a, const R& b) { a.x/=b; a.y/=b; return a; }

// 2d component-wise functions ----------------------
// max, min, clamp
inline vec2i max(const vec2i& a, const vec2i& b) { return vec2i(max(a.x,b.x), max(a.y,b.y)); }
inline vec2i max(const vec2i& a, int b) { return vec2i(max(a.x,b), max(a.y,b)); }
inline vec2i max(int a, const vec2i& b) { return vec2i(max(a,b.x), max(a,b.y)); }
inline vec2i min(const vec2i& a, const vec2i& b) { return vec2i(min(a.x,b.x), min(a.y,b.y)); }
inline vec2i min(const vec2i& a, int b) { return vec2i(min(a.x,b), min(a.y,b)); }
inline vec2i min(int a, const vec2i& b) { return vec2i(min(a,b.x), min(a,b.y)); }
inline vec2i clamp(const vec2i& x, const vec2i& m, const vec2i& M) { return vec2i(clamp(x.x,m.x,M.x), clamp(x.y,m.y,M.y)); }
inline vec2i clamp(const vec2i& x, int m, int M) { return vec2i(clamp(x.x,m,M), clamp(x.y,m,M)); }

// 3d integer vector
struct vec3i {
    int x; // X component
    int y; // Y component
    int z; // Z component
    
    // Default constructor (set to zeros)
    vec3i() : x(0), y(0), z(0) { }
    // Element-setting constructor
    vec3i(int x, int y, int z) : x(x), y(y), z(z) { }

    // Element access
    int& operator[](int i) { return (&x)[i]; }
    // Element access
    int operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    int* begin() { return data(); }
    int* end() { return data()+size; }
    const int* begin() const { return data(); }
    const int* end() const { return data()+size; }
    
    // type system
    typedef int value_type;
    static const int size = 3;
    int* data() { return &x; }
    const int* data() const { return &x; }
};

// 3d vector Constants -------------------------------
// zero
const vec3i zero3i = vec3i(0, 0, 0);
// one
const vec3i one3i = vec3i(1, 1, 1);
// x, y, z
const vec3i x3i= vec3i(1, 0, 0);
const vec3i y3i = vec3i(0, 1, 0);
const vec3i z3i = vec3i(0, 0, 1);

// 3d component-wise equality  -----------------------
inline bool operator==(const vec3i& a, const vec3i& b) { return a.x==b.x && a.y==b.y && a.z==b.z; }
inline bool operator!=(const vec3i& a, const vec3i& b) { return !operator==(a,b); }

// 3d lexical ordering  ------------------------------
inline bool operator<(const vec3i& a, const vec3i& b) { return a.x<b.x || (a.x==b.x && a.y<b.y) || (a.x==b.x && a.y==b.y && a.z<b.z); }

// 3d component-wise arithmetic operators -----------
inline vec3i operator-(const vec3i& a) { return vec3i(-a.x, -a.y, -a.z); }
inline vec3i operator+(const vec3i& a, const vec3i& b) { return vec3i(a.x+b.x, a.y+b.y, a.z+b.z); }
inline vec3i& operator+=(vec3i& a, const vec3i& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; return a; }
inline vec3i operator-(const vec3i& a, const vec3i& b) { return vec3i(a.x-b.x, a.y-b.y, a.z-b.z); }
inline vec3i& operator-=(vec3i& a, const vec3i& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; return a; }
inline vec3i operator*(const vec3i& a, const vec3i& b) { return vec3i(a.x*b.x, a.y*b.y, a.z*b.z); }
inline vec3i& operator*=(vec3i& a, const vec3i& b) { a.x*=b.x; a.y*=b.y; a.z*=b.z; return a; }
inline vec3i operator/(const vec3i& a, const vec3i& b) { return vec3i(a.x/b.x, a.y/b.y, a.z/b.z); }
inline vec3i& operator/=(vec3i& a, const vec3i& b) { a.x/=b.x; a.y/=b.y; a.z/=b.z; return a; }
inline vec3i operator*(const vec3i& a, int b) { return vec3i(a.x*b, a.y*b, a.z*b); }
inline vec3i operator*(int a, const vec3i& b) { return vec3i(a*b.x, a*b.y, a*b.z); }
inline vec3i& operator*=(vec3i& a, int b) { a.x*=b; a.y*=b; a.z*=b; return a; }
template<typename T, typename R> inline vec3i operator*(const vec3i& a, const R& b) { return vec3i(a.x*b, a.y*b, a.z*b); }
template<typename T, typename R> inline vec3i operator*(const R& a, const vec3i& b) { return vec3i(a*b.x, a*b.y, a*b.z); }
template<typename T, typename R> inline vec3i& operator*=(vec3i& a, const R& b) { a.x*=b; a.y*=b; a.z*=b; return a; }
inline vec3i operator/(const vec3i& a, int b) { return vec3i(a.x/b, a.y/b, a.z/b); }
inline vec3i operator/(int a, const vec3i& b) { return vec3i(a/b.x, a/b.y, a/b.z); }
inline vec3i& operator/=(vec3i& a, int b) { a.x/=b; a.y/=b; a.z/=b; return a; }
template<typename T, typename R> inline vec3i operator/(const vec3i& a, const R& b) { return vec3i(a.x/b, a.y/b, a.z/b); }
template<typename T, typename R> inline vec3i operator/(const R& a, const vec3i& b) { return vec3i(a/b.x, a/b.y, a/b.z); }
template<typename T, typename R> inline vec3i& operator/=(vec3i& a, const R& b) { a.x/=b; a.y/=b; a.z/=b; return a; }

// 3d component-wise functions ----------------------
// max, min, clamp
inline vec3i max(const vec3i& a, const vec3i& b) { return vec3i(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z)); }
inline vec3i max(const vec3i& a, int b) { return vec3i(max(a.x,b), max(a.y,b), max(a.z,b)); }
inline vec3i max(int a, const vec3i& b) { return vec3i(max(a,b.x), max(a,b.y), max(a,b.z)); }
inline vec3i min(const vec3i& a, const vec3i& b) { return vec3i(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z)); }
inline vec3i min(const vec3i& a, int b) { return vec3i(min(a.x,b), min(a.y,b), min(a.z,b)); }
inline vec3i min(int a, const vec3i& b) { return vec3i(min(a,b.x), min(a,b.y), min(a,b.z)); }
inline vec3i clamp(const vec3i& x, const vec3i& m, const vec3i& M) { return vec3i(clamp(x.x,m.x,M.x), clamp(x.y,m.y,M.y), clamp(x.z,m.z,M.z)); }
inline vec3i clamp(const vec3i& x, int m, int M) { return vec3i(clamp(x.x,m,M), clamp(x.y,m,M), clamp(x.z,m,M)); }
inline int max_component(const vec3i& a) { auto r = a.x; r = max(r,a.x); r = max(r,a.y); r = max(r,a.z);  return r; }
inline int min_component(const vec3i& a) { auto r = a.x; r = min(r,a.x); r = min(r,a.y); r = min(r,a.z);  return r; }

// 4d integer vector
struct vec4i {
    int x; // X component
    int y; // Y component
    int z; // Z component
    int w; // W component
    
    // Default constructor (set to zeros)
    vec4i() : x(0), y(0), z(0), w(0) { }
    // Element-setting constructor
    vec4i(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) { }

    // Element access
    int& operator[](int i) { return (&x)[i]; }
    // Element access
    int operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    int* begin() { return data(); }
    int* end() { return data()+size; }
    const int* begin() const { return data(); }
    const int* end() const { return data()+size; }
    
    // type system
    typedef int value_type;
    static const int size = 4;
    int* data() { return &x; }
    const int* data() const { return &x; }
};

// 4d vector Constants -------------------------------
// zero
const vec4i zero4i = vec4i(0, 0, 0, 0);
// one
const vec4i one4i = vec4i(1, 1, 1, 1);
// x, y, z, w
const vec4i x4i = vec4i(1, 0, 0, 0);
const vec4i y4i = vec4i(0, 1, 0, 0);
const vec4i z4i = vec4i(0, 0, 1, 0);
const vec4i w4i = vec4i(0, 0, 0, 1);

// 4d component-wise equality  -----------------------
inline bool operator==(const vec4i& a, const vec4i& b) { return a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w; }
inline bool operator!=(const vec4i& a, const vec4i& b) { return !operator==(a,b); }

// 4d lexical ordering  ------------------------------
inline bool operator<(const vec4i& a, const vec4i& b) { return a.x<b.x || (a.x==b.x && a.y<b.y) || (a.x==b.x && a.y==b.y && a.z<b.z) || (a.x==b.x && a.y==b.y && a.z==b.z && a.w<b.w); }

// 4d component-wise arithmetic operators -----------
inline vec4i operator-(const vec4i& a) { return vec4i(-a.x, -a.y, -a.z, -a.w); }
inline vec4i operator+(const vec4i& a, const vec4i& b) { return vec4i(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w); }
inline vec4i& operator+=(vec4i& a, const vec4i& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; a.w+=b.w; return a; }
inline vec4i operator-(const vec4i& a, const vec4i& b) { return vec4i(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w); }
inline vec4i& operator-=(vec4i& a, const vec4i& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; a.w-=b.w; return a; }
inline vec4i operator*(const vec4i& a, const vec4i& b) { return vec4i(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }
inline vec4i& operator*=(vec4i& a, const vec4i& b) { a.x*=b.x; a.y*=b.y; a.z*=b.z; a.w*=b.w; return a; }
inline vec4i operator/(const vec4i& a, const vec4i& b) { return vec4i(a.x/b.x, a.y/b.y, a.z/b.z, a.w/b.w); }
inline vec4i& operator/=(vec4i& a, const vec4i& b) { a.x/=b.x; a.y/=b.y; a.z/=b.z; a.w/=b.w; return a; }
inline vec4i operator*(const vec4i& a, int b) { return vec4i(a.x*b, a.y*b, a.z*b, a.w*b); }
inline vec4i operator*(int a, const vec4i& b) { return vec4i(a*b.x, a*b.y, a*b.z, a*b.w); }
inline vec4i& operator*=(vec4i& a, int b) { a.x*=b; a.y*=b; a.z*=b; a.w*=b; return a; }
template<typename T, typename R> inline vec4i operator*(const vec4i& a, const R& b) { return vec4i(a.x*b, a.y*b, a.z*b, a.w*b); }
template<typename T, typename R> inline vec4i operator*(const R& a, const vec4i& b) { return vec4i(a*b.x, a*b.y, a*b.z, a*b.w); }
template<typename T, typename R> inline vec4i& operator*=(vec4i& a, const R& b) { a.x*=b; a.y*=b; a.z*=b; a.w*=b; return a; }
inline vec4i operator/(const vec4i& a, int b) { return vec4i(a.x/b, a.y/b, a.z/b, a.w/b); }
inline vec4i operator/(int a, const vec4i& b) { return vec4i(a/b.x, a/b.y, a/b.z, a/b.w); }
inline vec4i& operator/=(vec4i& a, int b) { a.x/=b; a.y/=b; a.z/=b; a.w/=b; return a; }
template<typename T, typename R> inline vec4i operator/(const vec4i& a, const R& b) { return vec4i(a.x/b, a.y/b, a.z/b, a.w/b); }
template<typename T, typename R> inline vec4i operator/(const R& a, const vec4i& b) { return vec4i(a/b.x, a/b.y, a/b.z, a/b.w); }
template<typename T, typename R> inline vec4i& operator/=(vec4i& a, const R& b) { a.x/=b; a.y/=b; a.z/=b; a.w/=b; return a; }

// 4d component-wise functions ----------------------
// max, min, clamp
inline vec4i max(const vec4i& a, const vec4i& b) { return vec4i(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z), max(a.w,b.w)); }
inline vec4i max(const vec4i& a, int b) { return vec4i(max(a.x,b), max(a.y,b), max(a.z,b), max(a.w,b)); }
inline vec4i max(int a, const vec4i& b) { return vec4i(max(a,b.x), max(a,b.y), max(a,b.z), max(a,b.w)); }
inline vec4i min(const vec4i& a, const vec4i& b) { return vec4i(min(a.x,b.x), min(a.y,b.y), min(a.z,b.z), min(a.w,b.w)); }
inline vec4i min(const vec4i& a, int b) { return vec4i(min(a.x,b), min(a.y,b), min(a.z,b), min(a.w,b)); }
inline vec4i min(int a, const vec4i& b) { return vec4i(min(a,b.x), min(a,b.y), min(a,b.z), min(a,b.w)); }
inline vec4i clamp(const vec4i& x, const vec4i& m, const vec4i& M) { return vec4i(clamp(x.x,m.x,M.x), clamp(x.y,m.y,M.y), clamp(x.z,m.z,M.z), clamp(x.w,m.w,M.w)); }
inline vec4i clamp(const vec4i& x, int m, int M) { return vec4i(clamp(x.x,m,M), clamp(x.y,m,M), clamp(x.z,m,M), clamp(x.w,m,M)); }

// 3d coordinate frame
struct frame3f {
    vec3f o; // origin
    vec3f x; // x axis
    vec3f y; // y axis
    vec3f z; // z axis

    // default constructor (identity frame)
    frame3f() : o(0, 0, 0), x(1, 0, 0), y(0, 1, 0), z(0, 0, 1) { }
    // element-setting constructor
    frame3f(const vec3f& o, const vec3f& x, const vec3f& y, const vec3f& z) : o(o), x(x), y(y), z(z) { }
    // Vector constructor
    template<typename T> frame3f(const vector<T>& v) { error_if_not(v.size() == size, "bad length"); std::copy(v.begin(),v.end(),begin()); }
    
    // iterator access
    float* begin() { return data(); }
    float* end() { return data()+size; }
    const float* begin() const { return data(); }
    const float* end() const { return data()+size; }
    
    // type system
    typedef float value_type;
    static const int size = 12;
    float* data() { return &o.x; }
    const float* data() const { return &o.x; }
};


// frames constants ----------------------------------
// identity frame
const frame3f identity_frame3f = frame3f();

// component-wise equality --------------------------
inline bool operator==(const frame3f& a, const frame3f& b) { return a.o==b.o && a.x==b.x && a.y==b.y && a.z==b.z; }
inline bool operator!=(const frame3f& a, const frame3f& b) { return !operator==(a,b); }

// frames vector operations -------------------------
// inverse of a frame
inline frame3f inverse(const frame3f& f) { return frame3f(vec3f(-dot(f.o,f.x), -dot(f.o,f.y), -dot(f.o,f.z)), vec3f(f.x.x, f.y.x, f.z.x),vec3f(f.x.y, f.y.y, f.z.y),vec3f(f.x.z, f.y.z, f.z.z)); }

// orthogonalize (zxy order)
inline frame3f orthonormalize(const frame3f& f) { auto ret = f; ret.z = normalize(f.z); ret.x = orthonormalize(f.x,ret.z); ret.y = normalize(cross(ret.z,ret.x)); return ret; }
// creates a frame from z
inline frame3f frame_from_z(const vec3f& o, const vec3f& z) { auto f = identity_frame3f; f.o = o; f.z = normalize(z); f.x = normalize(orthogonal(f.z)); f.y = normalize(cross(f.z,f.x)); return f; }
inline frame3f frame_from_zx(const vec3f& o, const vec3f& x, const vec3f& z) { auto f = identity_frame3f; f.o = o; f.z = normalize(z); f.x = orthonormalize(f.x,f.z); f.y = normalize(cross(f.z,f.x)); return f; }

// frames creation ----------------------------------
// frame at eye that looks at center with vertical orientation as up (flipped changes z direction)
inline frame3f lookat_frame(const vec3f& eye, const vec3f& center, const vec3f& up, bool flipped = false) { auto f = frame3f(); f.o = eye; f.z = normalize(center-eye); if(flipped) f.z = -f.z; f.y = orthonormalize(up,f.z); f.x = normalize(cross(f.y,f.z)); return f; }

// frame-element transforms -------------------------
// transform a point by a frame
inline vec3f transform_point(const frame3f& f, const vec3f& v) { return f.o + f.x * v.x + f.y * v.y + f.z * v.z; }
// transform a vector by a frame
inline vec3f transform_vector(const frame3f& f, const vec3f& v) { return f.x * v.x + f.y * v.y + f.z * v.z; }
// transform a vector by a direciton
inline vec3f transform_direction(const frame3f& f, const vec3f& v) { return transform_vector(f,v); }
// transform a normal by a frame
inline vec3f transform_normal(const frame3f& f, const vec3f& v) { return transform_vector(f,v); }
// transform a frame by a frame
inline frame3f transform_frame(const frame3f& f, const frame3f& v) { return frame3f(transform_point(f,v.o), transform_vector(f,v.x), transform_vector(f,v.y), transform_vector(f,v.z)); }

// frame-element inverse transforms ------------------
// transform a point by a frame inverse
inline vec3f transform_point_inverse(const frame3f& f, const vec3f& v) { return vec3f(dot(v-f.o,f.x), dot(v-f.o,f.y), dot(v-f.o,f.z)); }
// transform a vector by a frame inverse
inline vec3f transform_vector_inverse(const frame3f& f, const vec3f& v) { return vec3f(dot(v,f.x), dot(v,f.y), dot(v,f.z)); }
// transform a direction by a frame inverse
inline vec3f transform_direction_inverse(const frame3f& f, const vec3f& v) { return transform_vector_inverse(f,v); }
// transform a normal by a frame inverse
inline vec3f transform_normal_inverse(const frame3f& f, const vec3f& v) { return transform_vector_inverse(f,v); }
// transform a frame by a frame inverse
inline frame3f transform_frame_inverse(const frame3f& f, const frame3f& v) { return frame3f(transform_point_inverse(f,v.o), transform_vector_inverse(f,v.x), transform_vector_inverse(f,v.y), transform_vector_inverse(f,v.z)); }

// 4x4 Matrix
struct mat4f {
    vec4f x, y, z, w; // rows

    // default constructor (identity matrix)
    mat4f() : x(1, 0, 0, 0), y(0, 1, 0, 0), z(0, 0, 1, 0), w(0, 0, 0, 1) { }
    // matrix from rows
    mat4f(const vec4f& x, const vec4f& y, const vec4f& z, const vec4f& w) : x(x), y(y), z(z), w(w) { }
    // matrix from elements
    mat4f(float x_x, float x_y, float x_z, float x_w , float y_x, float y_y, float y_z, float y_w , float z_x, float z_y, float z_z, float z_w , float w_x, float w_y, float w_z, float w_w ) : x(x_x, x_y, x_z, x_w), y(y_x, y_y, y_z, y_w), z(z_x, z_y, z_z, z_w), w(w_x, w_y, w_z, w_w) { }
    // Vector constructor
    template<typename T> mat4f(const vector<T>& v) { error_if_not(v.size() == size, "bad length"); std::copy(v.begin(),v.end(),begin()); }

    // row access
    vec4f& operator[](int i) { return (&x)[i]; }
    const vec4f& operator[](int i) const { return (&x)[i]; }
    
    // iterator access
    float* begin() { return data(); }
    float* end() { return data()+size; }
    const float* begin() const { return data(); }
    const float* end() const { return data()+size; }
    
    // type system
    typedef float value_type;
    static const int size = 16;
    float* data() { return &x.x; }
    const float* data() const { return &x.x; }
};

// matrix constants ----------------------------------
// identity matrix
const mat4f identity_mat4f = mat4f();

// component-wise equality --------------------------
inline bool operator==(const mat4f& a, const mat4f& b) { return a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w; }
inline bool operator!=(const mat4f& a, const mat4f& b) { return !operator==(a,b); }

// 4x4 linear algebra operations --------------------
inline mat4f operator-(const mat4f& a) { return mat4f(-a.x, -a.y, -a.z, -a.w); }
inline mat4f operator+(const mat4f& a, const mat4f& b) { return mat4f(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w); }
inline mat4f& operator+=(mat4f& a, const mat4f& b) { a.x+=b.x; a.y+=b.y; a.z+=b.z; a.w+=b.w; return a; }
inline mat4f operator-(const mat4f& a, const mat4f& b) { return mat4f(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w); }
inline mat4f& operator-=(mat4f& a, const mat4f& b) { a.x-=b.x; a.y-=b.y; a.z-=b.z; a.w-=b.w; return a; }
inline mat4f operator*(const mat4f& a, float b) { return mat4f(a.x*b, a.y*b, a.z*b, a.w*b); }
inline mat4f operator*(float a, const mat4f& b) { return mat4f(a*b.x, a*b.y, a*b.z, a*b.w); }
inline mat4f& operator*=(mat4f& a, float b) { a.x*=b; a.y*=b; a.z*=b; a.w*=b; return a; }
inline mat4f operator/(const mat4f& a, float b) { return mat4f(a.x/b, a.y/b, a.z/b, a.w/b); }
inline mat4f& operator/=(mat4f& a, float b) { a.x/=b; a.y/=b; a.z/=b; a.w/=b; return a; }

inline mat4f operator*(const mat4f& a, const mat4f& b) { return mat4f(a.x.x*b.x.x+a.x.y*b.y.x+a.x.z*b.z.x+a.x.w*b.w.x , a.x.x*b.x.y+a.x.y*b.y.y+a.x.z*b.z.y+a.x.w*b.w.y , a.x.x*b.x.z+a.x.y*b.y.z+a.x.z*b.z.z+a.x.w*b.w.z , a.x.x*b.x.w+a.x.y*b.y.w+a.x.z*b.z.w+a.x.w*b.w.w  , a.y.x*b.x.x+a.y.y*b.y.x+a.y.z*b.z.x+a.y.w*b.w.x , a.y.x*b.x.y+a.y.y*b.y.y+a.y.z*b.z.y+a.y.w*b.w.y , a.y.x*b.x.z+a.y.y*b.y.z+a.y.z*b.z.z+a.y.w*b.w.z , a.y.x*b.x.w+a.y.y*b.y.w+a.y.z*b.z.w+a.y.w*b.w.w  , a.z.x*b.x.x+a.z.y*b.y.x+a.z.z*b.z.x+a.z.w*b.w.x , a.z.x*b.x.y+a.z.y*b.y.y+a.z.z*b.z.y+a.z.w*b.w.y , a.z.x*b.x.z+a.z.y*b.y.z+a.z.z*b.z.z+a.z.w*b.w.z , a.z.x*b.x.w+a.z.y*b.y.w+a.z.z*b.z.w+a.z.w*b.w.w  , a.w.x*b.x.x+a.w.y*b.y.x+a.w.z*b.z.x+a.w.w*b.w.x , a.w.x*b.x.y+a.w.y*b.y.y+a.w.z*b.z.y+a.w.w*b.w.y , a.w.x*b.x.z+a.w.y*b.y.z+a.w.z*b.z.z+a.w.w*b.w.z , a.w.x*b.x.w+a.w.y*b.y.w+a.w.z*b.z.w+a.w.w*b.w.w  ); }
inline mat4f& operator*=(mat4f& a, const mat4f& b) { a = a*b; return a; }
inline vec4f operator*(const mat4f& a, const vec4f& b) { return vec4f(dot(a.x,b), dot(a.y,b), dot(a.z,b), dot(a.w,b)); }

// 4x4 matrix operations ----------------------------
inline mat4f transpose(const mat4f& a) { return mat4f(a.x.x, a.y.x, a.z.x, a.w.x , a.x.y, a.y.y, a.z.y, a.w.y , a.x.z, a.y.z, a.z.z, a.w.z , a.x.w, a.y.w, a.z.w, a.w.w ); }
inline float determinant(const mat4f& a) { return +a.x.x*(+a.y.y*(a.z.z*a.w.w-a.z.w*a.w.z)-a.y.z*(a.z.y*a.w.w-a.z.w*a.w.y)+a.y.w*(a.z.y*a.w.z-a.z.z*a.w.y))-a.x.y*(+a.y.x*(a.z.z*a.w.w-a.z.w*a.w.z)-a.y.z*(a.z.x*a.w.w-a.z.w*a.w.x)+a.y.w*(a.z.x*a.w.z-a.z.z*a.w.x))+a.x.z*(+a.y.x*(a.z.y*a.w.w-a.z.w*a.w.y)-a.y.y*(a.z.x*a.w.w-a.z.w*a.w.x)+a.y.w*(a.z.x*a.w.y-a.z.y*a.w.x))-a.x.w*(+a.y.x*(a.z.y*a.w.z-a.z.z*a.w.y)-a.y.y*(a.z.x*a.w.z-a.z.z*a.w.x)+a.y.z*(a.z.x*a.w.y-a.z.y*a.w.x)); }
inline mat4f inverse(const mat4f& a) { float d = determinant(a); float di = 1/d; return mat4f(+(+a.y.y*(a.z.z*a.w.w-a.z.w*a.w.z)-a.y.z*(a.z.y*a.w.w-a.z.w*a.w.y)+a.y.w*(a.z.y*a.w.z-a.z.z*a.w.y)), -(+a.x.y*(a.z.z*a.w.w-a.z.w*a.w.z)-a.x.z*(a.z.y*a.w.w-a.z.w*a.w.y)+a.x.w*(a.z.y*a.w.z-a.z.z*a.w.y)), +(+a.x.y*(a.y.z*a.w.w-a.y.w*a.w.z)-a.x.z*(a.y.y*a.w.w-a.y.w*a.w.y)+a.x.w*(a.y.y*a.w.z-a.y.z*a.w.y)), -(+a.x.y*(a.y.z*a.z.w-a.y.w*a.z.z)-a.x.z*(a.y.y*a.z.w-a.y.w*a.z.y)+a.x.w*(a.y.y*a.z.z-a.y.z*a.z.y)), -(+a.y.x*(a.z.z*a.w.w-a.z.w*a.w.z)-a.y.z*(a.z.x*a.w.w-a.z.w*a.w.x)+a.y.w*(a.z.x*a.w.z-a.z.z*a.w.x)), +(+a.x.x*(a.z.z*a.w.w-a.z.w*a.w.z)-a.x.z*(a.z.x*a.w.w-a.z.w*a.w.x)+a.x.w*(a.z.x*a.w.z-a.z.z*a.w.x)), -(+a.x.x*(a.y.z*a.w.w-a.y.w*a.w.z)-a.x.z*(a.y.x*a.w.w-a.y.w*a.w.x)+a.x.w*(a.y.x*a.w.z-a.y.z*a.w.x)), +(+a.x.x*(a.y.z*a.z.w-a.y.w*a.z.z)-a.x.z*(a.y.x*a.z.w-a.y.w*a.z.x)+a.x.w*(a.y.x*a.z.z-a.y.z*a.z.x)), +(+a.y.x*(a.z.y*a.w.w-a.z.w*a.w.y)-a.y.y*(a.z.x*a.w.w-a.z.w*a.w.x)+a.y.w*(a.z.x*a.w.y-a.z.y*a.w.x)), -(+a.x.x*(a.z.y*a.w.w-a.z.w*a.w.y)-a.x.y*(a.z.x*a.w.w-a.z.w*a.w.x)+a.x.w*(a.z.x*a.w.y-a.z.y*a.w.x)), +(+a.x.x*(a.y.y*a.w.w-a.y.w*a.w.y)-a.x.y*(a.y.x*a.w.w-a.y.w*a.w.x)+a.x.w*(a.y.x*a.w.y-a.y.y*a.w.x)), -(+a.x.x*(a.y.y*a.z.w-a.y.w*a.z.y)-a.x.y*(a.y.x*a.z.w-a.y.w*a.z.x)+a.x.w*(a.y.x*a.z.y-a.y.y*a.z.x)), -(+a.y.x*(a.z.y*a.w.z-a.z.z*a.w.y)-a.y.y*(a.z.x*a.w.z-a.z.z*a.w.x)+a.y.z*(a.z.x*a.w.y-a.z.y*a.w.x)), +(+a.x.x*(a.z.y*a.w.z-a.z.z*a.w.y)-a.x.y*(a.z.x*a.w.z-a.z.z*a.w.x)+a.x.z*(a.z.x*a.w.y-a.z.y*a.w.x)), -(+a.x.x*(a.y.y*a.w.z-a.y.z*a.w.y)-a.x.y*(a.y.x*a.w.z-a.y.z*a.w.x)+a.x.z*(a.y.x*a.w.y-a.y.y*a.w.x)), +(+a.x.x*(a.y.y*a.z.z-a.y.z*a.z.y)-a.x.y*(a.y.x*a.z.z-a.y.z*a.z.x)+a.x.z*(a.y.x*a.z.y-a.y.y*a.z.x)))*di; }

// transform tests ----------------------------------
// tests whether a matrix is affine (approximately)
inline bool isaffine(const mat4f& m) { return m.w.x==0 && m.w.y==0 && m.w.z==0 && m.w.w==1; }

// transform matrices -------------------------------
// translation matrix
inline mat4f translation_matrix(const vec3f& t) { return mat4f(1,0,0,t.x,  0,1,0,t.y,  0,0,1,t.z,  0,0,0,1); }
// rotation matrix
inline mat4f rotation_matrix(float angle, const vec3f& axis) { float c = cos(angle); float s = sin(angle); vec3f vv = normalize(axis); return mat4f(c + (1-c)*vv.x*vv.x, (1-c)*vv.x*vv.y - s*vv.z, (1-c)*vv.x*vv.z + s*vv.y, 0,  (1-c)*vv.x*vv.y + s*vv.z, c + (1-c)*vv.y*vv.y, (1-c)*vv.y*vv.z - s*vv.x, 0,  (1-c)*vv.x*vv.z - s*vv.y, (1-c)*vv.y*vv.z + s*vv.x, c + (1-c)*vv.z*vv.z, 0,  0,0,0,1); }
// scaling matrix
inline mat4f scaling_matrix(const vec3f& s) { return mat4f(s.x,0,0,0,  0,s.y,0,0,  0,0,s.z,0,  0,0,0,1); }
// opengl frustum matrix
inline mat4f frustum_matrix(float l, float r, float b, float t, float n, float f) { return mat4f(2*n/(r-l), 0, (r+l)/(r-l), 0,  0, 2*n/(t-b), (t+b)/(t-b), 0,  0, 0, -(f+n)/(f-n), -2*f*n/(f-n), 0, 0, -1, 0); }
// opengl ortho matrix
inline mat4f ortho_matrix(float l, float r, float b, float t, float n, float f) { return mat4f(2/(r-l), 0, 0, -(r+l)/(r-l),  0, 2/(t-b), 0, -(t+b)/(t-b),  0, 0, -2/(f-n), -(f+n)/(f-n), 0, 0, 0, 1); }

// euler angles -------------------------------
inline vec3f euler_angles(const mat4f& m) {
    auto rot = zero3f;
    rot.x = atan2(m.z.y,m.z.z);
    rot.y = atan2(-m.z.x,sqrt(m.z.y*m.z.y+m.z.z*m.z.z));
    rot.z = atan2(m.y.x,m.x.x);
    return rot;
}
inline mat4f euler_matrix(const vec3f& rot) { return rotation_matrix(rot.z, z3f) * rotation_matrix(rot.y, y3f) * rotation_matrix(rot.x, x3f); }

// lookat matrix
inline mat4f lookat_matrix(const vec3f& eye, const vec3f& center, const vec3f& up) { vec3f w = normalize(eye - center); vec3f u = normalize(cross(up,w)); vec3f v = cross(w,u); return mat4f(u.x, u.y, u.z, -dot(u,eye),  v.x, v.y, v.z, -dot(v,eye),  w.x, w.y, w.z, -dot(w,eye),  0, 0, 0, 1); }
// opengl glu ortho2d matrix
inline mat4f ortho2d_matrix(float l, float r, float b, float t) { return ortho_matrix(l,r,b,t,-1,1); }
// opengl glu perspective matrix
inline mat4f perspective_matrix(float fovy, float aspect, float near, float far) { float f = 1/tan(fovy/2); return mat4f(f/aspect,0,0,0,   0,f,0,0,    0,0,(far+near)/(near-far),2*far*near/(near-far),    0,0,-1,0); }

// matrix-element transforms ------------------------
inline vec3f transform_point(const mat4f& m, const vec3f& v) { auto tv = m * vec4f(v.x,v.y,v.z,1); return vec3f(tv.x,tv.y,tv.z) / tv.w; }
inline vec3f transform_vector(const mat4f& m, const vec3f& v) { auto tv = m * vec4f(v.x,v.y,v.z,0); return vec3f(tv.x,tv.y,tv.z); }
inline vec3f transform_direction(const mat4f& m, const vec3f& v) { return normalize(transform_vector(m,v)); }
// requires inverse transform
inline vec3f transform_normal(const mat4f& m, const vec3f& v) { return normalize(transform_vector(m,v)); }
inline frame3f transform_frame(const mat4f& m, const frame3f& v) { frame3f ret; ret.o = transform_point(m,v.o); ret.x = transform_direction(m,v.x); ret.y = transform_direction(m,v.y); ret.z = cross(ret.x,ret.y); ret = orthonormalize(ret); return ret; }

// frame-matrix conversion --------------------------
inline mat4f frame_to_matrix(const frame3f& f) { return mat4f(f.x.x, f.y.x, f.z.x,f.o.x, f.x.y, f.y.y, f.z.y,f.o.y, f.x.z, f.y.z, f.z.z,f.o.z, 0,0,0,1); }
inline mat4f frame_to_matrix_inverse(const frame3f& f) { return mat4f(f.x.x, f.x.y, f.x.z, -dot(f.o,f.x), f.y.x, f.y.y, f.y.z, -dot(f.o,f.y), f.z.x, f.z.y, f.z.z, -dot(f.o,f.z), 0,0,0,1); }
inline frame3f matrix_to_frame(const mat4f& m) { error_if_not(isaffine(m), "not affine"); frame3f f; f.o = vec3f(m.x.w, m.y.w, m.z.w); f.x = vec3f(m.x.x, m.y.x, m.z.x); f.y = vec3f(m.x.y, m.y.y, m.z.y); f.z = vec3f(m.x.z, m.y.z, m.z.z); return f; }

// 1D Bounding Interval
struct range1f {
    float min; // min value
    float max; // max value

    // Default constructor (set to invalid)
    range1f() : min(+1), max(-1) { }
    // Element-wise constructor
    range1f(float min, float max) : min(min), max(max) { }
    
    // type system
    typedef float value_type;
    static const int size = 2;
};

// 1d component-wise equality  -----------------------
inline bool operator==(const range1f& a, const range1f& b) { return a.min==b.min && a.max==b.max; }
inline bool operator!=(const range1f& a, const range1f& b) { return !operator==(a,b); }

// 1d bounding interval operatons  -------------------
inline bool isvalid(const range1f& a) { return min(a.min,a.max) == a.min; }
inline float size(const range1f& a) { return (a.max-a.min); }
inline float center(const range1f& a) { return (a.max+a.min)/2; }
inline range1f runion(const range1f& a, float b) { if(!isvalid(a)) return range1f(b,b); return range1f(min(a.min,b),max(a.max,b)); }
inline range1f runion(const range1f& a, const range1f& b) { if(!isvalid(a)) return b; if(!isvalid(b)) return a; return range1f(min(a.min,b.min),max(a.max,b.max)); }
inline range1f rscale(const range1f& a, float b) { return range1f(center(a)-size(a)*b/2,center(a)+size(a)*b/2); }

// 2D Bounding Box
struct range2f {
    vec2f min; // min corner
    vec2f max; // max corner
    
    // Default constructor (set to invalid)
    range2f() : min(+1,+1), max(-1,-1) { }
    // Element-wise constructor
    range2f(const vec2f& min, const vec2f& max) : min(min), max(max) { }
    
    // type system
    typedef float value_type;
    static const int size = 4;
    float* data() { return &min.x; }
    const float* data() const { return &min.x; }
};

// 3d component-wise equality  -----------------------
inline bool operator==(const range2f& a, const range2f& b) { return a.min==b.min && a.max==b.max; }
inline bool operator!=(const range2f& a, const range2f& b) { return !operator==(a,b); }

// 3d bounding interval operatons  -------------------
inline bool isvalid(const range2f& a) { return min(a.min,a.max) == a.min; }
inline vec2f size(const range2f& a) { return (a.max-a.min); }
inline vec2f center(const range2f& a) { return (a.max+a.min)/2; }
inline range2f runion(const range2f& a, const vec2f& b) { if(!isvalid(a)) return range2f(b,b); return range2f(min(a.min,b),max(a.max,b)); }
inline range2f runion(const range2f& a, const range2f& b) { if(!isvalid(a)) return b; if(!isvalid(b)) return a; return range2f(min(a.min,b.min),max(a.max,b.max)); }
inline range2f rscale(const range2f& a, float b) { return range2f(center(a)-size(a)*b/2,center(a)+size(a)*b/2); }

// 3D Bounding Box
struct range3f {
    vec3f min; // min corner
    vec3f max; // max corner

    // Default constructor (set to invalid)
    range3f() : min(+1,+1,+1), max(-1,-1,-1) { }
    // Element-wise constructor
    range3f(const vec3f& min, const vec3f& max) : min(min), max(max) { }
    
    // type system
    typedef float value_type;
    static const int size = 6;
    float* data() { return &min.x; }
    const float* data() const { return &min.x; }
};

// 3d component-wise equality  -----------------------
inline bool operator==(const range3f& a, const range3f& b) { return a.min==b.min && a.max==b.max; }
inline bool operator!=(const range3f& a, const range3f& b) { return !operator==(a,b); }

// 3d bounding interval operatons  -------------------
inline bool isvalid(const range3f& a) { return min(a.min,a.max) == a.min; }
inline vec3f size(const range3f& a) { return (a.max-a.min); }
inline vec3f center(const range3f& a) { return (a.max+a.min)/2; }
inline range3f runion(const range3f& a, const vec3f& b) { if(!isvalid(a)) return range3f(b,b); return range3f(min(a.min,b),max(a.max,b)); }
inline range3f runion(const range3f& a, const range3f& b) { if(!isvalid(a)) return b; if(!isvalid(b)) return a; return range3f(min(a.min,b.min),max(a.max,b.max)); }
inline range3f radd(const range3f& a, float b) { return range3f(a.min-vec3f(b,b,b),a.max+vec3f(b,b,b)); }
inline range3f rscale(const range3f& a, float b) { return range3f(center(a)-size(a)*b/2,center(a)+size(a)*b/2); }
inline range3f make_range3f(const std::initializer_list<vec3f>& points) { auto bbox = range3f(); for(auto& p : points) bbox = runion(bbox,p); return bbox; }
inline range3f make_range3f(const std::vector<vec3f>& points) { auto bbox = range3f(); for(auto& p : points) bbox = runion(bbox,p); return bbox; }
inline std::array<vec3f,8> corners(const range3f& a) { std::array<vec3f,8> ret; ret[0] = vec3f(a.min.x,a.min.y,a.min.z); ret[1] = vec3f(a.min.x,a.min.y,a.max.z); ret[2] = vec3f(a.min.x,a.max.y,a.min.z); ret[3] = vec3f(a.min.x,a.max.y,a.max.z); ret[4] = vec3f(a.max.x,a.min.y,a.min.z); ret[5] = vec3f(a.max.x,a.min.y,a.max.z); ret[6] = vec3f(a.max.x,a.max.y,a.min.z); ret[7] = vec3f(a.max.x,a.max.y,a.max.z); return ret; }

// 1D Bounding Interval
struct range1i {
    int min; // min value
    int max; // max value
    
    // Default constructor (set to invalid)
    range1i() : min(+1), max(-1) { }
    // Element-wise constructor
    range1i(int min, int max) : min(min), max(max) { }
    
    // type system
    typedef int value_type;
    static const int size = 2;
};

// 1d component-wise equality  -----------------------
inline bool operator==(const range1i& a, const range1i& b) { return a.min==b.min && a.max==b.max; }
inline bool operator!=(const range1i& a, const range1i& b) { return !operator==(a,b); }

// 1d bounding interval operatons  -------------------
inline bool isvalid(const range1i& a) { return min(a.min,a.max) == a.min; }
inline int size(const range1i& a) { return (a.max-a.min); }
inline int center(const range1i& a) { return (a.max+a.min)/2; }
inline range1i runion(const range1i& a, int b) { if(!isvalid(a)) return range1i(b,b); return range1i(min(a.min,b),max(a.max,b)); }
inline range1i range2fion(const range1i& a, const range1i& b) { if(!isvalid(a)) return b; if(!isvalid(b)) return a; return range1i(min(a.min,b.min),max(a.max,b.max)); }

// 2D Bounding Box
struct range2i {
    vec2i min; // min corner
    vec2i max; // max corner
    
    // Default constructor (set to invalid)
    range2i() : min(+1,+1), max(-1,-1) { }
    // Element-wise constructor
    range2i(const vec2i& min, const vec2i& max) : min(min), max(max) { }
    
    // type system
    typedef int value_type;
    static const int size = 6;
    int* data() { return &min.x; }
    const int* data() const { return &min.x; }
};

// 3d component-wise equality  -----------------------
inline bool operator==(const range2i& a, const range2i& b) { return a.min==b.min && a.max==b.max; }
inline bool operator!=(const range2i& a, const range2i& b) { return !operator==(a,b); }

// 3d bounding interval operatons  -------------------
inline bool isvalid(const range2i& a) { return min(a.min,a.max) == a.min; }
inline vec2i size(const range2i& a) { return (a.max-a.min); }
inline vec2i center(const range2i& a) { return (a.max+a.min)/2; }
inline range2i runion(const range2i& a, const vec2i& b) { if(!isvalid(a)) return range2i(b,b); return range2i(min(a.min,b),max(a.max,b)); }
inline range2i runion(const range2i& a, const range2i& b) { if(!isvalid(a)) return b; if(!isvalid(b)) return a; return range2i(min(a.min,b.min),max(a.max,b.max)); }

// 3D Bounding Box
struct range3i {
    vec3i min; // min corner
    vec3i max; // max corner
    
    // Default constructor (set to invalid)
    range3i() : min(+1,+1,+1), max(-1,-1,-1) { }
    // Element-wise constructor
    range3i(const vec3i& min, const vec3i& max) : min(min), max(max) { }
    
    // type system
    typedef int value_type;
    static const int size = 6;
    int* data() { return &min.x; }
    const int* data() const { return &min.x; }
};

// 3d component-wise equality  -----------------------
inline bool operator==(const range3i& a, const range3i& b) { return a.min==b.min && a.max==b.max; }
inline bool operator!=(const range3i& a, const range3i& b) { return !operator==(a,b); }

// 3d bounding interval operatons  -------------------
inline bool isvalid(const range3i& a) { return min(a.min,a.max) == a.min; }
inline vec3i size(const range3i& a) { return (a.max-a.min); }
inline vec3i center(const range3i& a) { return (a.max+a.min)/2; }
inline range3i runion(const range3i& a, const vec3i& b) { if(!isvalid(a)) return range3i(b,b); return range3i(min(a.min,b),max(a.max,b)); }
inline range3i runion(const range3i& a, const range3i& b) { if(!isvalid(a)) return b; if(!isvalid(b)) return a; return range3i(min(a.min,b.min),max(a.max,b.max)); }

// 3d ray constants
const float ray3f_epsilon = 0.0005f;
const float ray3f_rayinf = 1000000.0f;

// 3D Ray
struct ray3f {
    vec3f e;        // origin
    vec3f d;        // direction
    float tmin;     // min t value
    float tmax;     // max t value
    
    // Default constructor
    ray3f() : e(zero3f), d(z3f), tmin(ray3f_epsilon), tmax(ray3f_rayinf) { }
    
    // Element-wise constructor
    ray3f(const vec3f& e, const vec3f& d) :
    e(e), d(d), tmin(ray3f_epsilon), tmax(ray3f_rayinf) { }
    
    // Element-wise constructor
    ray3f(const vec3f& e, const vec3f& d, float tmin, float tmax) :
    e(e), d(d), tmin(tmin), tmax(tmax) { }
    
    // Eval ray at a specific t
    vec3f eval(float t) const { return e + d * t; }
    
    // Create a ray from a segment
    static ray3f make_segment(const vec3f& a, const vec3f& b) { return ray3f(a,normalize(b-a),ray3f_epsilon,dist(a,b)-2*ray3f_epsilon); }
};

// transform a ray by a frame
inline ray3f transform_ray(const frame3f& f, const ray3f& v) { return ray3f(transform_point(f,v.e), transform_vector(f,v.d), v.tmin, v.tmax); }
// transform a ray by a frame inverse
inline ray3f transform_ray_inverse(const frame3f& f, const ray3f& v) { return ray3f(transform_point_inverse(f,v.e),transform_vector_inverse(f,v.d),v.tmin,v.tmax); }

// functions used for ambient occlusion
inline float randf() { return std::rand() / float(RAND_MAX); }
inline float rand_in_range(float a, float b) { float random = std::rand() / float(RAND_MAX); float diff = b - a; random *= diff; return a + random; }
inline float lerp(float a, float b, float t) { return (1 - t) * a + t * b; }

#endif
