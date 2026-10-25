#pragma once
#include <glew.h>

class Texture
{
public:
	Texture();
	Texture(const char* FileLoc);
	bool LoadTexture();
	bool LoadTextureA();

	// Permite cargar texturas desde búferes de memoria (clave para .glb / glTF)
	bool LoadTextureFromMemory(const unsigned char* buffer, int length, bool hasAlpha = true);

	void UseTexture();
	void ClearTexture();
	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;
	const char* fileLocation;
};