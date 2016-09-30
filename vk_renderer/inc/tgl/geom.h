#ifndef _GEOM_H_
#define _GEOM_H_

#include "vmath.h"

// spherical coordinates --------------------------
// BUG: check theta angle
// atan2 in [0,2*pi)
inline float atan2pos(float y, float x) { auto a = atan2(y,x); return (a >= 0) ? a : 2*pi+a; }
// spherical angles to direction
inline vec3f spherical_to_direction(const vec2f& angles) { auto phi = angles.x; auto theta = angles.y; return vec3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)); }
// direction to spherical angles
inline vec2f direction_to_spherical(const vec3f& w) {
    auto p = atan2(w.y, w.x);
    auto phi = (p < 0) ? p + 2*pif : p;
    auto theta = acos(clamp(w.z, -1.0f, 1.0f));
    return vec2f(phi,theta); }
// latlong texture coordinates to direction
inline vec3f latlong_to_direction(const vec2f& uv) { return spherical_to_direction(vec2f(uv.x * 2 * pif, uv.y * pif)); }
// direction to latlong texture coordinates
inline vec2f direction_to_latlong(const vec3f& w) { auto a = direction_to_spherical(w); return vec2f(a.x / (2*pif), a.y / pif); }
// spherical coordinates --------------------------

// interpolation ----------------------------------
// bernstein polynomials (for Bezier)
inline float bernstein(float u, int i, int degree) {
    if(i < 0 || i > degree) return 0;
    if(degree == 0) return 1;
    else if(degree == 1) {
        if(i == 0) return 1-u;
        else if(i == 1) return u;
    } else if(degree == 2) {
        if(i == 0) return (1-u)*(1-u);
        else if(i == 1) return 2*u*(1-u);
        else if(i == 2) return u*u;
    } else if(degree == 3) {
        if(i == 0) return (1-u)*(1-u)*(1-u);
        else if(i == 1) return 3*u*(1-u)*(1-u);
        else if(i == 2) return 3*u*u*(1-u);
        else if(i == 3) return u*u*u;
    } else {
        return (1-u)*bernstein(u, i, degree-1) + u*bernstein(u, i-1, degree-1);
    }
    return 0;
}
// bernstein polynomials (for Bezier)
inline float bernstein_derivative(float u, int i, int degree) { return degree * (bernstein(u, i-1, degree-1) - bernstein(u, i, degree-1)); }
// linear interpolation
template<typename T> inline T linear(const T& v0, const T& v1, float t) { return v0*(1-t)+v1*t; }
// linear interpolation
template<typename T> inline T linear(const std::vector<T>& v, const vec2i& i, float t) { return v[i.x]*(1-t) + v[i.y]*t; }
// bilinear interpolation
template<typename T> inline T bilinear(const T& v00, const T& v10, const T& v11, const T& v01, const vec2f& uv) { return v00*(1-uv.x)*(1-uv.y)+v10*uv.x*(1-uv.y)+v11*uv.x*uv.y+v01*(1-uv.x)*uv.y; }
// cubic bezier interpolation
template<typename T> inline T bezier_cubic(const T& v0, const T& v1, const T& v2, const T& v3, float t) { return v0*bernstein(t,0,3)+v1*bernstein(t,1,3)+v2*bernstein(t,2,3)+v3*bernstein(t,3,3); }
// cubic bezier interpolation
template<typename T> inline T bezier_cubic(const std::vector<T>& v, const vec4i& i, float t) { return bezier_cubic(v[i.x], v[i.y], v[i.z], v[i.w], t); }
// cubic bezier interpolation
template<typename T> inline T bezier_cubic_derivative(const T& v0, const T& v1, const T& v2, const T& v3, float t) { return v0*bernstein_derivative(t,0,3)+v1*bernstein_derivative(t,1,3)+v2*bernstein_derivative(t,2,3)+v3*bernstein_derivative(t,3,3); }
// cubic bezier interpolation
template<typename T> inline T bezier_cubic_derivative(const std::vector<T>& v, const vec4i& i, float t) { return bezier_cubic_derivative(v[i.x], v[i.y], v[i.z], v[i.w], t); }
// baricentric triangle interpolation
template<typename T> inline T baricentric_triangle(const T& v0, const T& v1, const T& v2, const vec2f& uv) { return v0*uv.x+v1*uv.y+v2*(1-uv.x-uv.y); }
// baricentric triangle interpolation
template<typename T> inline T baricentric_triangle(const std::vector<T>& v, const vec3i& i, const vec2f& uv) { return baricentric_triangle(v[i.x], v[i.y], v[i.z], uv); }
// interpolation ----------------------------------

