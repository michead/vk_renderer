#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "common.h"
#include "vmath.h"

#include "image.h"
#include "brdf.h"
#include "texture.h"


// blinn-phong material
// textures are scaled by the respective coefficient and may be missing
struct Material {
    string      name = "";              // name
    float       opacity = 1;            // opacity
    
    vec3f       ke = zero3f;            // emission coefficient
    vec3f       kd = zero3f;            // diffuse coefficient
    vec3f       ks = zero3f;            // specular coefficient
    float       rs = 0.15f;             // roughness coefficient (alpha)
    vec3f       kr = zero3f;            // reflection coefficient
    
    Texture*    ke_txt = nullptr;       // emission texture
    Texture*    kd_txt = nullptr;       // diffuse texture
    Texture*    ks_txt = nullptr;       // specular texture
    Texture*    rs_txt = nullptr;       // roughness parameter
    Texture*    kr_txt = nullptr;       // reflection texture
    Texture*    norm_txt = nullptr;     // normal texture

    bool        double_sided = false;   // double-sided material

	float		translucency = 0;		// translucency
	float		sss_width = 0;			// subsurface scattering profile width
};

// evaluate the material to reolve texture lookups
inline Brdf material_brdf(const Material* material, const vec3f& col, const vec2f& texcoord) {
    return {
        col * material->kd * ((material->kd_txt) ? eval_texture(material->kd_txt, texcoord) : one3f),
        col * material->ks * ((material->ks_txt) ? eval_texture(material->ks_txt, texcoord) : one3f),
        material->rs * ((material->rs_txt) ? mean(eval_texture(material->rs_txt, texcoord)) : 1),
        col * material->kr * ((material->kr_txt) ? eval_texture(material->kr_txt, texcoord) : one3f)
    };
}

// evaluate the material to reolve texture lookups
inline vec3f material_emission(const Material* material, const vec3f& col, const vec2f& texcoord) {
    return col * material->ke * ((material->ke_txt) ? eval_texture(material->ke_txt, texcoord) : one3f);
}

// evaluate the material perturbed frame
inline frame3f material_frame(const Material* material, const vec2f& texcoord, const frame3f& frame) {
    // handle normal map
    if(material->norm_txt) {
        // BUG: check spaces
        auto norm = eval_texture(material->norm_txt, texcoord);
        norm = normalize(2*norm-one3f);
        // BUG: this should be a rotation
        auto ret = frame;
        ret.z = transform_normal(frame,norm);
        ret.x = orthonormalize(ret.x,ret.z);
        ret.y = normalize(cross(ret.z,ret.x));
        return ret;
    } else return frame;
}

#endif

