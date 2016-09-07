#include "Scene.h"

#include "Camera.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader\tiny_obj_loader.h"


void Scene::load()
{
	tinyobj::attrib_t attrib_;
	std::vector<tinyobj::shape_t> shapes_;
	std::vector<tinyobj::material_t> materials_;
	std::string err_;

	if (!tinyobj::LoadObj(&attrib_, &shapes_, &materials_, &err_, path.c_str(), path.substr(path.rfind('/') + 1).c_str()))
	{
		throw std::runtime_error(err_);
	}

	uint16_t i = 0;
	for (const tinyobj::material_t material : materials_)
	{
		if (!textureMap[material.diffuse_texname])
		{
			textureMap.insert(std::make_pair(material.diffuse_texname, new Texture(material.diffuse_texname)));
		}
	}

	i = 0;
	for (const tinyobj::material_t material : materials_)
	{
		materials[i].kd = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
		materials[i].ks = { material.specular[0], material.specular[1], material.specular[2] };
		materials[i].ke = { material.emission[0], material.emission[1], material.emission[2] };

		materials[i].kdTxt = textureMap[material.diffuse_texname];
		materials[i].ksTxt = textureMap[material.specular_texname];
		materials[i].keTxt = textureMap[material.emissive_texname];
	}

	elems.resize(shapes_.size());

	i = 0;
	for (const tinyobj::shape_t& shape : shapes_)
	{
		SceneElem elem = elems[i];
		std::unordered_map<Vertex, int> uniqueVertices = {};

		elem.name = shape.name;

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

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = elem.mesh.vertices.size();
				elem.mesh.vertices.push_back(vertex);
			}

			elem.mesh.indices.push_back(uniqueVertices[vertex]);
		}

		i++;
	}
}

void Scene::cleanup()
{
	std::unordered_map<std::string, Texture*>::iterator it;
	for (it = textureMap.begin(); it != textureMap.end(); it++)
	{
		delete it->second;
		textureMap.erase(it);
	}
}