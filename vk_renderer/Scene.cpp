#include "Scene.h"

#include <algorithm>
#include <string>
#include <unordered_map>

#include "Camera.h"
#include "Frame.h"
#include "MathUtils.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader\tiny_obj_loader.h"


void Scene::load()
{
	std::string fileExtension = filename.substr(filename.find('.') + 1);
	std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

	std::ifstream f(path + SCENE_FILENAME);

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

		json11::Json scene = json["scene"];

		json11::Json cameraNode = scene["camera"];
		initCamera(cameraNode);

		std::vector<json11::Json> jsonMeshes = scene["meshes"].array_items();
		for (const auto& jsonMesh : jsonMeshes)
		{
			// loadObjMesh(jsonMesh);
			loadBinMesh(jsonMesh);
		}

		std::vector<json11::Json> lightsNode = scene["lights"].array_items();
		loadLights(lightsNode);

		std::vector<json11::Json> ambientNode = scene["ka"].array_items();
		ambient = { 
			ambientNode[0].number_value(), 
			ambientNode[1].number_value(), 
			ambientNode[2].number_value() };
	}
	else
	{
		throw std::runtime_error("Scene could not be parsed.");
	}
}

void Scene::loadObjMesh(json11::Json jsonMesh)
{
	tinyobj::attrib_t attrib_;
	std::vector<tinyobj::shape_t> shapes_;
	std::vector<tinyobj::material_t> materials_;
	std::string err_;

	std::string meshFilename = jsonMesh["filename"].string_value();
	
	json11::Json jsonMaterial = jsonMesh["material"];
	float translucency = jsonMaterial["translucency"].number_value();
	float sswidth = jsonMaterial["ss_width"].number_value();

	if (!tinyobj::LoadObj(&attrib_, &shapes_, &materials_, &err_, (path + meshFilename).c_str(), path.c_str()))
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

		materials[i]->translucency = translucency;
		materials[i]->subsurfWidth = sswidth;

		if (textureMap.find(material.diffuse_texname) != textureMap.end())
			materials[i]->kdMap = textureMap[material.diffuse_texname];
		if (textureMap.find(material.specular_texname) != textureMap.end())
			materials[i]->ksMap = textureMap[material.specular_texname];
		if (textureMap.find(material.normal_texname) != textureMap.end())
			materials[i]->normalMap = textureMap[material.normal_texname];
	}

	i = 0;
	for (const tinyobj::shape_t& shape : shapes_)
	{
		std::unordered_map<Vertex, int> uniqueVertices = {};

		elems.push_back(new Mesh());
		elems.back()->name = shape.name;
		elems.back()->material = materials[shape.mesh.material_ids.front()];

		std::vector<float> tangents;
		std::vector<float> normals;
		std::vector<int> indices;

		for (const auto& index : shape.mesh.indices)
		{
			indices.push_back(index.vertex_index);
		}

		if (attrib_.normals.empty())
		{
			normals = fComputeVertexNormals(
				attrib_.vertices.size(),
				indices.size(),
				attrib_.vertices.data(), 
				indices.data());
			tangents = fComputeTangents(
				attrib_.vertices.size(),
				indices.size(),
				attrib_.vertices.data(), 
				normals.data(), 
				attrib_.texcoords.data(), 
				indices.data());
		}
		else
		{
			normals = attrib_.normals;
		}

		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex = {};

			vertex.position = {
				attrib_.vertices[3 * index.vertex_index],
				attrib_.vertices[3 * index.vertex_index + 1],
				attrib_.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib_.texcoords[2 * index.texcoord_index],
				1.f - attrib_.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal = {
				normals[3 * index.vertex_index],
				normals[3 * index.vertex_index + 1],
				normals[3 * index.vertex_index + 2]
			};

			vertex.tangent = {
				tangents[3 * index.vertex_index],
				tangents[3 * index.vertex_index + 1],
				tangents[3 * index.vertex_index + 2]
			};

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = elems.back()->vertices.size();
				elems.back()->vertices.push_back(vertex);
			}

			elems.back()->indices.push_back(uniqueVertices[vertex]);
		}

		i++;
	}
}

