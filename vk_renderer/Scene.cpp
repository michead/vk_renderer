#include "Scene.h"

#include <algorithm>
#include <string>
#include <unordered_map>

#include "Camera.h"
#include "MathUtils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader\tiny_obj_loader.h"
#include "json11\json11.hpp"


void Scene::load()
{
	std::string fileExtension = filename.substr(filename.find('.') + 1);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

	if (fileExtension == "obj") loadObjMesh();
	else if (fileExtension == "mesh") loadBinMesh();
	else throw std::runtime_error("Unsupported mesh format.");

	loadLights();
}

void Scene::loadObjMesh(bool onlyMtl)
{
	tinyobj::attrib_t attrib_;
	std::vector<tinyobj::shape_t> shapes_;
	std::vector<tinyobj::material_t> materials_;
	std::string err_;

	std::string filename_ = filename;
	if (onlyMtl)
	{
		int fileExtStart = filename.find('.');
		filename_ = filename.substr(0, fileExtStart) + ".OBJ";
	}

	if (!tinyobj::LoadObj(&attrib_, &shapes_, &materials_, &err_, (path + filename_).c_str(), path.c_str()))
	{
		throw std::runtime_error(err_);
	}

	uint16_t i = 0;
	for (const tinyobj::material_t material : materials_)
	{
		textureMap[material.diffuse_texname] = new Texture(path + material.diffuse_texname);

		if (!material.normal_texname.empty())
		{
			textureMap[material.normal_texname] = new Texture(path + material.normal_texname);
		}
	}

	i = 0;
	materials.resize(materials_.size());
	for (const tinyobj::material_t material : materials_)
	{
		materials[i] = new Material();
		materials[i]->id = i;

		materials[i]->kd = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
		materials[i]->ks = { material.specular[0], material.specular[1], material.specular[2] };
		materials[i]->ns = material.shininess;

		if (textureMap.find(material.diffuse_texname) != textureMap.end())
			materials[i]->kdMap = textureMap[material.diffuse_texname];
		if (textureMap.find(material.specular_texname) != textureMap.end())
			materials[i]->ksMap = textureMap[material.specular_texname];
		if (textureMap.find(material.normal_texname) != textureMap.end())
			materials[i]->normalMap = textureMap[material.normal_texname];
	}

	if (onlyMtl)
		return;

	elems.resize(shapes_.size());

	i = 0;
	for (const tinyobj::shape_t& shape : shapes_)
	{
		std::unordered_map<Vertex, int> uniqueVertices = {};

		elems[i] = new Mesh();
		elems[i]->name = shape.name;
		elems[i]->material = materials[shape.mesh.material_ids.front()];

		std::vector<float> tangents;
		std::vector<float> normals;

		if (attrib_.normals.empty())
		{
			normals = fComputeVertexNormals(
				attrib_.vertices.size(), 
				attrib_.vertices.data(), 
				(int*)shape.mesh.indices.data());
			tangents = fComputeTangents(
				attrib_.vertices.size(), 
				attrib_.vertices.data(), 
				attrib_.normals.data(), 
				attrib_.texcoords.data(), 
				(int*)shape.mesh.indices.data());
		}
		else
		{
			normals = attrib_.normals;
		}

		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex = {};

			vertex.position = {
				attrib_.vertices[3 * index.vertex_index + 0],
				attrib_.vertices[3 * index.vertex_index + 1],
				attrib_.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib_.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib_.texcoords[2 * index.texcoord_index + 1]
			};

			if (!attrib_.normals.empty())
			{
				vertex.normal = {
					attrib_.normals[3 * index.vertex_index + 0],
					attrib_.normals[3 * index.vertex_index + 1],
					attrib_.normals[3 * index.vertex_index + 2]
				};
			}

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = elems[i]->vertices.size();
				elems[i]->vertices.push_back(vertex);
			}

			elems[i]->indices.push_back(uniqueVertices[vertex]);
		}

		i++;
	}
}

