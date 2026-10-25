#pragma once

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh_tn.h"
#include "Texture.h"

class Model
{
public:
	Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	Model(Model&& other) noexcept;
	Model& operator=(Model&& other) noexcept;

	void LoadModel(const std::string& fileName);
	void RenderModel();
	void ClearModel();

	// Raycasting Rayo-Triángulo (Devuelve true y la altura Y más alta colisionada)
	bool GetRayIntersectionY(float x, float z, float& outY) const;

	~Model();

private:
	void LoadNode(const aiNode* node, const aiScene* scene);
	void LoadMesh(const aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);

	Assimp::Importer importer;

	std::vector<MeshModel*> MeshList;
	std::vector<Texture*> TextureList;
	std::vector<unsigned int> meshTotex;
};