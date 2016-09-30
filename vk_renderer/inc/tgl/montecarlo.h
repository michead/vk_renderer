#ifndef _MONTECARLO_H_
#define _MONTECARLO_H_

#include "vmath.h"

#include <random>
#include "sobol.h"

// create a sequence of seeds
inline vector<unsigned int> make_seeds(int n, const vector<unsigned int>& start_seeds = {0,1,2,3,4,5,6,7,8,9}) {
    std::seed_seq sseq(start_seeds.begin(), start_seeds.end());
    auto seeds = std::vector<unsigned int>(n);
    sseq.generate(seeds.begin(), seeds.end());
    return seeds;
}

// hashing (http://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key?s=1|4.3847)
// this is a perfect hash
inline unsigned int hash(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x);
    return x;
}

// hashing
// another has function - maybe better then previous one
#if 0
inline uint32_t hash( uint32_t a) {
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}
#endif

// sobol sequence wrapper (limited to 1024 dimension)
inline float sobol(unsigned long long index, const unsigned dimension, const unsigned scramble = 0U) {
    return sobolimpl::sample(index, dimension, scramble);
}

// correlated multi-jittered sampling from Pixar Technical Memo ---
inline unsigned cmj_permute(unsigned i, unsigned l, unsigned p) {
    unsigned w = l - 1;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    do {
        i ^= p;
        i *= 0xe170893d;
        i ^= p >> 16;
        i ^= (i & w) >> 4;
        i ^= p >> 8;
        i *= 0x0929eb3f;
        i ^= p >> 23;
        i ^= (i & w) >> 1;
        i *= 1 | p >> 27;
        i *= 0x6935fa69;
        i ^= (i & w) >> 11;
        i *= 0x74dcb303;
        i ^= (i & w) >> 2;
        i *= 0x9e501cc3;
        i ^= (i & w) >> 2;
        i *= 0xc860a3df;
        i &= w;
        i ^= i >> 5;
    } while (i >= l);
    return (i + p) % l;
}

// hash function
inline unsigned cmj_hash(unsigned i, unsigned p) {
    i ^= p;
    i ^= i >> 17;
    i ^= i >> 10;
    i *= 0xb36534e5;
    i ^= i >> 12;
    i ^= i >> 21;
    i *= 0x93fc4795;
    i ^= 0xdf6e307f;
    i ^= i >> 17;
    i *= 1 | p >> 18;
    return i;
}

// hashed float
inline  float cmj_randfloat(unsigned i, unsigned p) {
    return cmj_hash(i, p) * (1.0f / 4294967808.0f);
}

// pick the s-th float from a set of N with pattern p
inline float cmj_sample_float(unsigned s, unsigned N, unsigned p) {
    unsigned x = cmj_permute(s, N, p * 0x68bc21eb);
    float jx = cmj_randfloat(s, p * 0x967a889b);
    return (x + jx)/N;
}

// pick the s-th float from a 2D set of N with pattern p
inline vec2f cmj_sample_vec2f(unsigned s, unsigned N, unsigned p) {
    unsigned m = static_cast<unsigned>(sqrtf(N));
    unsigned n = (N + m - 1)/m;
    s = cmj_permute(s, N, p * 0x51633e2d);
    unsigned sx = cmj_permute(s % m, m, p * 0x68bc21eb);
    unsigned sy = cmj_permute(s / m, n, p * 0x02e5be93);
    float jx = cmj_randfloat(s, p * 0x967a889b);
    float jy = cmj_randfloat(s, p * 0x368cc8b7);
    return {(sx + (sy + jx)/n)/m, (s + jy)/N};
}
// end of correlated multi-jittered sampling from Pixar Technical Memo ---

// Random number generator
struct rng {
    // C++11 random number engine
    std::minstd_rand                        engine;
    
    // initiallize the generator
    rng() { }
    // initiallize the seeded generator
    rng(unsigned int seed) { engine.seed(seed); }
    
    // Seed the generator
    void seed(unsigned int seed) { engine.seed(seed); }
	
