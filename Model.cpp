#include "Model.h"
#include <unordered_map>
#include <cmath>
#include <gtc/matrix_transform.hpp>

Model::Model()
{
}

Model::Model(Model&& other) noexcept
{
	MeshList = std::move(other.MeshList);
	TextureList = std::move(other.TextureList);
	meshTotex = std::move(other.meshTotex);

	other.MeshList.clear();
	other.TextureList.clear();
	other.meshTotex.clear();
}

Model& Model::operator=(Model&& other) noexcept
{
	if (this != &other)
	{
		ClearModel();

		MeshList = std::move(other.MeshList);
		TextureList = std::move(other.TextureList);
		meshTotex = std::move(other.meshTotex);

		other.MeshList.clear();
		other.TextureList.clear();
		other.meshTotex.clear();
	}
	return *this;
}

void Model::LoadModel(const std::string& fileName)
{
	unsigned int flags = aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_CalcTangentSpace;

	const aiScene* scene = importer.ReadFile(fileName, flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		printf("Falló en cargar el modelo: %s | Error: %s \n", fileName.c_str(), importer.GetErrorString());
		return;
	}

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
}

void Model::ClearModel()
{
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		if (MeshList[i]) { delete MeshList[i]; MeshList[i] = nullptr; }
	}
	MeshList.clear();

	for (size_t i = 0; i < TextureList.size(); i++)
	{
		if (TextureList[i]) { delete TextureList[i]; TextureList[i] = nullptr; }
	}
	TextureList.clear();
	meshTotex.clear();
}

void Model::RenderModel()
{
	for (size_t i = 0; i < MeshList.size(); i++)
	{
		if (MeshList[i] == nullptr) continue;

		if (i < meshTotex.size())
		{
			unsigned int materialIndex = meshTotex[i];
			if (materialIndex < TextureList.size() && TextureList[materialIndex] != nullptr)
			{
				TextureList[materialIndex]->UseTexture();
			}
		}

		MeshList[i]->RenderMeshModel();
	}
}

// Raycasting dinámico con transformación de mapa sincronizada
bool Model::GetRayIntersectionY(float x, float z, float& outY) const
{
	glm::vec3 rayOrigin(x, 200.0f, z);
	glm::vec3 rayDir(0.0f, -1.0f, 0.0f);

	const float toRad = 3.14159265f / 180.0f;

	// Matriz de transformación del mapa (Debe coincidir con la de E09-318266080.cpp)
	glm::mat4 mapTransform(1.0f);
	mapTransform = glm::rotate(mapTransform, -90.0f * toRad, glm::vec3(1.0f, 0.0f, 0.0f)); // Eje X
	mapTransform = glm::rotate(mapTransform, 0.0f * toRad, glm::vec3(0.0f, 1.0f, 0.0f));   // Eje Y

	float highestY = -99999.0f;
	bool hit = false;
	const float rangoDistancia = 15.0f;

	for (size_t m = 0; m < MeshList.size(); m++)
	{
		if (!MeshList[m]) continue;

		const auto& vertices = MeshList[m]->GetVertices();
		const auto& indices = MeshList[m]->GetIndices();

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			unsigned int idx0 = indices[i] * 8;
			unsigned int idx1 = indices[i + 1] * 8;
			unsigned int idx2 = indices[i + 2] * 8;

			glm::vec4 rawV0(vertices[idx0], vertices[idx0 + 1], vertices[idx0 + 2], 1.0f);
			glm::vec3 v0 = glm::vec3(mapTransform * rawV0);

			if (std::abs(v0.x - x) > rangoDistancia || std::abs(v0.z - z) > rangoDistancia)
			{
				continue;
			}

			glm::vec4 rawV1(vertices[idx1], vertices[idx1 + 1], vertices[idx1 + 2], 1.0f);
			glm::vec4 rawV2(vertices[idx2], vertices[idx2 + 1], vertices[idx2 + 2], 1.0f);

			glm::vec3 v1 = glm::vec3(mapTransform * rawV1);
			glm::vec3 v2 = glm::vec3(mapTransform * rawV2);

			glm::vec3 edge1 = v1 - v0;
			glm::vec3 edge2 = v2 - v0;
			glm::vec3 h = glm::cross(rayDir, edge2);
			float a = glm::dot(edge1, h);

			if (a > -0.00001f && a < 0.00001f) continue;

			float f = 1.0f / a;
			glm::vec3 s = rayOrigin - v0;
			float u = f * glm::dot(s, h);

			if (u < 0.0f || u > 1.0f) continue;

			glm::vec3 q = glm::cross(s, edge1);
			float v = f * glm::dot(rayDir, q);

			if (v < 0.0f || u + v > 1.0f) continue;

			float t = f * glm::dot(edge2, q);

			if (t > 0.00001f)
			{
				float intersectionY = rayOrigin.y - t;
				if (intersectionY > highestY)
				{
					highestY = intersectionY;
					hit = true;
				}
			}
		}
	}

	if (hit)
	{
		outY = highestY;
		return true;
	}
	return false;
}

