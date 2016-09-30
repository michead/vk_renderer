#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "common.h"
#include "vmath.h"
#include "image.h"

// texture data
struct Texture {
    string  name = "";          // texture name
    string  filename = "";      // filename
    bool    filelinear = false; // file is saved in linear format
    
    image3f img;                // texture image
};

// eval texture
inline vec3f eval_texture(const image3f* texture, const vec2f& uv, bool tile = true) {
    auto size = vec2i(texture->width(),texture->height());
    auto ij = vec2i(uv.x*size.x,uv.y*size.y);
    auto st = uv*vec2f(size.x,size.y) - vec2f(ij.x,ij.y);
    auto iijj = ij+one2i;
    if(tile) {
        ij.x %= size.x; if(ij.x < 0) ij.x += size.x;
        ij.y %= size.y; if(ij.y < 0) ij.y += size.y;
        iijj.x %= size.x; if(iijj.x < 0) iijj.x += size.x;
        iijj.y %= size.y; if(iijj.y < 0) iijj.y += size.y;
    } else {
        ij = clamp(ij,zero2i,size-one2i);
        iijj = clamp(iijj,zero2i,size-one2i);
    }
    return (texture->at(ij.x,ij.y)*(1-st.x)*(1-st.y)+
            texture->at(ij.x,iijj.y)*(1-st.x)*st.y+
            texture->at(iijj.x,ij.y)*st.x*(1-st.y)+
            texture->at(iijj.x,iijj.y)*st.x*st.y);
}

// eval texture
inline vec3f eval_texture(const Texture* texture, const vec2f& uv) {
    return eval_texture(&texture->img, uv);
}

#endif

