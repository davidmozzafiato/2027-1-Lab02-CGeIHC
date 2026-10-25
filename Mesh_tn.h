#pragma once

#include <glew.h>
#include <vector>
#include <glm.hpp>

class MeshModel
{
public:
	MeshModel();

	void CreateMeshModel(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMeshModel();
	void ClearMeshModel();

	// Acceso a datos para Raycasting
	const std::vector<GLfloat>& GetVertices() const { return verticesData; }
	const std::vector<unsigned int>& GetIndices() const { return indicesData; }

	~MeshModel();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;

	std::vector<GLfloat> verticesData;
	std::vector<unsigned int> indicesData;
};