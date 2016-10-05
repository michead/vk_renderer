#pragma once

#include <map>
#include <vector>

#include "VkEngine.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"

#include "json11\json11.hpp"

#define MAX_NUM_LIGHTS	4
#define PATH_SEPARATOR	'/'
#define SCENE_FILENAME	"scene.json"


struct Camera;


struct Scene {
public:
	Scene(std::string path) : path(path.substr(0, path.rfind(PATH_SEPARATOR) + 1)), 
		filename(path.substr(path.rfind(PATH_SEPARATOR) + 1)) { load(); }
	~Scene() { cleanup(); }

	std::vector<Mesh*>& getMeshes() { return elems; }
	std::vector<Material*>& getMaterials() { return materials; }
	std::vector<Light*>& getLights() { return lights; }
	glm::vec3& getAmbient() { return ambient; }
	Camera* getCamera() const { return camera; }
	std::map<std::string, Texture*>& getTextureMap() { return textureMap; }

private:
	std::string filename;
	std::string path;
	std::vector<Mesh*> elems;
	std::vector<Material*> materials;
	std::map<std::string, Texture*> textureMap;
	std::vector<Light*> lights;
	glm::vec3 ambient;
	Camera* camera;
	
	void load();
	void cleanup();

	void initCamera(json11::Json);
	void loadLights(std::vector<json11::Json>);
	void loadObjMesh(json11::Json);
	void loadBinMesh(json11::Json);
};