// triangle ---------------------------------------
// baricentric interpolation
template<typename T> inline T triangle_value(const T& v0, const T& v1, const T& v2, const vec2f& uv) { return v0*uv.x+v1*uv.y+v2*(1-uv.x-uv.y); }
template<typename T> inline T triangle_value(const vector<T>& v, const vec3i& t, const vec2f& uv) { return v[t.x]*uv.x+v[t.y]*uv.y+v[t.z]*(1-uv.x-uv.y); }

// triangle normal
inline vec3f triangle_normal(const vec3f& v0, const vec3f& v1, const vec3f& v2) { return normalize(cross(v1-v0,v2-v0)); }

// triangle tangent and bitangent from uv (not othornormalized with themselfves not the normal)
inline pair<vec3f,vec3f> triangle_tangents_fromuv(const vec3f& v0, const vec3f& v1, const vec3f& v2,
                                                  const vec2f& uv0, const vec2f& uv1, const vec2f& uv2) {
    // normal points up from texture space
    // see http://www.terathon.com/code/tangent.html
    // see https://gist.github.com/aras-p/2843984
    auto p = v1 - v0;
    auto q = v2 - v0;
    auto s = vec2f(uv1.x-uv0.x,uv2.x-uv0.x);
    auto t = vec2f(uv1.y-uv0.y,uv2.y-uv0.y);
    auto div = s.x*t.y - s.y*t.x;
    
    if(div > 0) return { vec3f{t.y * p.x - t.x * q.x, t.y * p.y - t.x * q.y, t.y * p.z - t.x * q.z} / div,
        vec3f{s.x * q.x - s.y * p.x, s.x * q.y - s.y * p.y, s.x * q.z - s.y * p.z} / div };
    else return { x3f, y3f };
}
inline pair<vec3f,vec3f> triangle_tangents_fromuv(const vector<vec3f>& pos, const vector<vec2f>& uv, const vec3i& t) {
    return triangle_tangents_fromuv(pos[t.x], pos[t.y], pos[t.z], uv[t.x], uv[t.y], uv[t.z]);
}
// triangle ---------------------------------------

// frames -----------------------------------------
// sphere frame
inline frame3f sphere_frame(float radius, const vec2f& uv) {
    auto p = 2 * pif * uv.x, t = pif * (1 - uv.y);
    auto norm = normalize(vec3f{cos(p)*sin(t),sin(p)*sin(t),cos(t)});
    auto tang = normalize(vec3f{sin(p),cos(p),0});
    return frame3f{norm*radius,tang,normalize(cross(norm,tang)),norm};
}
// quad frame
inline frame3f quad_frame(float radius, const vec2f& uv) { return frame3f(vec3f(2*uv.x-1,2*uv.y-1,0)*radius,x3f,y3f,z3f); }
// triangle frame
inline frame3f triangle_frame(const vec3f& v0, const vec3f& v1, const vec3f& v2, const vec2f& uv) {
    auto frame = identity_frame3f;
    frame.o = triangle_value(v0, v1, v2, uv);
    frame.z = triangle_normal(v0, v1, v2);
    frame.x = normalize(v1-v0);
    frame.y = normalize(cross(frame.z,frame.x));
    return frame;
}
// frames -----------------------------------------

