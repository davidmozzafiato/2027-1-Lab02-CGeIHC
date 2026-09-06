#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>

// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Clases de la práctica
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

const float toRadians = 3.14159265f / 180.0f;
Window mainWindow;

std::vector<Mesh*> meshList;
std::vector<MeshColor*> meshColorList;
std::vector<Shader> shaderList;

// Rutas de Shaders
static const char* fShaderComun = "shaders/shader.frag";
static const char* vShaderVerde = "shaders/shader_verde.vert";
static const char* vShaderAzul = "shaders/shader_azul.vert";
static const char* vShaderCafe = "shaders/shader_cafe.vert";
static const char* vShaderMagenta = "shaders/shader_magenta.vert";
static const char* vShaderRojo = "shaders/shaderrojo.vert";

// Shader para las iniciales DPS
static const char* vShaderColor = "shaders/shadercolor.vert";
static const char* fShaderColor = "shaders/shadercolor.frag";

// Punteros a shaders específicos
Shader* sVerde;
Shader* sAzul;
Shader* sCafe;
Shader* sMagenta;
Shader* sRojo;
Shader* sLetras;

// -------------------------------------------------------------
// GEOMETRÍAS INDEXADAS
// -------------------------------------------------------------

// Pirámide regular de base cuadrada
void CrearPiramideCuadrangular()
{
	unsigned int indices[] = {
		0, 3, 4,
		3, 2, 4,
		2, 1, 4,
		1, 0, 4,
		0, 1, 2,
		0, 2, 3
	};
	GLfloat vertices[] = {
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		 0.0f,  0.5f,  0.0f
	};
	Mesh* piramide = new Mesh();
	piramide->CreateMesh(vertices, indices, 15, 18);
	meshList.push_back(piramide); // meshList[0]
}

// Cubo unitario indexado
void CrearCubo()
{
	unsigned int indices[] = {
		0, 1, 2,   2, 3, 0, // Frente
		1, 5, 6,   6, 2, 1, // Derecha
		7, 6, 5,   5, 4, 7, // Atras
		4, 0, 3,   3, 7, 4, // Izquierda
		4, 5, 1,   1, 0, 4, // Abajo
		3, 2, 6,   6, 7, 3  // Arriba
	};

	GLfloat vertices[] = {
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f
	};
	Mesh* cubo = new Mesh();
	cubo->CreateMesh(vertices, indices, 24, 36);
	meshList.push_back(cubo); // meshList[1]
}

// -------------------------------------------------------------
// INICIALES DPS CON COLOR POR VÉRTICE (Clase MeshColor)
// -------------------------------------------------------------
void CrearInicialesDPS()
{
	GLfloat vertices_DPS[] = {
		// X      Y      Z      R     G     B
		// ======================= LETRA D =======================
		-0.75f, -0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f, -0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f,  0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.75f, -0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f,  0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.75f,  0.45f, 0.0f,   0.0f, 0.4f, 1.0f,

		-0.65f,  0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f,  0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f,  0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f,  0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f,  0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f,  0.45f, 0.0f,   0.0f, 0.4f, 1.0f,

		-0.35f, -0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.25f, -0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.25f,  0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f, -0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.25f,  0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f,  0.35f, 0.0f,   0.0f, 0.4f, 1.0f,

		-0.65f, -0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f, -0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f, -0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f, -0.45f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.35f, -0.35f, 0.0f,   0.0f, 0.4f, 1.0f,
		-0.65f, -0.35f, 0.0f,   0.0f, 0.4f, 1.0f,

		// ======================= LETRA P =======================
		-0.15f, -0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f, -0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f,  0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.15f, -0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f,  0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.15f,  0.45f, 0.0f,   1.0f, 0.1f, 0.1f,

		-0.05f,  0.35f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.35f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f,  0.35f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.45f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f,  0.45f, 0.0f,   1.0f, 0.1f, 0.1f,

		 0.15f,  0.00f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.00f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.35f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.15f,  0.00f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.35f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.15f,  0.35f, 0.0f,   1.0f, 0.1f, 0.1f,

		-0.05f,  0.00f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.00f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.10f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f,  0.00f, 0.0f,   1.0f, 0.1f, 0.1f,
		 0.25f,  0.10f, 0.0f,   1.0f, 0.1f, 0.1f,
		-0.05f,  0.10f, 0.0f,   1.0f, 0.1f, 0.1f,

		// ======================= LETRA S =======================
		 0.45f,  0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f,  0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f,  0.45f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f,  0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f,  0.45f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f,  0.45f, 0.0f,   0.0f, 1.0f, 0.3f,

		 0.45f,  0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.55f,  0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.55f,  0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f,  0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.55f,  0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f,  0.35f, 0.0f,   0.0f, 1.0f, 0.3f,

		 0.45f, -0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f,  0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f, -0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f,  0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f,  0.05f, 0.0f,   0.0f, 1.0f, 0.3f,

		 0.85f, -0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.85f, -0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.05f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.85f, -0.05f, 0.0f,   0.0f, 1.0f, 0.3f,

		 0.45f, -0.45f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.45f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f, -0.45f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.95f, -0.35f, 0.0f,   0.0f, 1.0f, 0.3f,
		 0.45f, -0.35f, 0.0f,   0.0f, 1.0f, 0.3f
	};

	MeshColor* letras = new MeshColor();
	letras->CreateMeshColor(vertices_DPS, 78 * 6);
	meshColorList.push_back(letras); // meshColorList[0]
}

