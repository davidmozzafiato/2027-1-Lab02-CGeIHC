#define STB_IMAGE_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <limits>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/spline.hpp>
#include <gtx/norm.hpp>

#include "Window.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Model.h"
#include "Skybox.h"
#include "DirectionalLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;
Window mainWindow;
std::vector<Shader> shaderList;
Camera camera;

Model Planeta_M;
Model Vehiculo_M;

// El satélite está dividido en cuatro modelos para animar cada parte por separado
Model SateliteCuerpo_M;
Model SatelitePanelIzquierdo_M;
Model SatelitePanelDerecho_M;
Model SateliteAntena_M;

Skybox* skybox = nullptr;
Material Material_brillante;
DirectionalLight mainLight;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Puntos de la pista ya filtrados
std::vector<glm::vec3> waypoints;

// Vehículo
int puntoActual = 0;
float progreso = 0.0f;          // 0 a 1 dentro del tramo actual de la spline
float velocidad = 25.0f;
float offsetLateral = -6.5f;     // separación respecto al centro de la pista
float anguloYawSuave = 0.0f;

// La altura se toma del planeta con raycast y se suaviza
float alturaYActual = 25.0f;
float alturaObjetivo = 25.0f;
float offsetAlturaVehiculo = 2.5f;
int contadorFrames = 0;

// Cámara
float movVelocidad = 100.0f;

// Satélite
float anguloPanelSolar = 0.0f;
float anguloAntena = 0.0f;
float anguloOrbita = 0.0f;
float velocidadOrbita = 15.0f;  // grados por segundo, negativo invierte el sentido
float radioOrbita = 600.0f;
float escalaSatelite = 8.0f;
float inclinacionOrbita = 25.0f;
float giroSatelite = 0.0f;      // ajuste de orientación del modelo (0, 90, 180, 270)
glm::vec3 centroPlaneta = glm::vec3(0.0f, 0.0f, 0.0f);

float rotacionMapaX = 0.0f;
float rotacionMapaY = 0.0f;

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

// Lee un punto "x y z" por línea (también acepta comas). Ignora líneas vacías y las que empiezan con / o #
std::vector<glm::vec3> CargarWaypointsDesdeTXT(const std::string& rutaArchivo)
{
	std::vector<glm::vec3> pts;
	std::ifstream archivo(rutaArchivo);

	if (!archivo.is_open())
	{
		std::cout << "Error al abrir el archivo de waypoints: " << rutaArchivo << std::endl;
		return pts;
	}

	std::string linea;
	while (std::getline(archivo, linea))
	{
		if (linea.empty() || linea[0] == '/' || linea[0] == '#')
			continue;

		float x, y, z;
		for (char& c : linea)
		{
			if (c == ',') c = ' ';
		}

		std::stringstream ss(linea);
		if (ss >> x >> y >> z)
		{
			pts.push_back(glm::vec3(x, y, z));
		}
	}

	archivo.close();
	std::cout << "Vértices leídos desde TXT: " << pts.size() << std::endl;
	return pts;
}

// Descarta puntos que quedan a menos de distMinima del anterior (en XZ) para que la spline no se amontone
std::vector<glm::vec3> UnirYFiltrarPista(const std::vector<glm::vec3>& puntosEntrada, float distMinima = 1.0f)
{
	if (puntosEntrada.empty()) return {};

	std::vector<glm::vec3> resultado;
	resultado.push_back(puntosEntrada[0]);

	for (size_t i = 1; i < puntosEntrada.size(); ++i)
	{
		glm::vec3 pUltimo = resultado.back();
		glm::vec3 pNuevo = puntosEntrada[i];

		float distXZ = glm::distance(glm::vec2(pUltimo.x, pUltimo.z), glm::vec2(pNuevo.x, pNuevo.z));

		if (distXZ >= distMinima)
		{
			resultado.push_back(pNuevo);
		}
	}

	std::cout << "Waypoints limpios y unidos: " << resultado.size() << std::endl;
	return resultado;
}

