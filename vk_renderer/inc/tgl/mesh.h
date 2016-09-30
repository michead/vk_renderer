#ifndef _MESH_H_
#define _MESH_H_

#include "common.h"
#include "vmath.h"
#include "geom.h"
#include "montecarlo.h"
#include "texture.h"

#define Mesh Mesh_

// indexed mesh data structure with vertex data and element data
struct Mesh_ {
    // type
    enum struct Type : int { TRIANGLE = 0, LINE = 1, POINT = 2 };
    
    // full mesh properties
    string          name = "";                  // name
    Type            type = Type::TRIANGLE;      // type
    string          filename = "";              // filename
    
    // vertex properties
    vector<vec3f>   pos;                        // vertex position
    vector<vec3f>   norm;                       // vertex normal
    vector<vec2f>   texcoord;                   // vertex texture coordinates
    vector<float>   radius;                     // vertex radius (lines / points)
    vector<vec3f>   col;                        // vertex colors
    vector<vec3f>   vel;                        // vertex velocity
    
    // tangent space --- computed on the fly
    vector<vec3f>   tang_sp;                    // vertex tangents
    
    // elements
    vector<vec3i>   triangle;                   // triangle
    vector<vec2i>   line;                       // line
    vector<int>     point;                      // point
    
    // elements to be tesselated
    vector<vec4i>   quad;                       // quads
    vector<vec4i>   spline;                     // spline
};

// mesh number of elements
inline int mesh_numelems(const Mesh_* mesh) {
    switch(mesh->type) {
        case Mesh::Type::TRIANGLE: return mesh->triangle.size();
        case Mesh::Type::LINE: return mesh->line.size();
        case Mesh::Type::POINT: return mesh->point.size();
    }
}

// compute properties of the mesh point
inline void mesh_point(const Mesh* mesh, int eid, const vec2f& euv, frame3f* local_frame, vec3f* col, vec2f* texcoord) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: {
            const auto& triangle = mesh->triangle[eid];
            if(local_frame) {
                if(mesh->tang_sp.empty()) {
                    *local_frame = frame_from_z(baricentric_triangle(mesh->pos,triangle,euv),triangle_value(mesh->norm,triangle,euv));
                } else {
                    *local_frame = frame_from_zx(baricentric_triangle(mesh->pos,triangle,euv),
                                                 baricentric_triangle(mesh->tang_sp,triangle,euv),
                                                 baricentric_triangle(mesh->norm,triangle,euv));
                }
            }
            if(texcoord) *texcoord = (mesh->texcoord.empty()) ? zero2f : baricentric_triangle(mesh->texcoord,triangle,euv);
            if(col) *col = (mesh->col.empty()) ? one3f : baricentric_triangle(mesh->col, triangle, euv);
        } break;
        case Mesh::Type::LINE: {
            const auto& line = mesh->line[eid];
            if(local_frame) *local_frame = frame_from_z(linear(mesh->pos,line,euv.x),linear(mesh->norm,line,euv.x));
            if(texcoord) *texcoord = (mesh->texcoord.empty()) ? zero2f : linear(mesh->texcoord,line,euv.x);
            if(col) *col = (mesh->col.empty()) ? one3f : linear(mesh->col,line,euv.x);
        } break;
        case Mesh::Type::POINT: {
            const auto& point = mesh->point[eid];
            if(local_frame) *local_frame = frame3f(mesh->pos[point],x3f,y3f,z3f);
            if(texcoord) *texcoord = (mesh->texcoord.empty()) ? zero2f : mesh->texcoord[point];
            if(col) *col = (mesh->col.empty()) ? one3f : mesh->col[point];
        } break;
        default:
            break;
    }
}

// compute the total mesh size (area, length, num)
inline float mesh_size(const Mesh* mesh) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: {
            auto area = 0.0f;
            for(auto&& triangle : mesh->triangle) area += triangle_area(mesh->pos[triangle.x],mesh->pos[triangle.y],mesh->pos[triangle.z]);
            return area;
        } break;
        case Mesh::Type::LINE: {
            auto length = 0.0f;
            for(auto&& line : mesh->line) length += dist(mesh->pos[line.x],mesh->pos[line.y]);
            return length;
        } break;
        case Mesh::Type::POINT: {
            return mesh->point.size();
        } break;
    }
}

// compute the size of a mesh element (area, length, or count)
inline float mesh_elem_size(const Mesh* mesh, int eid) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: {
            auto& triangle = mesh->triangle[eid];
            return triangle_area(mesh->pos[triangle.x],mesh->pos[triangle.y],mesh->pos[triangle.z]);
        } break;
        case Mesh::Type::LINE: {
            const auto& line = mesh->line[eid];
            return dist(mesh->pos[line.x],mesh->pos[line.y]);
        } break;
        case Mesh::Type::POINT: {
            return 1;
        } break;
    }
}