// bounds -----------------------------------------
// triangle bounds
inline range3f triangle_bounds(const vec3f& v0, const vec3f& v1, const vec3f& v2) { return make_range3f({v0,v1,v2}); }
// sphere bounds
inline range3f sphere_bounds(const vec3f& pos, float r) { return range3f(pos-vec3f(r,r,r),pos+vec3f(r,r,r)); }
// triangle bounds
inline range3f cylinder_bounds(float r, float h) { return range3f(vec3f(-r,-r,0),vec3f(r,r,h)); }
// quad bounds
inline range3f quad_bounds(float w, float h) { return range3f(vec3f(-w/2,-h/2,0),vec3f(w/2,h/2,0)); }
// quad bounds
inline range3f quad_bounds(float r) { return range3f(vec3f(-r,-r,0),vec3f(r,r,0)); }
// disk bounds
inline range3f disk_bounds(float r) { return range3f({-r,-r,0},{r,r,0}); }
// bounds -----------------------------------------

// areas ------------------------------------------
// triangle area
inline float triangle_area(const vec3f& v0, const vec3f& v1, const vec3f& v2) { return length(cross(v2-v0, v2-v1))/2; }
// sphere area
inline float sphere_area(float r) { return 4*pi*r*r; }
// cylinder area
inline float cylinder_area(float r, float h) { return 2*pi*r*h; }
// disk area
inline float disk_area(float r) { return pi * r * r; }
/// sphere volume
inline float sphere_volume(float r) { return 4*pi/3*r*r*r; }
// areas ------------------------------------------

// intersect --------------------------------------

// intersect bounding box
inline bool intersect_bbox(const ray3f& ray, const range3f& bbox, float* t0_out = nullptr, float* t1_out = nullptr) {
    auto t0 = ray.tmin, t1 = ray.tmax;
    for (int i = 0; i < 3; ++i) {
        auto invRayDir = 1.f / ray.d[i];
        auto tNear = (bbox.min[i] - ray.e[i]) * invRayDir;
        auto tFar  = (bbox.max[i] - ray.e[i]) * invRayDir;
        if (tNear > tFar) std::swap(tNear, tFar);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar  < t1 ? tFar  : t1;
        if (t0 > t1) return false;
    }
    if(t0_out) *t0_out = t0;
    if(t1_out) *t1_out = t1;
    return true;
}

// intersect triangle
inline bool intersect_triangle(const ray3f& ray, const vec3f& v0, const vec3f& v1, const vec3f& v2, float* t_out, vec2f* uv_out) {
    auto a = v0 - v2;
    auto b = v1 - v2;
    auto e = ray.e - v2;
    auto i = ray.d;
    
    auto d = dot(cross(i,b),a);
    if(d == 0) return false;
    
    auto t =  dot(cross(e,a),b) / d;
    if(t < ray.tmin || t > ray.tmax) return false;
    
    auto ba = dot(cross(i,b),e) / d;
    auto bb = dot(cross(a,i),e) / d;
    if(ba < 0 || bb < 0 || ba+bb > 1) return false;
    
    if(t_out) *t_out = t;
    if(uv_out) *uv_out = {ba,bb};
    
    return true;
}

// intersect sphere
// BUG: only considers first intersection
inline bool intersect_sphere(const ray3f& ray, float radius, float* t_out, vec2f* uv_out) {
    auto a = lengthSqr(ray.d);
    auto b = 2*dot(ray.d,ray.e);
    auto c = lengthSqr(ray.e) - radius*radius;
    auto d = b*b-4*a*c;
    if(d < 0) return false;
    auto t = (-b-sqrt(d)) / (2*a);
    if (t < ray.tmin || t > ray.tmax) return false;
    if(t_out) *t_out = t;
    // BUG: wrong parameterization in atan2
    if(uv_out) { auto n = normalize(ray.eval(t)); *uv_out = {(pif+(float)atan2(n.y, n.x))/(2*pif),(float)acos(n.z)/pif}; }
    return true;
}