int main()
{
	mainWindow = Window(1366, 768);
	mainWindow.Initialise();
	glfwSwapInterval(1);

	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 80.0f, 300.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f, movVelocidad, 0.3f);

	Planeta_M.LoadModel("Models/Planeta2.glb");
	Vehiculo_M.LoadModel("Models/HaloGhost.glb");

	SateliteCuerpo_M.LoadModel("Models/SateliteCuerpo.glb");
	SatelitePanelIzquierdo_M.LoadModel("Models/SatelitePanelIzquierdo.glb");
	SatelitePanelDerecho_M.LoadModel("Models/SatelitePanelDerecho.glb");
	SateliteAntena_M.LoadModel("Models/SateliteAntena.glb");

	std::vector<glm::vec3> waypointsCrudos = CargarWaypointsDesdeTXT("vertices_pista_opengl.txt");

	if (waypointsCrudos.empty())
	{
		std::cout << "Advertencia: Archivo no encontrado o vacío. Cargando waypoints por defecto." << std::endl;
		waypoints = {
			glm::vec3(10.60f, 25.0f, 64.50f),
			glm::vec3(13.60f, 25.0f, -24.25f),
			glm::vec3(-35.50f, 25.0f, -32.55f)
		};
	}
	else
	{
		waypoints = UnirYFiltrarPista(waypointsCrudos, 1.2f);
	}

	if (!waypoints.empty())
	{
		alturaYActual = waypoints[0].y;
		alturaObjetivo = waypoints[0].y;
	}

	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox/space_rt.png");
	skyboxFaces.push_back("Textures/Skybox/space_lf.png");
	skyboxFaces.push_back("Textures/Skybox/space_dn.png");
	skyboxFaces.push_back("Textures/Skybox/space_up.png");
	skyboxFaces.push_back("Textures/Skybox/space_bk.png");
	skyboxFaces.push_back("Textures/Skybox/space_ft.png");
	skybox = new Skybox(skyboxFaces);

	Material_brillante = Material(2.0f, 64);
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f, 0.8f, 1.0f, 0.0f, -0.5f, -1.0f);

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0, uniformTextureOffset = 0, uniformColor = 0;

	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Avance sobre la spline Catmull-Rom; cada tramo usa 4 waypoints consecutivos
		progreso += velocidad * deltaTime;
		if (progreso >= 1.0f)
		{
			progreso -= 1.0f;
			puntoActual = (puntoActual + 1) % waypoints.size();
		}

		int numPuntos = (int)waypoints.size();
		int p0 = (puntoActual - 1 + numPuntos) % numPuntos;
		int p1 = puntoActual;
		int p2 = (puntoActual + 1) % numPuntos;
		int p3 = (puntoActual + 2) % numPuntos;

		glm::vec3 posInterpolada = glm::catmullRom(waypoints[p0], waypoints[p1], waypoints[p2], waypoints[p3], progreso);

		// Punto un poco adelante, solo para saber hacia dónde mira el vehículo
		float progresoDelta = progreso + 0.03f;
		glm::vec3 posSiguiente;
		if (progresoDelta < 1.0f)
		{
			posSiguiente = glm::catmullRom(waypoints[p0], waypoints[p1], waypoints[p2], waypoints[p3], progresoDelta);
		}
		else
		{
			int p0_n = p1, p1_n = p2, p2_n = p3, p3_n = (puntoActual + 3) % numPuntos;
			posSiguiente = glm::catmullRom(waypoints[p0_n], waypoints[p1_n], waypoints[p2_n], waypoints[p3_n], progresoDelta - 1.0f);
		}

		// Dirección en el plano XZ, desplazamiento lateral y giro suavizado
		glm::vec3 dir = posSiguiente - posInterpolada;
		dir.y = 0.0f;

		glm::vec3 posOffset = posInterpolada;

		if (glm::length(dir) > 0.0001f)
		{
			dir = glm::normalize(dir);

			glm::vec3 derecha = glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f));
			if (glm::length(derecha) > 0.0001f)
			{
				derecha = glm::normalize(derecha);
				posOffset += derecha * offsetLateral;
			}

			float anguloObjetivo = glm::degrees(atan2(dir.x, dir.z));
			float diff = anguloObjetivo - anguloYawSuave;
			while (diff < -180.0f) diff += 360.0f;
			while (diff > 180.0f)  diff -= 360.0f;

			anguloYawSuave += diff * 10.0f * deltaTime;
		}

		// El raycast se hace cada 2 frames; si el resultado salta más de 20 unidades se descarta
		contadorFrames++;
		if (contadorFrames >= 2)
		{
			float nuevaAlturaRaycast = 0.0f;
			if (Planeta_M.GetRayIntersectionY(posOffset.x, posOffset.z, nuevaAlturaRaycast))
			{
				if (std::abs(nuevaAlturaRaycast - posOffset.y) < 20.0f)
				{
					alturaObjetivo = nuevaAlturaRaycast;
				}
				else
				{
					alturaObjetivo = posOffset.y;
				}
			}
			else
			{
				alturaObjetivo = posOffset.y;
			}
			contadorFrames = 0;
		}

		float alturaDeseada = alturaObjetivo + offsetAlturaVehiculo;
		alturaYActual = glm::mix(alturaYActual, alturaDeseada, 0.1f);

		float maxVariacionY = 5.0f * deltaTime;
		alturaYActual = glm::clamp(alturaYActual, alturaYActual - maxVariacionY, alturaYActual + maxVariacionY);

		glm::vec3 posVehiculo = glm::vec3(posOffset.x, alturaYActual, posOffset.z);

		anguloPanelSolar += 30.0f * deltaTime;
		anguloAntena += 60.0f * deltaTime;

		anguloOrbita += velocidadOrbita * deltaTime;
		if (anguloOrbita >= 360.0f) anguloOrbita -= 360.0f;
		if (anguloOrbita < 0.0f)    anguloOrbita += 360.0f;

		glfwPollEvents();
		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (skybox != nullptr)
		{
			skybox->DrawSkybox(camera.calculateViewMatrix(), projection);
		}

		shaderList[0].UseShader();
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();
		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformColor = shaderList[0].getColorLocation();
		uniformTextureOffset = shaderList[0].getOffsetLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		shaderList[0].SetDirectionalLight(&mainLight);

		glm::vec2 toffset(0.0f, 0.0f);
		glUniform2fv(uniformTextureOffset, 1, glm::value_ptr(toffset));
		glm::vec3 color(1.0f, 1.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);

		// Planeta
		glm::mat4 modelMapa(1.0f);
		modelMapa = glm::translate(modelMapa, centroPlaneta);
		modelMapa = glm::rotate(modelMapa, rotacionMapaX * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMapa = glm::rotate(modelMapa, rotacionMapaY * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		modelMapa = glm::scale(modelMapa, glm::vec3(1.0f, 1.0f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelMapa));
		Planeta_M.RenderModel();

		// Vehículo
		glm::mat4 modelVehiculo(1.0f);
		modelVehiculo = glm::translate(modelVehiculo, posVehiculo);
		modelVehiculo = glm::rotate(modelVehiculo, glm::radians(anguloYawSuave), glm::vec3(0.0f, 1.0f, 0.0f));
		modelVehiculo = glm::rotate(modelVehiculo, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelVehiculo = glm::rotate(modelVehiculo, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		modelVehiculo = glm::scale(modelVehiculo, glm::vec3(2.0f, 2.0f, 2.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelVehiculo));
		Vehiculo_M.RenderModel();

		// Satélite: círculo en XZ inclinado sobre el eje X, centrado en el planeta
		float radOrb = glm::radians(anguloOrbita);

		glm::vec3 posLocalOrbita(radioOrbita * cos(radOrb), 0.0f, radioOrbita * sin(radOrb));
		glm::mat4 matInclinacion = glm::rotate(glm::mat4(1.0f), glm::radians(inclinacionOrbita), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 posSatelite = centroPlaneta + glm::vec3(matInclinacion * glm::vec4(posLocalOrbita, 1.0f));

		// Esta matriz es la base de las cuatro partes, así que todas siguen la órbita juntas
		glm::mat4 modelSatelitePadre(1.0f);
		modelSatelitePadre = glm::translate(modelSatelitePadre, posSatelite);

		// Mismo plano de la órbita y orientación tangente a ella
		modelSatelitePadre = modelSatelitePadre * matInclinacion;
		modelSatelitePadre = glm::rotate(modelSatelitePadre, -radOrb + glm::radians(giroSatelite), glm::vec3(0.0f, 1.0f, 0.0f));

		// Los modelos vienen de Blender con Z hacia arriba
		modelSatelitePadre = glm::rotate(modelSatelitePadre, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelSatelitePadre = glm::scale(modelSatelitePadre, glm::vec3(escalaSatelite, escalaSatelite, escalaSatelite));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelSatelitePadre));
		SateliteCuerpo_M.RenderModel();

		// Paneles: giran sobre X local; la rotación de 90° en Y los deja horizontales.
		// El orden importa, el giro animado va primero para que ocurra sobre el eje ya acomodado.
		glm::mat4 modelPanelIzq = modelSatelitePadre;
		modelPanelIzq = glm::rotate(modelPanelIzq, glm::radians(anguloPanelSolar), glm::vec3(1.0f, 0.0f, 0.0f));
		modelPanelIzq = glm::rotate(modelPanelIzq, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelPanelIzq));
		SatelitePanelIzquierdo_M.RenderModel();

		// El derecho gira en sentido contrario
		glm::mat4 modelPanelDer = modelSatelitePadre;
		modelPanelDer = glm::rotate(modelPanelDer, glm::radians(-anguloPanelSolar), glm::vec3(1.0f, 0.0f, 0.0f));
		modelPanelDer = glm::rotate(modelPanelDer, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelPanelDer));
		SatelitePanelDerecho_M.RenderModel();

		// Antena: gira sobre Z local
		glm::mat4 modelAntena = modelSatelitePadre;
		modelAntena = glm::rotate(modelAntena, glm::radians(anguloAntena), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelAntena));
		SateliteAntena_M.RenderModel();

		glUseProgram(0);
		mainWindow.swapBuffers();
	}

	if (skybox) delete skybox;

	return 0;
}