// -------------------------------------------------------------
// COMPILACIÓN DE SHADERS INDEPENDIENTES
// -------------------------------------------------------------
void CreateShaders()
{
	// 1. Shader Verde
	sVerde = new Shader();
	sVerde->CreateFromFiles(vShaderVerde, fShaderComun);
	shaderList.push_back(*sVerde);

	// 2. Shader Azul
	sAzul = new Shader();
	sAzul->CreateFromFiles(vShaderAzul, fShaderComun);
	shaderList.push_back(*sAzul);

	// 3. Shader Café
	sCafe = new Shader();
	sCafe->CreateFromFiles(vShaderCafe, fShaderComun);
	shaderList.push_back(*sCafe);

	// 4. Shader Magenta
	sMagenta = new Shader();
	sMagenta->CreateFromFiles(vShaderMagenta, fShaderComun);
	shaderList.push_back(*sMagenta);

	// 5. Shader Rojo
	sRojo = new Shader();
	sRojo->CreateFromFiles(vShaderRojo, fShaderComun);
	shaderList.push_back(*sRojo);

	// 6. Shader de Iniciales (Color por vértice)
	sLetras = new Shader();
	sLetras->CreateFromFiles(vShaderColor, fShaderColor);
	shaderList.push_back(*sLetras);
}