// compute mesh bounds
inline range3f mesh_bounds(const Mesh* mesh) {
    switch(mesh->type) {
        case Mesh::Type::TRIANGLE: return make_range3f(mesh->pos);
        case Mesh::Type::LINE:
        case Mesh::Type::POINT: {
            auto bbox = range3f();
            for(auto i = 0; i < mesh->pos.size(); i ++) {
                bbox = runion(bbox, mesh->pos[i] - vec3f(mesh->radius[i],mesh->radius[i],mesh->radius[i]));
                bbox = runion(bbox, mesh->pos[i] + vec3f(mesh->radius[i],mesh->radius[i],mesh->radius[i]));
            }
            return bbox;
        } break;
    }
}

// compute mesh bounds
inline range3f mesh_bounds_transformed(const Mesh* mesh, const frame3f& frame) {
    switch(mesh->type) {
        case Mesh::Type::TRIANGLE: {
            auto bbox = range3f();
            for(auto&& p : mesh->pos) bbox = runion(bbox, transform_point(frame, p));
            return bbox;
        } break;
        case Mesh::Type::LINE:
        case Mesh::Type::POINT: {
            auto bbox = range3f();
            for(auto i = 0; i < mesh->pos.size(); i ++) {
                bbox = runion(bbox, transform_point(frame, mesh->pos[i] - vec3f(mesh->radius[i],mesh->radius[i],mesh->radius[i])));
                bbox = runion(bbox, transform_point(frame, mesh->pos[i] + vec3f(mesh->radius[i],mesh->radius[i],mesh->radius[i])));
            }
            return bbox;
        } break;
    }
}

// compute mesh bounds
inline range3f mesh_elem_bounds(const Mesh* mesh, int eid) {
    switch(mesh->type) {
        case Mesh::Type::TRIANGLE: {
            auto triangle = mesh->triangle[eid];
            return make_range3f({mesh->pos[triangle.x],mesh->pos[triangle.y],mesh->pos[triangle.z]});
        } break;
        case Mesh::Type::LINE: {
            auto line = mesh->line[eid];
            return make_range3f({
                mesh->pos[line.x] - vec3f(mesh->radius[line.x],mesh->radius[line.x],mesh->radius[line.x]),
                mesh->pos[line.x] + vec3f(mesh->radius[line.x],mesh->radius[line.x],mesh->radius[line.x]),
                mesh->pos[line.y] - vec3f(mesh->radius[line.y],mesh->radius[line.y],mesh->radius[line.y]),
                mesh->pos[line.y] + vec3f(mesh->radius[line.y],mesh->radius[line.y],mesh->radius[line.y])
            });
        } break;
        case Mesh::Type::POINT: {
            auto point = mesh->point[eid];
            return make_range3f({
                mesh->pos[point] - vec3f(mesh->radius[point],mesh->radius[point],mesh->radius[point]),
                mesh->pos[point] + vec3f(mesh->radius[point],mesh->radius[point],mesh->radius[point])
            });
        } break;
    }
}

// return the pdf of sampling a mesh
inline float sample_mesh_pdf(const Mesh* mesh, const distribution1d* dist, int eid, const vec2f& euv) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: return dist->pdf_discreet(eid) / mesh_elem_size(mesh, eid);
        case Mesh::Type::LINE: return dist->pdf_discreet(eid) / mesh_elem_size(mesh, eid);
        case Mesh::Type::POINT: return mesh->point.size();
    }
}

// picking points on a mesh using a uniform distribution on the elements areas/lengths/numbers
inline pair<int,vec2f> sample_mesh(const Mesh* mesh, const distribution1d* dist, const vec2f& ruv, float re) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: return { dist->sample_discreet(re), {1-sqrt(ruv.x),ruv.y*sqrt(ruv.x)} };
        case Mesh::Type::LINE: return { dist->sample_discreet(re), ruv };
        case Mesh::Type::POINT: return { clamp((int)(re*mesh->point.size()),0,mesh->point.size()-1), ruv };
    }
}

// make a uniform distribution of mesh elements
inline distribution1d* sample_mesh_dist(const Mesh* mesh, const vector<vec3f>* vertex_weight, const image3f* texture_weight) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: {
            auto areas = vector<float>();
            for(auto eid : range(mesh->triangle.size())) areas.push_back(mesh_elem_size(mesh,eid));
            if(vertex_weight) for(auto eid : range(mesh->triangle.size())) {
                auto& t = mesh->triangle[eid];
                areas[eid] *= mean(vertex_weight->at(t.x) + vertex_weight->at(t.x) + vertex_weight->at(t.x)) / 3;
            }
            if(texture_weight && !mesh->texcoord.empty()) for(auto eid : range(mesh->triangle.size())) {
                auto& t = mesh->triangle[eid];
                vec3f tt = zero3f;
                for(auto i : range(4)) for(auto j : range(4)) tt += eval_texture(texture_weight, triangle_value(mesh->texcoord[t.x], mesh->texcoord[t.y], mesh->texcoord[t.z], sample_uniform_triangle({(i + 0.5f) / 4, (j + 0.5f) / 4})));
                areas[eid] *= mean(tt) / 16;
            }
            return new distribution1d(areas);
        } break;
        case Mesh::Type::LINE: {
            auto lengths = vector<float>();
            for(auto eid : range(mesh->line.size())) lengths.push_back(mesh_elem_size(mesh,eid));
            if(vertex_weight && !mesh->texcoord.empty()) for(auto eid : range(mesh->line.size())) {
                auto& l = mesh->line[eid];
                lengths[eid] *= mean(vertex_weight->at(l.x) + vertex_weight->at(l.x)) / 2;
            }
            return new distribution1d(lengths);
        } break;
        case Mesh::Type::POINT: {
            return nullptr;
        } break;
    }
}

