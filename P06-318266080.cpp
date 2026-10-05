/*
Pr�ctica 6: Texturizado
*/
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <math.h>

#include <glew.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_m.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"
#include "Skybox.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshListColor;
std::vector<MeshModel*> meshListModel;

std::vector<Shader> shaderList;

Camera camera;

Texture plainTexture;
Texture pisoTexture;
Texture dadoTexture;
Texture holocronTexture;
Texture avionTexture;

Model Kitt_M;
Model Llanta_M;
Model Dado_M;
Model Holocron_M;
Model BizarrePlane_M; // Ejercicio 3: Avi�n personaje estilo Cars/Planes

Skybox skybox;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Shaders
static const char* vShader = "shaders/shader_texture.vert";
static const char* fShader = "shaders/shader_texture.frag";

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	unsigned int vegetacionIndices[] = {
		0, 1, 2,
		0, 2, 3,
		4, 5, 6,
		4, 6, 7
	};

	GLfloat vegetacionVertices[] = {
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.0f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,

		0.0f, -0.5f, -0.5f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.5f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.5f,		1.0f, 1.0f,		0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, -0.5f,		0.0f, 1.0f,		0.0f, 0.0f, 0.0f,
	};

	MeshModel* obj1 = new MeshModel();
	obj1->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj1);

	MeshModel* obj2 = new MeshModel();
	obj2->CreateMeshModel(vertices, indices, 32, 12);
	meshListModel.push_back(obj2);

	MeshModel* obj3 = new MeshModel();
	obj3->CreateMeshModel(floorVertices, floorIndices, 32, 6);
	meshListModel.push_back(obj3);

	MeshModel* obj4 = new MeshModel();
	obj4->CreateMeshModel(vegetacionVertices, vegetacionIndices, 64, 12);
	meshListModel.push_back(obj4);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

void CrearDado()
{
	unsigned int cubo_indices[] = {
		// front
		0, 1, 2,
		2, 3, 0,

		// back
		8, 9, 10,
		10, 11, 8,

		// left
		12, 13, 14,
		14, 15, 12,

		// bottom
		16, 17, 18,
		18, 19, 16,

		// top
		20, 21, 22,
		22, 23, 20,

		// right
		4, 5, 6,
		6, 7, 4,
	};

	GLfloat cubo_vertices[] = {
		// FRONT
		-0.5f, -0.5f,  0.5f,    0.064302f, 0.209996f,    0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,    0.201899f, 0.210746f,    0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,    0.200959f, 0.393287f,    0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,    0.063362f, 0.392537f,    0.0f, 0.0f, 1.0f,

		// RIGHT
		 0.5f, -0.5f,  0.5f,    0.044416f, 0.599559f,    1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,    0.207583f, 0.599597f,    1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,    0.207527f, 0.769798f,    1.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,    0.044359f, 0.769760f,    1.0f, 0.0f, 0.0f,

		 // BACK
		 -0.5f, -0.5f, -0.5f,    0.687354f, 0.406082f,    0.0f, 0.0f, -1.0f,
		  0.5f, -0.5f, -0.5f,    0.505931f, 0.404278f,    0.0f, 0.0f, -1.0f,
		  0.5f,  0.5f, -0.5f,    0.503545f, 0.591182f,    0.0f, 0.0f, -1.0f,
		 -0.5f,  0.5f, -0.5f,    0.684968f, 0.592986f,    0.0f, 0.0f, -1.0f,

		 // LEFT
		 -0.5f, -0.5f, -0.5f,    0.331055f, 0.399037f,   -1.0f, 0.0f, 0.0f,
		 -0.5f, -0.5f,  0.5f,    0.494017f, 0.401320f,   -1.0f, 0.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,    0.491364f, 0.598227f,   -1.0f, 0.0f, 0.0f,
		 -0.5f,  0.5f, -0.5f,    0.328403f, 0.595945f,   -1.0f, 0.0f, 0.0f,

		 // BOTTOM
		 -0.5f, -0.5f,  0.5f,    0.972806f, 0.783820f,    0.0f, -1.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,    0.972741f, 0.588214f,    0.0f, -1.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,    0.830746f, 0.588273f,    0.0f, -1.0f, 0.0f,
		 -0.5f, -0.5f, -0.5f,    0.830811f, 0.783880f,    0.0f, -1.0f, 0.0f,

		 // TOP
		 -0.5f,  0.5f,  0.5f,    0.568274f, 0.792398f,    0.0f, 1.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,    0.570154f, 0.975610f,    0.0f, 1.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,    0.396210f, 0.979147f,    0.0f, 1.0f, 0.0f,
		 -0.5f,  0.5f, -0.5f,    0.394330f, 0.795936f,    0.0f, 1.0f, 0.0f
	};

	MeshModel* dado = new MeshModel();
	dado->CreateMeshModel(cubo_vertices, cubo_indices, 192, 36);
	meshListModel.push_back(dado);
}

