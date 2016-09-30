#pragma once

struct Mesh_ {
    // type
    enum struct Type : int { TRIANGLE = 0, LINE = 1, POINT = 2 };
    
    // full mesh properties
    std::string         name = "";                  // name
    Type                type = Type::TRIANGLE;      // type
    std::string         filename = "";              // filename
    
    // vertex properties
    std::vector<vec3f>   pos;                        // vertex position
    std::vector<vec3f>   norm;                       // vertex normal
    std::vector<vec2f>   texcoord;                   // vertex texture coordinates
    std::vector<float>   radius;                     // vertex radius (lines / points)
    std::vector<vec3f>   col;                        // vertex colors
    std::vector<vec3f>   vel;                        // vertex velocity
    
    // tangent space --- computed on the fly
    std::vector<vec3f>   tang_sp;                    // vertex tangents
    
    // elements
    std::vector<vec3i>   triangle;                   // triangle
    std::vector<vec2i>   line;                       // line
    std::vector<int>     point;                      // point
    
    // elements to be tesselated
    std::vector<vec4i>   quad;                       // quads
    std::vector<vec4i>   spline;                     // spline
};

// asset loading
void load_binmesh_data(Mesh_* mesh, const string& filename);

#endif