// Función auxiliar para renderizar con un shader específico
void DibujarInstancia(Shader* shaderObj, Mesh* malla, glm::mat4 matBase, glm::mat4 proj, glm::vec3 pos, glm::vec3 escala, float rotZ = 0.0f)
{
	shaderObj->useShader();
	GLuint uModel = shaderObj->getModelLocation();
	GLuint uProj = shaderObj->getProjectLocation();

	glm::mat4 model = matBase;
	model = glm::translate(model, pos);
	if (rotZ != 0.0f) {
		model = glm::rotate(model, rotZ * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	model = glm::scale(model, escala);

	glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(proj));
	malla->RenderMesh();
}

int main()
{
	mainWindow = Window(800, 600);
	mainWindow.Initialise();

	glEnable(GL_DEPTH_TEST);

	CrearPiramideCuadrangular(); // meshList[0]
	CrearCubo();                 // meshList[1]
	CrearInicialesDPS();         // meshColorList[0]

	CreateShaders();

	// Proyección ortogonal para mantener la distribución del ejercicio de clase
	glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -0.8f, 0.8f, -10.0f, 10.0f);

	bool mostrarIniciales = false;
	bool spaceTeclaPresionada = false;
	float curAngle = 0.0f;

	while (!mainWindow.getShouldClose())
	{
		glfwPollEvents();

		curAngle += 0.4f;
		if (curAngle >= 360.0f) curAngle -= 360.0f;

		// Alternar vistas mediante ESPACIO
		GLFWwindow* winPtr = glfwGetCurrentContext();
		if (glfwGetKey(winPtr, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			if (!spaceTeclaPresionada)
			{
				mostrarIniciales = !mostrarIniciales;
				spaceTeclaPresionada = true;
			}
		}
		else
		{
			spaceTeclaPresionada = false;
		}

		glClearColor(0.85f, 0.85f, 0.85f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 rootRotate = glm::mat4(1.0f);
		rootRotate = glm::rotate(rootRotate, 25.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		rootRotate = glm::rotate(rootRotate, curAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		if (!mostrarIniciales)
		{
			// =========================================================================
			// 1. FIGURAS 3D DE CLASE (CUBOS Y PIRÁMIDES CON SHADERS SÓLIDOS)
			// =========================================================================

			// --- FIGURA 1 (Izquierda): Postes café y pirámides apiladas ---
			DibujarInstancia(sCafe, meshList[1], rootRotate, projection, glm::vec3(-0.77f, -0.02f, 0.0f), glm::vec3(0.04f, 0.76f, 0.06f));
			DibujarInstancia(sCafe, meshList[1], rootRotate, projection, glm::vec3(-0.49f, -0.02f, 0.0f), glm::vec3(0.04f, 0.76f, 0.06f));

			DibujarInstancia(sVerde, meshList[0], rootRotate, projection, glm::vec3(-0.63f, -0.28f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 180.0f);
			DibujarInstancia(sRojo, meshList[0], rootRotate, projection, glm::vec3(-0.63f, -0.04f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 180.0f);
			DibujarInstancia(sMagenta, meshList[0], rootRotate, projection, glm::vec3(-0.63f, 0.20f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 180.0f);

			// --- FIGURA 2 (Centro): Cuadrantes y rombo central de cubos ---
			DibujarInstancia(sRojo, meshList[1], rootRotate, projection, glm::vec3(-0.12f, -0.04f, 0.0f), glm::vec3(0.24f, 0.24f, 0.12f));
			DibujarInstancia(sRojo, meshList[1], rootRotate, projection, glm::vec3(0.12f, -0.04f, 0.0f), glm::vec3(0.24f, 0.24f, 0.12f));
			DibujarInstancia(sMagenta, meshList[1], rootRotate, projection, glm::vec3(-0.12f, -0.28f, 0.0f), glm::vec3(0.24f, 0.24f, 0.12f));
			DibujarInstancia(sVerde, meshList[1], rootRotate, projection, glm::vec3(0.12f, -0.28f, 0.0f), glm::vec3(0.24f, 0.24f, 0.12f));

			// Rombos a 45°
			DibujarInstancia(sAzul, meshList[1], rootRotate, projection, glm::vec3(0.0f, -0.16f, 0.07f), glm::vec3(0.30f, 0.30f, 0.12f), 45.0f);
			DibujarInstancia(sCafe, meshList[1], rootRotate, projection, glm::vec3(0.0f, -0.16f, 0.14f), glm::vec3(0.18f, 0.18f, 0.12f), 45.0f);

			// --- FIGURA 3 (Derecha): Triángulo compuesto de pirámides ---
			DibujarInstancia(sVerde, meshList[0], rootRotate, projection, glm::vec3(0.50f, -0.28f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 0.0f);
			DibujarInstancia(sRojo, meshList[0], rootRotate, projection, glm::vec3(0.74f, -0.28f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 0.0f);
			DibujarInstancia(sMagenta, meshList[0], rootRotate, projection, glm::vec3(0.62f, -0.04f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 0.0f);
			DibujarInstancia(sAzul, meshList[0], rootRotate, projection, glm::vec3(0.62f, -0.28f, 0.0f), glm::vec3(0.24f, 0.24f, 0.24f), 180.0f);
		}
		else
		{
			// =========================================================================
			// 2. INICIALES DPS (CADA LETRA CON SU COLOR POR VÉRTICE)
			// =========================================================================
			sLetras->useShader();
			GLuint uModel = sLetras->getModelLocation();
			GLuint uProj = sLetras->getProjectLocation();

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(0.85f, 0.85f, 1.0f));

			glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(uProj, 1, GL_FALSE, glm::value_ptr(projection));
			meshColorList[0]->RenderMeshColor();
		}

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	return 0;
}