void CrearHolocron()
{
	unsigned int holocron_indices[84];
	for (unsigned int i = 0; i < 84; i++) {
		holocron_indices[i] = i;
	}

	GLfloat holocron_vertices[] = {
// x          y          z         S         T          NX       NY       NZ
		// Cara 1
		 2.453980f, -0.063388f,  2.477451f, 0.038828f, 0.968373f,  0.5055f, -0.6014f,  0.6187f,
		 1.185055f, -2.530499f,  1.198261f, 0.033146f, 0.971571f,  0.5056f, -0.6014f,  0.6187f,
		 2.517274f, -2.579095f, -0.002115f, 0.032889f, 0.968373f,  0.4910f, -0.6002f,  0.6313f,

		// Cara 2
		 2.646786f, -0.139047f, -2.493757f, 0.984368f, 0.886634f,  0.9999f,  0.0018f, -0.0167f,
		 2.453980f, -0.063388f,  2.477451f, 0.828132f, 0.886803f,  0.9999f,  0.0012f, -0.0162f,
		 2.517274f, -2.579095f, -0.002115f, 0.906165f, 0.808601f,  0.9995f, -0.0253f, -0.0172f,

		// Cara 3
		-2.521206f,  2.448241f, -0.063388f, 0.038828f, 0.969605f, -0.5667f,  0.5876f,  0.5776f,
		-2.534944f, -0.061146f,  2.580280f, 0.043199f, 0.965213f, -0.5666f,  0.5876f,  0.5776f,
		-0.049649f,  2.440128f,  2.417217f, 0.047100f, 0.969571f, -0.5666f,  0.5877f,  0.5775f,

		// Cara 4
		-0.049649f,  2.440128f,  2.417217f, 0.030787f, 0.949582f,  0.5776f,  0.5876f,  0.5666f,
		 2.453980f, -0.063388f,  2.477451f, 0.035123f, 0.956947f,  0.5776f,  0.5876f,  0.5667f,
		 2.451372f,  2.421759f, -0.077538f, 0.026520f, 0.956965f,  0.5776f,  0.5877f,  0.5666f,

		// Cara 5
		-2.632201f, -2.553892f, -0.007226f, 0.422485f, 0.603642f,  0.0093f, -0.9994f, -0.0343f,
		 0.140696f, -2.601934f, -2.558561f, 0.504032f, 0.501343f, -0.0111f, -0.9999f, -0.0052f,
		 1.185055f, -2.530499f,  1.198261f, 0.339773f, 0.454056f,  0.0013f, -1.0000f, -0.0066f,

		// Cara 6
		 2.451372f,  2.421759f, -0.077538f, 0.047130f, 0.977916f,  0.5667f,  0.5874f, -0.5777f,
		 2.646786f, -0.139047f, -2.493757f, 0.043419f, 0.982345f,  0.5664f,  0.5877f, -0.5778f,
		-0.051152f,  2.399781f, -2.545043f, 0.038828f, 0.977877f,  0.5667f,  0.5874f, -0.5777f,

		// Cara 7
		-0.051152f,  2.399781f, -2.545043f, 0.034655f, 0.982002f, -0.5788f,  0.5849f, -0.5682f,
		-2.593003f, -0.085222f, -2.505755f, 0.034677f, 0.973605f, -0.5789f,  0.5850f, -0.5681f,
		-2.521206f,  2.448241f, -0.063388f, 0.038828f, 0.977943f, -0.5788f,  0.5849f, -0.5682f,

		// Cara 8
		 2.646786f, -0.139047f, -2.493757f, 0.046767f, 0.965213f,  0.6176f, -0.5877f, -0.5227f,
		 1.328995f, -2.578386f, -1.297838f, 0.044094f, 0.962068f,  0.6176f, -0.5876f, -0.5228f,
		 0.140696f, -2.601934f, -2.558561f, 0.047130f, 0.959322f,  0.6166f, -0.5880f, -0.5235f,

		// Cara 9
		-2.593003f, -0.085222f, -2.505755f, 0.017529f, 0.982345f, -0.5565f, -0.5649f, -0.6092f,
		-1.385028f, -2.549476f, -1.308677f, 0.015370f, 0.976290f, -0.5564f, -0.5650f, -0.6093f,
		-2.632201f, -2.553892f, -0.007226f, 0.017425f, 0.974047f, -0.5865f, -0.5652f, -0.5801f,

		// Cara 10
		-2.534944f, -0.061146f,  2.580280f, 0.023805f, 0.965398f, -0.5585f, -0.5875f,  0.5856f,
		-1.344182f, -2.542571f,  1.294387f, 0.025910f, 0.971574f, -0.5582f, -0.5875f,  0.5859f,
		-0.081432f, -2.513204f,  2.426005f, 0.023794f, 0.973597f, -0.5377f, -0.5876f,  0.6046f,

		// Cara 11
		-0.051152f,  2.399781f, -2.545043f, 0.262993f, 0.205924f, -0.0014f,  0.0170f, -0.9999f,
		 2.646786f, -0.139047f, -2.493757f, 0.162240f, 0.109673f,  0.0191f,  0.0180f, -0.9997f,
		 0.140696f, -2.601934f, -2.558561f, 0.258491f, 0.008920f,  0.0001f,  0.0169f, -0.9999f,

		// Cara 12
		-0.049649f,  2.440128f,  2.417217f, 0.125887f, 0.201670f,  0.0010f,  0.0175f,  0.9998f,
		-0.081432f, -2.513204f,  2.426005f, 0.133879f, 0.026846f,  0.0013f,  0.0175f,  0.9998f,
		 2.453980f, -0.063388f,  2.477451f, 0.217295f, 0.118254f, -0.0413f,  0.0174f,  0.9990f,

		// Cara 13
		 2.453980f, -0.063388f,  2.477451f, 0.035034f, 0.964515f,  0.5055f, -0.6014f,  0.6187f,
		-0.081432f, -2.513204f,  2.426005f, 0.035367f, 0.958555f,  0.5208f, -0.6020f,  0.6052f,
		 1.185055f, -2.530499f,  1.198261f, 0.038205f, 0.961606f,  0.5056f, -0.6014f,  0.6187f,

		// Cara 14
		 2.646786f, -0.139047f, -2.493757f, 0.046767f, 0.965213f,  0.6176f, -0.5877f, -0.5227f,
		 2.517274f, -2.579095f, -0.002115f, 0.040977f, 0.964811f,  0.6185f, -0.5876f, -0.5216f,
		 1.328995f, -2.578386f, -1.297838f, 0.044094f, 0.962068f,  0.6176f, -0.5876f, -0.5228f,

		// Cara 15
		-2.593003f, -0.085222f, -2.505755f, 0.034677f, 0.973605f, -0.5565f, -0.5649f, -0.6092f,
		 0.140696f, -2.601934f, -2.558561f, 0.025910f, 0.974043f, -0.5263f, -0.5634f, -0.6368f,
		-1.385028f, -2.549476f, -1.308677f, 0.028526f, 0.971571f, -0.5564f, -0.5650f, -0.6093f,

		// Cara 16
		-2.534944f, -0.061146f,  2.580280f, 0.035034f, 0.952727f, -0.5585f, -0.5875f,  0.5856f,
		-2.632201f, -2.553892f, -0.007226f, 0.041239f, 0.952649f, -0.5779f, -0.5873f,  0.5667f,
		-1.344182f, -2.542571f,  1.294387f, 0.038102f, 0.955628f, -0.5582f, -0.5875f,  0.5859f,

		// Cara 17
		-0.051152f,  2.399781f, -2.545043f, 0.503670f, 0.501644f,  0.0010f,  0.9999f, -0.0170f,
		-0.049649f,  2.440128f,  2.417217f, 0.683830f, 0.492497f,  0.0012f,  0.9999f, -0.0170f,
		 2.451372f,  2.421759f, -0.077538f, 0.598324f, 0.587150f, -0.0046f,  0.9998f, -0.0170f,

		// Cara 18
		-2.521206f,  2.448241f, -0.063388f, 0.412886f, 0.616204f, -0.9992f,  0.0358f, -0.0172f,
		-2.632201f, -2.553892f, -0.007226f, 0.413286f, 0.403327f, -0.9992f,  0.0358f, -0.0171f,
		-2.534944f, -0.061146f,  2.580280f, 0.519524f, 0.509965f, -0.9993f,  0.0365f, -0.0120f,

		// Cara 19
		 2.646786f, -0.139047f, -2.493757f, 0.984368f, 0.886634f,  0.9999f,  0.0018f, -0.0167f,
		 2.451372f,  2.421759f, -0.077538f, 0.906335f, 0.964836f,  0.9994f,  0.0282f, -0.0180f,
		 2.453980f, -0.063388f,  2.477451f, 0.828132f, 0.886803f,  0.9999f,  0.0012f, -0.0162f,

		// Cara 20
		-0.081432f, -2.513204f,  2.426005f, 0.320186f, 0.522094f,  0.0160f, -0.9998f, -0.0127f,
		-1.344182f, -2.542571f,  1.294387f, 0.354447f, 0.584055f,  0.1421f, -0.9772f, -0.1577f,
		-2.632201f, -2.553892f, -0.007226f, 0.422485f, 0.603642f,  0.0093f, -0.9994f, -0.0343f,

		// Cara 21
		-2.632201f, -2.553892f, -0.007226f, 0.422485f, 0.603642f,  0.0093f, -0.9994f, -0.0343f,
		-1.385028f, -2.549476f, -1.308677f, 0.484446f, 0.569381f, -0.1667f, -0.9654f, -0.2004f,
		 0.140696f, -2.601934f, -2.558561f, 0.504032f, 0.501343f, -0.0111f, -0.9999f, -0.0052f,

		// Cara 22
		 0.140696f, -2.601934f, -2.558561f, 0.504032f, 0.501343f, -0.0111f, -0.9999f, -0.0052f,
		 1.328995f, -2.578386f, -1.297838f, 0.469772f, 0.439383f,  0.5208f, -0.6862f, -0.5078f,
		 2.517274f, -2.579095f, -0.002115f, 0.401734f, 0.419796f, -0.0253f, -0.9997f, -0.0064f,

		// Cara 23
		 1.185055f, -2.530499f,  1.198261f, 0.339773f, 0.454056f,  0.0013f, -1.0000f, -0.0066f,
		-0.081432f, -2.513204f,  2.426005f, 0.320186f, 0.522094f,  0.0160f, -0.9998f, -0.0127f,
		-2.632201f, -2.553892f, -0.007226f, 0.422485f, 0.603642f,  0.0093f, -0.9994f, -0.0343f,

		// Cara 24
		 0.140696f, -2.601934f, -2.558561f, 0.504032f, 0.501343f, -0.0111f, -0.9999f, -0.0052f,
		 2.517274f, -2.579095f, -0.002115f, 0.401734f, 0.419796f, -0.0253f, -0.9997f, -0.0064f,
		 1.185055f, -2.530499f,  1.198261f, 0.339773f, 0.454056f,  0.0013f, -1.0000f, -0.0066f,

		// Cara 25
		-0.051152f,  2.399781f, -2.545043f, 0.262993f, 0.205924f, -0.0014f,  0.0170f, -0.9999f,
		 2.646786f, -0.139047f, -2.493757f, 0.162240f, 0.109673f,  0.0191f,  0.0180f, -0.9997f,
		 0.140696f, -2.601934f, -2.558561f, 0.258491f, 0.008920f,  0.0001f,  0.0169f, -0.9999f,

		// Cara 26
		-0.049649f,  2.440128f,  2.417217f, 0.125887f, 0.201670f,  0.0010f,  0.0175f,  0.9998f,
		-2.534944f, -0.061146f,  2.580280f, 0.042471f, 0.110262f,  0.0450f,  0.0169f,  0.9988f,
		-0.081432f, -2.513204f,  2.426005f, 0.133879f, 0.026846f,  0.0013f,  0.0175f,  0.9998f,

		// Cara 27
		-0.051152f,  2.399781f, -2.545043f, 0.503670f, 0.501644f,  0.0010f,  0.9999f, -0.0170f,
		-2.521206f,  2.448241f, -0.063388f, 0.589176f, 0.406991f,  0.0077f,  0.9998f, -0.0168f,
		-0.049649f,  2.440128f,  2.417217f, 0.683830f, 0.492497f,  0.0012f,  0.9999f, -0.0170f,

		// Cara 28
		-2.521206f,  2.448241f, -0.063388f, 0.412886f, 0.616204f, -0.9992f,  0.0358f, -0.0172f,
		-2.593003f, -0.085222f, -2.505755f, 0.306648f, 0.509566f, -0.9991f,  0.0359f, -0.0221f,
		-2.632201f, -2.553892f, -0.007226f, 0.413286f, 0.403327f, -0.9992f,  0.0358f, -0.0171f
	};

	MeshModel* holocron = new MeshModel();
	holocron->CreateMeshModel(holocron_vertices, holocron_indices, 672, 84);
	meshListModel.push_back(holocron);
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();

	CreateObjects();
	CrearDado();
	CrearHolocron();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	pisoTexture = Texture("Textures/piso.tga");
	pisoTexture.LoadTextureA();

	// Texturas de los ejercicios
	dadoTexture = Texture("Textures/star wars logos_colorAlpha.png");
	dadoTexture.LoadTextureA();

	holocronTexture = Texture("Textures/star wars logos_colorAlpha.png");
	holocronTexture.LoadTextureA();

	// Textura para el Avi�n (Ejercicio 3)
	avionTexture = Texture("Textures/BizarrePlaneTexture.png");
	avionTexture.LoadTextureA();

	// Ejercicio 2: Modelos .obj exportados desde Blender
	Dado_M = Model();
	Dado_M.LoadModel("Models/CuboStarWars.obj");

	Holocron_M = Model();
	Holocron_M.LoadModel("Models/holocronSimpleTexturizado.obj");

	// Ejercicio 3: Modelo del Avi�n
	BizarrePlane_M = Model();
	BizarrePlane_M.LoadModel("Models/BizarrePlane.obj");

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
	skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

	skybox = Skybox(skyboxFaces);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	GLuint uniformColor = 0;
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	glm::mat4 model(1.0);
	glm::mat4 modelaux(1.0);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

	// Loop principal
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		deltaTime += (now - lastTime) / limitFPS;
		lastTime = now;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		skybox.DrawSkybox(camera.calculateViewMatrix(), projection);
		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformColor = shaderList[0].getColorLocation();
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		color = glm::vec3(1.0f, 1.0f, 1.0f);

		// Renderizar Piso
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		pisoTexture.UseTexture();
		meshListModel[2]->RenderMeshModel();

		// Ejercicio 1 (EjercicioPrevio 6): Cubo texturizado por c�digo OpenGL
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-1.5f, 4.5f, -2.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		meshListModel[4]->RenderMeshModel();

		// Ejercicio 2 (EjercicioPrevio 6): Cubo texturizado importado desde Blender
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.5f, 4.5f, -2.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dadoTexture.UseTexture();
		Dado_M.RenderModel();

		// Ejercicio 1 (Practica 6): Renderizar Holocr�n por C�digo
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-8.5f, 4.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		holocronTexture.UseTexture();
		meshListModel[5]->RenderMeshModel();

		// Ejercicio 2 (Practica 6): Renderizar Holocr�n importado de Blender (holocronSimpleTexturizado.obj)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-4.5f, 2.5f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		Holocron_M.RenderModel();

		// Ejercicio 3 (Practica 6): Renderizar Avi�n Personaje (BizarrePlane.obj)
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		model = glm::mat4(1.0);
		// Mapeo: X = (Mover a la derecha), Y = (Ajustar altura), Z = (Mover al fondo)
		model = glm::translate(model, glm::vec3(6.0f, 1.0f, -10.0f));
		// Escala reducida al 50% para que no me tape toda la vista el avi�n, JAJA
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		avionTexture.UseTexture();
		BizarrePlane_M.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}