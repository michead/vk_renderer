#pragma once

#include <unordered_map>

#include "VkEngine.h"
#include "SceneElem.h"
#include "GeomStructs.h"
#include "Texture.h"

#define PATH_SEPARATOR '/'


struct Camera;


struct Scene {
public:
	Scene(std::string path) : path(path.substr(0, path.rfind(PATH_SEPARATOR) + 1)), 
		filename(path.substr(path.rfind(PATH_SEPARATOR) + 1)) { load(); initCamera(); }
	~Scene() { cleanup(); }

	std::vector<SceneElem>& getElems() { return elems; }
	std::vector<Material>& getMaterials() { return materials; }
	std::vector<Light>& getLights() { return lights; }
	Camera* getCamera() { return camera; }
	std::unordered_map<std::string, Texture*>& getTextureMap() { return textureMap; }

private:
	std::string filename;
	std::string path;
	std::vector<SceneElem> elems;
	std::vector<Material> materials;
	std::unordered_map<std::string, Texture*> textureMap;
	std::vector<Light> lights;
	Camera* camera;
	
	void load();
	void initCamera();
	void cleanup();
};