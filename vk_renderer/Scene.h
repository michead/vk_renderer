#pragma once

#include <map>

#include "VkEngine.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"

#define MAX_NUM_LIGHTS	8
#define PATH_SEPARATOR	'/'
#define LIGHTS_FILENAME	"lights.json"


struct Camera;


struct Scene {
public:
	Scene(std::string path) : path(path.substr(0, path.rfind(PATH_SEPARATOR) + 1)), 
		filename(path.substr(path.rfind(PATH_SEPARATOR) + 1)) { load(); initCamera(); }
	~Scene() { cleanup(); }

	std::vector<Mesh*>& getMeshes() { return elems; }
	std::vector<Material*>& getMaterials() { return materials; }
	std::vector<Light*>& getLights() { return lights; }
	Camera* getCamera() const { return camera; }
	std::map<std::string, Texture*>& getTextureMap() { return textureMap; }

private:
	std::string filename;
	std::string path;
	std::vector<Mesh*> elems;
	std::vector<Material*> materials;
	std::map<std::string, Texture*> textureMap;
	std::vector<Light*> lights;
	Camera* camera;
	
	void load();
	void initCamera();
	void loadLights();
	void cleanup();

	void loadObjMesh();
	void loadBinMesh();
};