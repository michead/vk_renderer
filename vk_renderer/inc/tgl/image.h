#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "common.h"
#include "vmath.h"

// a generic image
template<typename T>
struct image {
    // Default Constructor (empty image)
    image() : _w(0), _h(0) { }
    // Size Constructor with initialization (sets width and height and initialize pixels)
    image(int w, int h, const T& v = T()) : _w(w), _h(h), _d(_w*_h,v) { }
    // Size Constructor with initialization (sets width and height and initialize pixels)
    image(const vec2i& s, const T& v = T()) : image(s.x,s.y,v) { }
    // Size Constructor with initialization (sets width and height and initialize pixels)
    image(int w, int h, const vector<T>& v) : _w(w), _h(h), _d(v) { assert(w*h == v.size()); }
    // Size Constructor with initialization (sets width and height and initialize pixels)
    image(const vec2i& s, const vector<T>& v) : image(s.x,s.y,v) { assert(s.x*s.y == v.size()); }
    
    // resize image clearing its content
    void resize(int w, int h, const T& v = T()) { _w = w; _h = h; _d.assign(w*h, v); }
    // resize image clearing its content
    void resize(const vec2i& s, const T& v = T()) { resize(s.x,s.y,v); }
    
    // assigning constant values
    void assign(const T& v) { for(auto& p : _d) p = v; }
    
    // image width
    int width() const { return _w; }
    // image height
    int height() const { return _h; }
    // image size
    vec2i size() const { return {_w,_h}; }
    // number of pixels
    int npixels() const { return _w*_h; }
    
    // element access
    T& at(int i, int j) { return _d[j*_w+i]; }
    // element access
    const T& at(int i, int j) const { return _d[j*_w+i]; }
    // element access
    T& at(const vec2i& ij) { return _d[ij.y*_w+ij.x]; }
    // element access
    const T& at(const vec2i& ij) const { return _d[ij.y*_w+ij.x]; }
    // element access
    T& operator[](const vec2i& ij) { return _d[ij.y*_w+ij.x]; }
    // element access
    const T& operator[](const vec2i& ij) const { return _d[ij.y*_w+ij.x]; }
    
    // data access
    T* data() { return _d.data(); }
    // data access
    const T* data() const { return _d.data(); }
    
    // set a subimage
    void set(const image<T>& img, int x, int y) {
        for(auto j = 0; j < img.height(); j ++) {
            for(auto i = 0; i < img.width(); i ++) {
                at(i+x, j+y) = img.at(i,j);
            }
        }
    }
    
    // flips this image along the y axis returning a new image
    image<T> flipy() const {
        image<T> ret(width(),height());
        for(int j = 0; j < height(); j ++) {
            for(int i = 0; i < width(); i ++) {
                ret.at(i,height()-1-j) = at(i,j);
            }
        }
        return ret;
    }
private:
    int _w, _h;
    vector<T> _d;

};

// common image cases
using image3f = image<vec3f>;
using image4f = image<vec4f>;

// write an image to file with the format specified (8bit formats have gamma applied)
void save_img(const string& filename, const image3f& img, bool flipped, bool default_gamma = false);
// load an image to file with the format specified (8bit formats are ungamma-ed)
image3f load_img(const string& filename, bool flipped, bool default_gamma = false);

// scales an image with fast box filter
image3f upsize_image(const image3f& img, int scale);
image3f downsize_image(const image3f& img, int scale);

// apply an image gamma
image3f gamma_image(const image3f& img, float gamma = 2.2f);
image3f ungamma_image(const image3f& img, float gamma = 2.2f);

//appy whitebalance
image3f wb_image(const image3f& img, vec3f perc_white);


#if 0
// Write an floating point color PFM image file
void write_pfm(const string& filename, const image3f& img, bool flipY = false);
// Write an 8-bit color compressed PNG file (sets PNG alpha to 1 everywhere)
void write_png(const string& filename, const image3f& img, bool flipY = false);

// Load a PFM or PPM color image and return it as a floating point color image
image3f read_pnm(const string& filename, bool flipY);
// Load a compressed PNG color image and return it as a floating point color image
image3f read_png(const string& filename, bool flipY);
#endif

#endif
