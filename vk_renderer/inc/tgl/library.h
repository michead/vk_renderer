#ifndef _SCENE_H_
#define _SCENE_H_

#include "common.h"
#include "vmath.h"
#include "json.h"

#include "elem.h"
#include "camera.h"
#include "environment.h"

// Library of components --- mostly used in IO
struct Library {
    // main objects
    vector<Camera*>         cameras;        // cameras
    vector<Elem*>           elems;          // elems
    vector<Environment*>    environments;   // environments

    // default objects
    Camera*                 camera = nullptr;       // current camera
    Environment*            environment = nullptr;  // current environment
    
    // elem objects
    vector<Texture*>        textures;       // textures
    vector<Mesh*>           meshes;         // meshes
    vector<Material*>       materials;      // materials
};

struct TupleSSF {
    string s1;
    string s2;
    float f;
};

struct TupleIII {
    int i1;
    int i2;  
    int i3;
};

// prune unused objects
void prune(Library* lib);

// asset loading
void load_textures(Library* lib, const string& dirname, bool gamma);
void load_meshes(Library* lib, const string& filename);
void load_binmesh_data(Mesh* mesh, const string& filename);

// asset saving
void save_meshes(const Library* lib, const string& dirname);
void save_textures(const Library* lib, const string& dirname, bool gamma);

// save a library
void save_library(const Library* lib, const string& filename,
                  bool external_meshes, bool external_textures, bool texture_gamma);
// load a library from a file
Library* load_library(const string& filename, bool pruned,
                      bool external_meshes, bool external_textures,
                      bool texture_gamma,
                      bool preserve_topology, bool verbose);
// load library with simpler default settings
inline Library* load_library(const string& filename, bool gamma) {
    return load_library(filename, true, true, true, gamma, false, false);
}
// load library without externals
Library* load_library(const string& filename, bool preserve_topology, bool verbose, Library* ref);

// save a mesh
void save_mesh(const Mesh* mesh, const string& filename);
// load a mesh
Mesh* load_mesh(const string& filename, bool preserve_topology = false, bool verbose = false);

// should not be used directly
json library_to_json(const Library* lib, const set<string>& includes);
json library_to_json(const Library* lib);

#endif

