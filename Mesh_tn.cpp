#include "Mesh_tn.h"

MeshModel::MeshModel()
{
	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;
}

void MeshModel::CreateMeshModel(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	indexCount = numOfIndices;

	// Guardar copias en memoria CPU para Raycasting
	verticesData.assign(vertices, vertices + numOfVertices);
	indicesData.assign(indices, indices + numOfIndices);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);

	// Posiciones (Attribute 0 - 3 floats)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);

	// UVs (Attribute 1 - 2 floats)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 3));
	glEnableVertexAttribArray(1);

	// Normales (Attribute 2 - 3 floats)
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 5));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void MeshModel::RenderMeshModel()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void MeshModel::ClearMeshModel()
{
	if (IBO != 0) { glDeleteBuffers(1, &IBO); IBO = 0; }
	if (VBO != 0) { glDeleteBuffers(1, &VBO); VBO = 0; }
	if (VAO != 0) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
	indexCount = 0;
	verticesData.clear();
	indicesData.clear();
}

MeshModel::~MeshModel()
{
	ClearMeshModel();
}