void Scene::loadBinMesh()
{
	FILE *f;
	if (fopen_s(&f, (path + filename).c_str(), "rb"))
	{
		std::cerr << "File could not be opened." << std::endl;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<float> radiuses;
	std::vector<glm::vec3> colors;
	std::vector<glm::vec3> velocities;
	std::vector<glm::ivec3> triangles;
	std::vector<glm::ivec2> lines;
	std::vector<int> points;
	std::vector<glm::ivec4> quads;
	std::vector<glm::ivec4> splines;

	loadVec(f, positions);
	loadVec(f, normals);
	loadVec(f, texCoords);
	loadVec(f, radiuses);
	loadVec(f, colors);
	loadVec(f, velocities);
	loadVec(f, triangles);
	loadVec(f, lines);
	loadVec(f, points);
	loadVec(f, quads);
	loadVec(f, splines);

	if (normals.empty()) normals = computeVertexNormals(positions, triangles);
	std::vector<glm::vec3> tangents = computeTangents(positions, normals, texCoords, triangles);

	elems.push_back(new Mesh());
	loadObjMesh(true);
	elems[0]->material = materials[0];

	std::unordered_map<Vertex, int> uniqueVertices = {};

	for (const auto& face : triangles)
	{
		for (int i = 0; i < 3; i++)
		{
			Vertex vertex = {};

			vertex.position = {
				positions[face[i]].x,
				positions[face[i]].y,
				positions[face[i]].z
			};

			vertex.texCoord = {
				texCoords[face[i]].x,
				1.f - texCoords[face[i]].y
			};

			if (!normals.empty())
			{
				vertex.normal = {
					normals[face[i]].x,
					normals[face[i]].y,
					normals[face[i]].z
				};
			}

			if (!colors.empty())
			{
				vertex.color = {
					colors[face[i]].x,
					colors[face[i]].y,
					colors[face[i]].z,
					1.f
				};
			}

			if (!tangents.empty())
			{
				vertex.tangent = {
					tangents[face[i]].x,
					tangents[face[i]].y,
					tangents[face[i]].z
				};
			}

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = elems[0]->vertices.size();
				elems[0]->vertices.push_back(vertex);
			}

			elems[0]->indices.push_back(uniqueVertices[vertex]);
		}
	}

	fclose(f);
}

void Scene::initCamera()
{
	camera = new Camera();

	glm::vec3 position;
	glm::vec3 target;

	std::ifstream f(path + CAMERA_FILENAME);

	if (f.good())
	{
		std::stringstream buffer;
		buffer << f.rdbuf();

		std::string s = buffer.str();
		std::string err;

		json11::Json json = json11::Json::parse(s, err);

		if (!err.empty())
		{
			throw std::runtime_error(err);
		}

		if (json.has_member("position"))
		{
			std::vector<json11::Json> jsonPosArray = json["position"].array_items();
			position = { 
				jsonPosArray[0].number_value(), 
				jsonPosArray[1].number_value(), 
				jsonPosArray[2].number_value() };
		}
		else
		{
			camera->frame.origin = CAMERA_POSITION;
		}

		if (json.has_member("center"))
		{
			std::vector<json11::Json> jsonPosArray = json["center"].array_items();
			target = {
				jsonPosArray[0].number_value(),
				jsonPosArray[1].number_value(),
				jsonPosArray[2].number_value() };
		}
		else
		{
			camera->target = CAMERA_TARGET;
		}
	}

	camera->frame = Frame::lookAtFrame(position, target, CAMERA_UP);
	camera->target = target;
	camera->fovy = CAMERA_FOVY;
}

void Scene::loadLights()
{
	std::ifstream f(path + LIGHTS_FILENAME);

	if (f.good())
	{
		std::stringstream buffer;
		buffer << f.rdbuf();

		std::string s = buffer.str();
		std::string err;

		json11::Json json = json11::Json::parse(s, err);

		if (!err.empty())
		{
			throw std::runtime_error(err);
		}

		if (json.has_member("lights"))
		{
			const std::vector<json11::Json> jsonLights = json["lights"].array_items();

			for (const json11::Json jsonLight : jsonLights)
			{
				Light* light = new Light();

				if (jsonLight.has_member("position"))
				{
					std::vector<json11::Json> jsonPosArray = jsonLight["position"].array_items();
					light->position = {
						jsonPosArray[0].number_value(),
						jsonPosArray[1].number_value(),
						jsonPosArray[2].number_value() };
				}

				if (jsonLight.has_member("ke"))
				{
					std::vector<json11::Json> jsonColorArray = jsonLight["ke"].array_items();
					light->intensity = {
						jsonColorArray[0].number_value(),
						jsonColorArray[1].number_value(),
						jsonColorArray[2].number_value() };
				}

				lights.push_back(light);
			}
		}
	}
}

void Scene::cleanup()
{
	std::vector<Mesh*>::iterator it3;
	for (it3 = elems.begin(); it3 != elems.end(); it3++)
	{
		delete *it3;
	}

	std::map<std::string, Texture*>::iterator it;
	for (it = textureMap.begin(); it != textureMap.end(); it++)
	{
		delete it->second;
	}

	std::vector<Material*>::iterator it2;
	for (it2 = materials.begin(); it2 != materials.end(); it2++)
	{
		delete *it2;
	}

	std::vector<Light*>::iterator it4;
	for (it4 = lights.begin(); it4 != lights.end(); it4++)
	{
		delete *it4;
	}
}