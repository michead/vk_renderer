#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "common.h"
#include "elem.h"
#include "vmath.h"

#define NEAR_PLANE_DIST 3
#define FAR_PLANE_DIST	7

// perspective camera at frame.o with direction (-z)
// and image plane orientation (x,y); the image plane
// is at a distance dist with size (width,height);
// the camera is focussed at a distance focus.
struct Camera {
    string  name = "";                  // camera name
    frame3f frame = identity_frame3f;   // frame
    float   width = 1;                  // image plane width
    float   height = 1;                 // image plane height
    float   dist = 1;                   // image plane distance
    float   focus = 1;                  // distance of focus
	mat4f	view_matrix;				// view matrix
	mat4f	projection_matrix;			// projection matrix

	void update_matrices() 
	{
		projection_matrix = frustum_matrix(-NEAR_PLANE_DIST * width / (2 * dist),
											NEAR_PLANE_DIST * width / (2 * dist),
										   -NEAR_PLANE_DIST * height / (2 * dist),
											NEAR_PLANE_DIST * height / (2 * dist),
											NEAR_PLANE_DIST, FAR_PLANE_DIST);

		view_matrix = frame_to_matrix_inverse(frame);
	}
};

// camera image size from resolution
inline vec2i image_size(const Camera* camera, int resolution) { return vec2i(resolution * camera->width / camera->height, resolution); }

// compute camera ray
inline ray3f camera_ray(const Camera* camera, const vec2f& uv) {
    return transform_ray(camera->frame,ray3f(zero3f,normalize(vec3f((uv.x-0.5f)*camera->width,(uv.y-0.5f)*camera->height,-camera->dist))));
}

// NAVIGATION -----------------------------------------------

// create a Camera at eye, pointing towards center with up vector up, and with specified image plane params
inline void lookat_camera(Camera* camera, const vec3f& eye, const vec3f& center, const vec3f& up, float width, float height, float dist) {
    camera->frame = lookat_frame(eye, center, up, true);
    camera->width = width;
    camera->height = height;
    camera->dist = dist;
    camera->focus = length(eye-center);
}

// set camera view with a "turntable" modification
inline void turntable_camera(Camera* camera, float rotate_phi, float rotate_theta, float dolly, float pan_x, float pan_y) {
    auto phi = atan2(camera->frame.z.z,camera->frame.z.x) + rotate_phi;
    auto theta = clamp(acos(camera->frame.z.y) + rotate_theta, 0.001f,pif-0.001f);
    auto new_z = vec3f(sin(theta)*cos(phi),cos(theta),sin(theta)*sin(phi));
    auto new_center = camera->frame.o-camera->frame.z*camera->focus;
    auto new_o = new_center + new_z * camera->focus;
    camera->frame = lookat_frame(new_o,new_center,y3f,true);
    camera->focus = dist(new_o,new_center);
    
    // apply dolly
    vec3f c = camera->frame.o - camera->frame.z * camera->focus;
    camera->focus = max(camera->focus+dolly,0.00001f);
    camera->frame.o = c + camera->frame.z * camera->focus;
    
    // apply pan
    camera->frame.o += camera->frame.x * pan_x + camera->frame.y * pan_y;
}

// set the camera view to frame a bounding box
inline void frame_camera(Camera* camera, const range3f& bbox, float scale = 1.25) {
    auto bsphere_radius = length(size(bbox))/2;
    auto focus = scale * bsphere_radius * camera->dist / (camera->height/2);
    camera->focus = focus;
    camera->frame.o = center(bbox) + focus * camera->frame.z;
}

inline mat4f view_proj_tex_mat(mat4f view, mat4f proj) 
{
	mat4f scale = scaling_matrix(vec3f(0.5f, -0.5f, 1.0f));
	mat4f translation = translation_matrix(vec3f(.5f, -.5f, 1.f));

	return view * proj * scale * translation;
}

#endif
