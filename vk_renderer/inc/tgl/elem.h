#ifndef _ELEM_H_
#define _ELEM_H_

#include "common.h"
#include "vmath.h"

#include "mesh.h"
#include "material.h"
#include "environment.h"

// scene element
struct Elem {
    // main attributes
    string  name = "";                          // name
    
    // basic attributes
    frame3f         frame = identity_frame3f;   // frame
    Material*       material = nullptr;         // material
    Mesh*           mesh = nullptr;             // mesh shape
};

// resolve mesh point
inline void elem_point(const Elem* elem, int eid, const vec2f& euv, frame3f* geom_frame, frame3f* shading_frame, vec3f* ke, Brdf* brdf) {
    auto local_frame = identity_frame3f; auto col = zero3f; auto texcoord = zero2f;
    mesh_point(elem->mesh, eid, euv, &local_frame, &col, &texcoord);
    auto material = elem->material;
    if(ke) *ke = material_emission(material, col, texcoord);
    if(brdf) *brdf = material_brdf(material, col, texcoord);
    if(shading_frame) {
        auto perturbed_frame = material_frame(material, texcoord, local_frame);
        *shading_frame = transform_frame(elem->frame, perturbed_frame);
    }
    if(geom_frame) {
        *geom_frame = transform_frame(elem->frame, local_frame);
    }
}

// compute mesh bounds
inline range3f elem_bounds(const Elem* elem) {
    return mesh_bounds_transformed(elem->mesh,elem->frame);
}

// compute scene bounds
inline range3f elems_bounds(const vector<Elem*>& elems, bool skip_lights) {
    auto bbox = range3f();
    for(auto elem : elems) if(!skip_lights || !elem->material || elem->material->ke == zero3f) bbox = runion(bbox, elem_bounds(elem));
    return bbox;
}

// eval approximate point light
inline void approximate_pointlight(const Elem* elem, vec3f* pos, vec3f* ke) {
    *ke = elem->material->ke * mesh_size(elem->mesh);
    *pos = transform_point(elem->frame,center(mesh_bounds(elem->mesh)));
}

#endif

