#pragma once

#include "Common.h"
#include "Camera.h"
#include "SceneElem.h"
#include "GeomStructs.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader\tiny_obj_loader.h"

class Scene {
public:
	Scene(std::string path) : path(path) { load(); }
	~Scene() { cleanup(); }

	std::vector<SceneElem>& getElems() { return elems; }
	std::vector<Material>& getMaterials() { return materials; }
	std::vector<Light>& getLights() { return lights; }
	Camera& getCamera() { return camera; }
	std::unordered_map<std::string, Texture*>& getTextureMap() { return textureMap; }

private:
	std::string path;
	std::vector<SceneElem> elems;
	std::vector<Material> materials;
	std::unordered_map<std::string, Texture*> textureMap;
	std::vector<Light> lights;
	Camera camera;
	
	void load();
	void cleanup();
};