Model::~Model()
{
	ClearModel();
}

void Model::LoadNode(const aiNode* node, const aiScene* scene)
{
	if (!node || !scene) return;

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(const aiMesh* mesh, const aiScene* scene)
{
	if (!mesh) return;

	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		vertices.insert(vertices.end(), { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z });

		if (mesh->mTextureCoords[0])
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		else
			vertices.insert(vertices.end(), { 0.0f, 0.0f });

		if (mesh->HasNormals())
			vertices.insert(vertices.end(), { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z });
		else
			vertices.insert(vertices.end(), { 0.0f, 1.0f, 0.0f });
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	MeshModel* newMesh = new MeshModel();
	newMesh->CreateMeshModel(&vertices[0], &indices[0], (unsigned int)vertices.size(), (unsigned int)indices.size());
	MeshList.push_back(newMesh);
	meshTotex.push_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene* scene)
{
	std::unordered_map<std::string, Texture*> loadedTextures;
	TextureList.resize(scene->mNumMaterials, nullptr);

	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		const aiMaterial* material = scene->mMaterials[i];
		TextureList[i] = nullptr;

		aiTextureType type = aiTextureType_DIFFUSE;
		if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0 && material->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
			type = aiTextureType_BASE_COLOR;

		if (material->GetTextureCount(type) > 0)
		{
			aiString path;
			if (material->GetTexture(type, 0, &path) == AI_SUCCESS)
			{
				std::string texPathStr = path.C_Str();

				const aiTexture* embeddedTexture = nullptr;

				if (texPathStr.size() > 0 && texPathStr[0] == '*')
				{
					int texIndex = std::atoi(texPathStr.substr(1).c_str());
					if (texIndex >= 0 && (unsigned int)texIndex < scene->mNumTextures)
						embeddedTexture = scene->mTextures[texIndex];
				}
				else
				{
					embeddedTexture = scene->GetEmbeddedTexture(texPathStr.c_str());
				}

				if (embeddedTexture)
				{
					Texture* newTex = new Texture();
					bool loaded = false;

					if (embeddedTexture->mHeight == 0)
					{
						loaded = newTex->LoadTextureFromMemory(
							reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
							embeddedTexture->mWidth,
							true
						);
					}
					else
					{
						loaded = newTex->LoadTextureFromMemory(
							reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
							embeddedTexture->mWidth * embeddedTexture->mHeight * 4,
							true
						);
					}

					if (loaded) TextureList[i] = newTex;
					else { delete newTex; TextureList[i] = nullptr; }
				}
				else
				{
					std::string filename = texPathStr;
					size_t lastSlash = filename.find_last_of("/\\");
					if (lastSlash != std::string::npos) filename = filename.substr(lastSlash + 1);

					std::string fullPath = std::string("Textures/") + filename;

					auto it = loadedTextures.find(fullPath);
					if (it != loadedTextures.end()) TextureList[i] = it->second;
					else
					{
						Texture* newTex = new Texture(fullPath.c_str());
						std::string ext = filename.substr(filename.find_last_of('.') + 1);

						bool loaded = (ext == "tga" || ext == "png") ? newTex->LoadTextureA() : newTex->LoadTexture();

						if (loaded)
						{
							TextureList[i] = newTex;
							loadedTextures[fullPath] = newTex;
						}
						else { delete newTex; TextureList[i] = nullptr; }
					}
				}
			}
		}

		if (TextureList[i] == nullptr)
		{
			TextureList[i] = new Texture("Textures/plain.png");
			if (!TextureList[i]->LoadTextureA())
			{
				delete TextureList[i];
				TextureList[i] = nullptr;
			}
		}
	}
}