// intersect quad
inline bool intersect_quad(const ray3f& ray, float radius, float* t_out, vec2f* uv_out) {
    // BUG: handle infinite intersections
    if(ray.d.z == 0) return false;
    auto t = - ray.e.z / ray.d.z;
    auto p = ray.eval(t);
    if(radius < p.x || -radius > p.x || radius < p.y || -radius > p.y) return false;
    if (t < ray.tmin || t > ray.tmax) return false;
    if(t_out) *t_out = t;
    if(uv_out) *uv_out = {0.5f*p.x/radius+0.5f,0.5f*p.y/radius+0.5f};
    return true;
}

// interset disk
inline bool intersect_disk(const ray3f& ray, float radius, float* t_out, vec2f* uv_out) {
    // BUG: handle infinite intersections
    if(ray.d.z == 0) return false;
    auto t = - ray.e.z / ray.d.z;
    if(t < ray.tmin || t > ray.tmax) return false;
    auto p = ray.eval(t);
    if(p.x*p.x+p.y*p.y < radius*radius) return false;
    if(t_out) *t_out = t;
    // BUG: wrong parameterization in atan2
    if(uv_out) { auto pl = vec2f(p.x/radius,p.y/radius); *uv_out = {length(pl),atan2(pl.y,pl.x)/(2*pif)}; }
    return true;
}

// intersect cylinder
inline bool intersect_cylinder(const ray3f& ray, float r, float h, float* t_out) {
    // http://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html
    // http://www.gamedev.net/topic/467789-raycylinder-intersection/
    auto a = ray.d.x*ray.d.x+ray.d.y*ray.d.y;
    auto b = 2*ray.e.x*ray.d.x+2*ray.e.y*ray.d.y;
    auto c = ray.e.x*ray.e.x+ray.e.y*ray.e.y-r*r;
    auto d = b*b-4*a*c;
    if(d < 0) return false;
    auto tmin = (-b-sqrt(d)) / (2*a);
    auto tmax = (-b+sqrt(d)) / (2*a);
    auto zmin = ray.e.z+tmin*ray.d.z;
    auto zmax = ray.e.z+tmax*ray.d.z;
    if (tmin >= ray.tmin && tmin <= ray.tmax && zmin >= 0 && zmin <= h) {
        if(t_out) *t_out = tmin;
        return true;
    }
    if (tmax >= ray.tmin && tmax <= ray.tmax && zmax >= 0 && zmax <= h) {
        if(t_out) *t_out = tmax;
        return true;
    }
    return false;
}

// intersect point (approximate)
inline bool intersect_point_approximate(const ray3f& ray, const vec3f& p, float radius, float* t_out) {
    // http://geomalgorithms.com/a02-_lines.html
    auto t = - dot(ray.e - p, ray.d);
    t = clamp(t, ray.tmin, ray.tmax);
    if(length(p-ray.eval(t)) > radius) return false;
    if (t < ray.tmin || t > ray.tmax) return false;
    if(t_out) *t_out = t;
    return true;
}

// intersect line (approximate)
inline bool intersect_line_approximate(const ray3f& ray, const vec3f& v0, const vec3f& v1, float r0, float r1, float* t_out, float* u_out) {
    // http://geomalgorithms.com/a05-_intersect-1.html
    // http://geomalgorithms.com/a07-_distance.html#dist3D_Segment_to_Segment
    auto r = ray3f::make_segment(v0, v1);
    if(abs(dot(ray.d,r.d)) == 1) return false;
    auto a = dot(ray.d,ray.d);
    auto b = dot(ray.d,r.d);
    auto c = dot(r.d,r.d);
    auto d = dot(ray.d,ray.e-r.e);
    auto e = dot(r.d,ray.e-r.e);
    auto t = (b*e-c*d)/(a*c-b*b);
    auto s = (a*e-b*d)/(a*c-b*b);
    t = clamp(t, ray.tmin, ray.tmax);
    s = clamp(s, r.tmin, r.tmax);
    auto ss = s / length(v1-v0);
    auto rr = r0*(1-ss)+r1*ss;
    if(dist(ray.eval(t), r.eval(s)) > rr) return false;
    if(t_out) *t_out = t;
    if(u_out) *u_out = ss;
    return true;
}

// intersect --------------------------------------

#endif