    // Generate a float in [0,1)
	float next_float() { return std::uniform_real_distribution<float>(0,1)(engine); }
    // Generate a float in [v.x,v.y)
	float next_float(const vec2f& v) { return std::uniform_real_distribution<float>(v.x,v.y)(engine); }
    
	// Generate 2 floats in [0,1)^2
    vec2f next_vec2f() { return vec2f(next_float(),next_float()); }
	// Generate 3 floats in [0,1)^3
    vec3f next_vec3f() { return vec3f(next_float(),next_float(),next_float()); }
};

// hemispherical direction with uniform distribution
inline vec3f sample_direction_hemispherical_uniform(const vec2f& ruv) {
    auto z = ruv.y;
    auto r = sqrt(1-z*z);
    auto phi = 2 * pi * ruv.x;
    return vec3f(r*cos(phi), r*sin(phi), z);
}

// pdf for hemispherical direction with uniform distribution
inline float sample_direction_hemispherical_uniform_pdf(const vec3f& w) {
    return (w.z <= 0) ? 0 : 1/(2*pi);
}

// spherical direction with uniform distribution
inline vec3f sample_direction_spherical_uniform(const vec2f ruv) {
    auto z = 2*ruv.y-1;
    auto r = sqrt(1-z*z);
    auto phi = 2 * pi * ruv.x;
    return vec3f(r*cos(phi), r*sin(phi), z);
}

// pdf for spherical direction with uniform distribution
inline float sample_direction_spherical_uniform_pdf(const vec3f& w) {
    return 1/(4*pi);
}

// hemispherical direction with cosine distribution
inline vec3f sample_direction_hemispherical_cosine(const vec2f& ruv) {
    auto z = sqrt(ruv.y);
    auto r = sqrt(1-z*z);
    auto phi = 2 * pi * ruv.x;
    return vec3f(r*cos(phi), r*sin(phi), z);
}

// pdf for hemispherical direction with cosine distribution
inline float sample_direction_hemispherical_cosine_pdf(const vec3f& w) {
    return (w.z <= 0) ? 0 : w.z/pi;
}

// hemispherical direction with cosine power distribution
inline vec3f sample_direction_hemispherical_cospower(const vec2f& ruv, float n) {
    auto z = pow(ruv.y,1/(n+1));
    auto r = sqrt(1-z*z);
    auto phi = 2 * pi * ruv.x;
    return vec3f(r*cos(phi), r*sin(phi), z);
}

// pdf for hemispherical direction with cosine power distribution
inline float sample_direction_hemispherical_cospower_pdf(const vec3f& w, float n) {
    return (w.z <= 0) ? 0 : pow(w.z,n) * (n+1) / (2*pi);
}

// uniform disk
inline vec3f sample_uniform_disk(const vec2f& ruv) {
    auto r = sqrt(ruv.y);
    auto phi = 2 * pi * ruv.x;
    return vec3f(cos(phi)*r, sin(phi)*r, 0);
}

// pdf for uniform disk
inline float sample_uniform_disk_pdf() {
    return 1/pi;
}

// uniform cylinder
inline vec3f sample_uniform_cylinder(const vec2f& ruv) {
    auto phi = 2 * pi * ruv.x;
    return vec3f(sin(phi), cos(phi), ruv.y*2-1);
}

// pdf for uniform cylinder
inline float sample_uniform_cylinder_pdf() {
    return 1 / pi;
}

// uniform triangle
inline vec2f sample_uniform_triangle(const vec2f& ruv) {
    return {1-sqrt(ruv.x),ruv.y*sqrt(ruv.x)};
}

// uniform triangle
inline vec3f sample_uniform_triangle(const vec2f& ruv, const vec3f& v0, const vec3f& v1, const vec3f& v2) {
    auto uv = sample_uniform_triangle(ruv);
    return v0*uv.x + v1*uv.y + v2*(1-uv.x-uv.y);
}

// pdf for uniform triangle (triangle area)
inline float sample_uniform_triangle_pdf(const vec3f& v0, const vec3f& v1, const vec3f& v2) {
    return 2/length(cross(v1 - v0, v2 - v0));
}

