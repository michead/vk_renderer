#ifndef _BRDF_H_
#define _BRDF_H_

#include "common.h"
#include "vmath.h"
#include "montecarlo.h"

// brdf
struct Brdf {
    vec3f               kd = zero3f;
    vec3f               ks = zero3f;
    float               rs = 0;
    vec3f               kr = zero3f;

	Brdf() {}

	Brdf(vec3f kd_, vec3f ks_, float rs_, vec3f kr_) : kd(kd_), ks(ks_), rs(rs_), kr(kr_) {}
    
    bool is_zero() const { return kd == zero3f && ks == zero3f && kr == zero3f; }
};

// phong exponent (check from walter)
inline float beckman_to_phong(float alpha) { return 2 / (alpha*alpha) - 2; }
inline float phong_to_beckman(float n) { return sqrt(2 / (n + 2)); }

// fresnel approximation
inline vec3f fresnel_approx(const vec3f& f, float cost) {
    return f + (one3f-f) * pow(1.0f-clamp(cost,0.0f,1.0f),5.0f);
}

// TODO: fresnel hack
// could normalize diffuse by
// 1 - (F(ks) - Ks) / (1 - Ks)

// compute the brdf
inline vec3f eval_brdf(const Brdf& brdf, const frame3f& frame, const vec3f& v, const vec3f& l) {
    // phong
    // auto h = normalize(v+l);
    // return brdf.kd/pif + brdf.ks*(brdf.n+8)/(8*pif) * pow(max(0.0f,dot(norm,h)),brdf.n);
    
    // BUG: denominator clamp
    // BUG: shadow masking clamp
    // BUG: bad clamp for dot on same side
    // microfacet phong
    if(dot(frame.z,v) <= 0 || dot(frame.z,l) <= 0) return zero3f;
    const float denominator_clamp = 0.00001f;
    const bool clamp_s = true;
    auto h = normalize(v+l);
    auto n = beckman_to_phong(brdf.rs);
    auto s = min(1.0f,min(2*dot(h,frame.z)*dot(l,frame.z)/dot(h,l),2*dot(h,frame.z)*dot(v,frame.z)/dot(h,v)));
    if(clamp_s && s < 0) s = 0;
    auto f = (brdf.ks == zero3f) ? zero3f : fresnel_approx(brdf.ks, dot(h,l));
    auto d = (n+2)/(2*pif) * pow(clamp(dot(frame.z,h),0.0f,1.0f),n);
    auto dd = 4*max(denominator_clamp,dot(v,frame.z))*max(denominator_clamp,dot(l,frame.z));
    return brdf.kd/pif + s*f*d/dd;
}

// compute emission
inline vec3f eval_emission(const vec3f& ke, const frame3f& frame, const vec3f& v) {
    if(dot(v,frame.z) <= 0) return zero3f;
    return ke;
}

// compute the pdf for brdf sampling
inline float sample_brdf_pdf(const Brdf& brdf, const frame3f& frame, const vec3f& v, const vec3f& l) {
    if(dot(frame.z,v) <= 0 || dot(frame.z,l) <= 0) return 0;
    auto n = beckman_to_phong(brdf.rs);
    auto dw = mean(brdf.kd) / (mean(brdf.kd) + mean(brdf.ks));
    auto v_local = transform_direction_inverse(frame, v);
    auto l_local = transform_direction_inverse(frame, l);
    auto h_local = normalize(v_local+l_local);
    auto dpdf = sample_direction_hemispherical_cosine_pdf(l_local);
    auto spdf = sample_direction_hemispherical_cospower_pdf(h_local,n) / (4*dot(v_local,h_local));
    return dw * dpdf + (1-dw) * spdf;
}

// pick a direction according to the brdf (returns direction and its pdf)
inline vec3f sample_brdf(const Brdf& brdf, const frame3f& frame, const vec3f& v, const vec2f& ruv, float rl) {
    if(dot(frame.z,v) <= 0) return zero3f;
    auto n = beckman_to_phong(brdf.rs);
    auto dw = mean(brdf.kd) / (mean(brdf.kd) + mean(brdf.ks));
    auto v_local = transform_direction_inverse(frame, v);
    auto l_local = zero3f, h_local = zero3f;
    if(rl < dw) {
        l_local = sample_direction_hemispherical_cosine(ruv);
        h_local = normalize(l_local+v_local);
    } else {
        h_local = sample_direction_hemispherical_cospower(ruv, n);
        l_local = -v_local + h_local*2*dot(v_local,h_local);
    }
    return transform_direction(frame, l_local);
}

#endif