// SIMPLE UTILITIES -------------------------------------

// quads to triangles
inline vector<vec3i> quads_to_triangles(const vector<vec4i>& quad) {
    if(quad.empty()) return {};
    vector<vec3i> triangle;
    for(auto q : quad) {
        triangle.push_back({q.x,q.y,q.z});
        triangle.push_back({q.z,q.w,q.x});
    }
    return triangle;
}

// vertex normals
inline vector<vec3f> vertex_normals(const vector<vec3f>& pos, const vector<vec3i>& triangle) {
    // set normals array to the same length as pos and init all elements to zero
    auto norm = vector<vec3f>(pos.size(),zero3f);
    // foreach triangle
    for(auto f : triangle) {
        // compute face normal
        auto fn = triangle_normal(pos[f.x], pos[f.y], pos[f.z]);
        // compute triangle area
        auto a = triangle_area(pos[f.x], pos[f.y], pos[f.z]);
        // accumulate face normal to the vertex normals of each face index
        for (auto v : f) norm[v] += a * fn;
    }
    // normalize all vertex normals
    for (auto& n : norm) n = normalize(n);
    // done
    return norm;
}

// tangent space
inline pair<vector<vec3f>,vector<vec3f>> vertex_tangentspace(const vector<vec3f>& pos, const vector<vec3f>& norm, const vector<vec2f>& texcoord, const vector<vec3i>& triangle) {
    // set normals array to the same length as pos and init all elements to zero
    auto tang = vector<vec3f>(pos.size(),zero3f);
    auto bitang = vector<vec3f>(pos.size(),zero3f);
    // foreach triangle
    for(auto f : triangle) {
        // compute face normal
        auto ft = triangle_tangents_fromuv(pos[f.x], pos[f.y], pos[f.z], texcoord[f.x], texcoord[f.y], texcoord[f.z]);
        // compute triangle area
        auto a = triangle_area(pos[f.x], pos[f.y], pos[f.z]);
        // accumulate face normal to the vertex normals of each face index
        for (auto v : f) { tang[v] += a * ft.first; bitang[v] += a * ft.second; }
    }
    for (auto i : range(pos.size())) {
        // orthonormalize
        tang[i] = orthonormalize(tang[i], norm[i]);
        // get sign
        auto s = (dot(cross(norm[i], tang[i]), bitang[i]) < 0) ? -1.0f : 1.0f;
        bitang[i] = s * normalize(cross(norm[i], tang[i]));
    }
    // done
    return {tang,bitang};
}

// line vertex tangent
inline vector<vec3f> vertex_tangents(const vector<vec3f>& pos, const vector<vec2i>& line) {
    // set tangent array
    auto tang = vector<vec3f>(pos.size(),zero3f);
    // foreach line
    for(auto l : line) {
        // compute line tangent
        auto lt = normalize(pos[l.y]-pos[l.x]);
        // accumulate segment tangent to vertex tangent on each vertex
        for (auto v : l) tang[v] += lt;
    }
    // normalize all vertex tangents
    for (auto& t : tang) t = normalize(t);
    // done
    return tang;
}

// UPDATES ----------------------------------------------

// update vertex normals
inline void update_vertex_frames(Mesh* mesh) {
    switch (mesh->type) {
        case Mesh::Type::TRIANGLE: mesh->norm = vertex_normals(mesh->pos, mesh->triangle); break;
        case Mesh::Type::LINE: mesh->norm = vertex_tangents(mesh->pos, mesh->line); break;
        case Mesh::Type::POINT: mesh->norm.clear(); break;
    }
}

// update vertex normals
inline void update_tangent_space(Mesh* mesh) {
	if (!mesh->texcoord.empty()) 
	{
		auto tangentspace = vertex_tangentspace(mesh->pos, mesh->norm, mesh->texcoord, mesh->triangle);
		mesh->tang_sp = tangentspace.first;
	}
}

// transform a mesh data
inline void transform_mesh(Mesh* mesh, const mat4f& xf) {
    for(auto& p : mesh->pos) p = transform_point(xf, p);
    for(auto& n : mesh->norm) n = transform_normal(xf, n);
}

#endif