/**
 * No support for materials yet
 */
void Scene::loadBinMesh(json11::Json jsonMesh)
{
	Material* material = new Material();

	json11::Json jsonMaterial = jsonMesh["material"];
	
	float translucency = jsonMaterial["translucency"].number_value();
	float ssWidth = jsonMaterial["ss_width"].number_value();
	std::vector<json11::Json> jsonKs = jsonMaterial["ks"].array_items();
	glm::vec3 ks = { jsonKs[0].number_value(), jsonKs[1].number_value(), jsonKs[2].number_value() };
	float ns = jsonMaterial["ns"].number_value();

	material->translucency = translucency;
	material->subsurfWidth = ssWidth;
	material->ks = ks;
	material->ns = ns;

	std::string kdTxt = jsonMaterial["kd_txt"].string_value();
	std::string normTxt = jsonMaterial["norm_txt"].string_value();

	textureMap[kdTxt] = new Texture(path + kdTxt);
	textureMap[normTxt] = new Texture(path + normTxt);

	material->kdMap = textureMap[kdTxt];
	material->normalMap = textureMap[normTxt];

	materials.push_back(material);

	std::string meshFilename = jsonMesh["filename"].string_value();

	FILE *f;
	if (fopen_s(&f, (path + meshFilename).c_str(), "rb"))
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

	Mesh* mesh = new Mesh();
	mesh->material = materials.front();

	if (normals.empty()) normals = computeVertexNormals(positions, triangles);
	std::vector<glm::vec3> tangents = computeTangents(positions, normals, texCoords, triangles);

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
				uniqueVertices[vertex] = mesh->vertices.size();
				mesh->vertices.push_back(vertex);
			}

			mesh->indices.push_back(uniqueVertices[vertex]);
		}
	}

	if (jsonMesh.has_member("position"))
	{
		std::vector<json11::Json> jsonO = jsonMesh["position"].array_items();
		mesh->frame.origin = {
			jsonO[0].number_value(),
			jsonO[1].number_value(),
			jsonO[2].number_value() };
	}

	if (jsonMesh.has_member("z"))
	{
		std::vector<json11::Json> jsonZ = jsonMesh["z"].array_items();
		mesh->frame.zAxis = {
			jsonZ[0].number_value(),
			jsonZ[1].number_value(),
			jsonZ[2].number_value() };
	}

	mesh->frame = Frame::orthonormalizeF(mesh->frame);

	elems.push_back(mesh);

	fclose(f);
}

void Scene::initCamera(json11::Json cameraNode)
{
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	
	if (cameraNode.has_member("position"))
	{
		std::vector<json11::Json> jsonPosArray = cameraNode["position"].array_items();
		position = { 
			jsonPosArray[0].number_value(), 
			jsonPosArray[1].number_value(), 
			jsonPosArray[2].number_value() };
	}
	else
	{
		position = CAMERA_POSITION;
	}

	if (cameraNode.has_member("center"))
	{
		std::vector<json11::Json> jsonPosArray = cameraNode["center"].array_items();
		target = {
			jsonPosArray[0].number_value(),
			jsonPosArray[1].number_value(),
			jsonPosArray[2].number_value() };
	}
	else
	{
		target = CAMERA_TARGET;
	}

	if (cameraNode.has_member("up"))
	{
		std::vector<json11::Json> jsonPosArray = cameraNode["up"].array_items();
		up = {
			jsonPosArray[0].number_value(),
			jsonPosArray[1].number_value(),
			jsonPosArray[2].number_value() };
	}
	else
	{
		up = CAMERA_UP;
	}

	camera = new Camera(position, up, CAMERA_FOVY, target);
}

void Scene::loadLights(std::vector<json11::Json> lightsNode)
{
	for (const json11::Json jsonLight : lightsNode)
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