// index with uniform distribution
inline int sample_index_uniform(float r, int size) {
    return clamp((int)(r * size), 0, size-1);
}

// pdf for index with uniform distribution
inline float sample_index_uniform_pdf(int size) {
    return 1.0f / size;
}

// computes the sample number in each dimension for stratified sampling
inline int sample_stratify_samplesnumber(int samples) {
    return (int)round(sqrt(samples));
}

// computes a sample for stratified sampling
inline vec2f sample_stratify_sample(const vec2f& uv, int sample, int samples_x, int samples_y) {
    int sample_x = sample % samples_x;
    int sample_y = sample / samples_x;
    return vec2f((sample_x + uv.x / samples_x), (sample_y + uv.y / samples_y));
}

// power distribution heuristics
inline float sample_power_heuristics(float fPdf, float gPdf) {
    return (fPdf*fPdf) / (fPdf*fPdf + gPdf*gPdf);
}

// 1d distribution
struct distribution1d {
    vector<float>           _pdf;               // normalized probabilities
    vector<float>           _cdf;               // normalized cdf
    float                   _integral;          // total probability
    
    distribution1d() { }
    distribution1d(const vector<float>& probs) : distribution1d(probs.data(),probs.size()) { }
    distribution1d(const float* probs, int n) {
        _pdf = vector<float>(probs,probs+n);
        _cdf.resize(_pdf.size());
        std::partial_sum(_pdf.begin(), _pdf.end(), _cdf.begin());
        _integral = _cdf.back();
        if(_integral == 0) return;
        for(auto&& p : _pdf) p /= _integral;
        for(auto&& c : _cdf) c /= _integral;
    }
    
    float integral() const { return _integral; }
    
    int sample_discreet(float u) const { return std::upper_bound(_cdf.begin(), _cdf.end(), clamp(u,0.0f,0.999999f)) - _cdf.begin(); }
    float pdf_discreet(int idx) const { return _pdf[idx]; }
    
    float sample_continous(float u) const {
        auto idx = sample_discreet(u);
        auto du = (idx == 0) ? u / _cdf[0] : (u - _cdf[idx]) / (_cdf[idx] - _cdf[idx-1]);
        return (idx + du) / _pdf.size();
    }
    float pdf_continous(float u) const {
        auto idx = clamp((int)(u*_pdf.size()),0,_pdf.size()-1);
        return pdf_discreet(idx);
    }
};

// 2d distribution (data layout like an image) - picks a column and then a row (idea from pbrt)
struct distribution2d {
    vector<distribution1d>  _conditional;   // conditional probability (rows)
    distribution1d          _marginal;      // marginal probability (columns)
    
    distribution2d() { }
    distribution2d(const float* probs, int w, int h) {
        auto marginal_probs = vector<float>(h);
        for(auto j : range(h)) { _conditional.push_back(distribution1d(probs+j*w, w)); marginal_probs[j] = _conditional[j].integral(); }
        _marginal = distribution1d(marginal_probs);
    }
    
    float integral() const { return _marginal.integral(); }
    
    vec2i sample_discreet(const vec2f& uv) const {
        auto j = _marginal.sample_discreet(uv.y);
        error_if_not(_conditional[j].integral() > 0, "bad probability");
        auto i = _conditional[j].sample_discreet(uv.x);
        return {i,j};
    }
    float pdf_discreet(const vec2i& ij) const {
        return _conditional[ij.y].pdf_discreet(ij.x) * _marginal.pdf_discreet(ij.y);
    }
    vec2f sample_continous(const vec2f& uv) const {
        auto j = _marginal.sample_discreet(uv.y);
        error_if_not(_conditional[j].integral() > 0, "bad probability");
        auto v = _marginal.sample_continous(uv.y);
        auto u = _conditional[j].sample_continous(uv.x);
        return {u,v};
    }
    float pdf_continous(const vec2f& uv) const {
        auto j = clamp((int)(uv.y*_conditional.size()),0,_conditional.size()-1);
        return _conditional[j].pdf_continous(uv.x) * _marginal.pdf_continous(uv.y);
    }
};

#endif
