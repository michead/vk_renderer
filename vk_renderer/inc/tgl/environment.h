#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "common.h"
#include "vmath.h"

#include "material.h"

// environment map for background
struct Environment {
    string              name = "";                  // environment name
    frame3f             frame = identity_frame3f;   // environment frame
    vec3f               ke = zero3f;                // environment color
    Texture*            ke_txt = nullptr;           // environment texture
};

// envuv to direction
inline vec3f eval_envuv_to_dir(const frame3f& frame, const vec2f& uv) {
    return transform_direction(frame,{cos(uv.x*2*pif)*sin((1-uv.y)*pif),sin(uv.x*2*pif)*sin((1-uv.y)*pif),cos((1-uv.y)*pif)});  }
// direction to envuv
inline vec2f eval_dir_to_envuv(const frame3f& frame, const vec3f& dir) {
    auto ldir = transform_direction_inverse(frame, dir);
    auto phi = atan2(ldir.y, ldir.x);
    if(phi < 0) phi += 2*pif;
    auto theta = pif - acos(ldir.z);
    return vec2f(phi/(2*pif),theta/pif);
}

// evaluates the environment map
inline vec3f eval_env(const Environment* env, const vec3f& dir) {
    return env->ke * ((env->ke_txt) ? eval_texture(env->ke_txt, eval_dir_to_envuv(env->frame, dir)) : one3f);
}

// get pdf for light sampling
inline float sample_env_light_pdf(const Environment* env, const distribution2d* dist, const vec3f& wi) {
    // pick direction and pdf
    if(dist) {
        auto env_uv = eval_dir_to_envuv(env->frame,wi);
        return dist->pdf_continous(env_uv) * (env->ke_txt->img.width() * env->ke_txt->img.height()) / (2 * pif * pif * sin(env_uv.y * pif));
    } else {
        auto env_lwi = transform_direction_inverse(env->frame, wi);
        return sample_direction_spherical_uniform_pdf(env_lwi);
    }
}

// sample environment as a light
inline vec3f sample_env_light(const Environment* env, const distribution2d* dist, const vec3f& pos, const vec2f& ruv) {
    // pick direction and pdf
    if(dist) {
        auto env_uv = dist->sample_continous(ruv);
        return eval_envuv_to_dir(env->frame, env_uv);
    } else {
        auto env_lwi = sample_direction_spherical_uniform(ruv);
        return transform_direction(env->frame, env_lwi);
    }
}

// get light smpling distribution
inline distribution2d* sample_env_light_dist(const Environment* env) {
    if(!env->ke_txt) return nullptr;
    auto weights = image<float>(env->ke_txt->img.size());
    for(auto j = 0; j < weights.height(); j ++) {
        for (auto i = 0; i < weights.width(); i ++) {
            weights[{i,j}] = mean(env->ke_txt->img.at({i,j})) * sin((j + 0.5f) * pi / weights.height());
        }
    }
    return new distribution2d(weights.data(), weights.width(), weights.height());
}

#endif
