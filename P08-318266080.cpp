/*
  Práctica 8: Iluminación
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
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh_tn.h"
#include "Shader_light.h"
#include "Camera.h"
#include "Texture.h"
#include "Sphere.h"
#include "Model.h"

#include "Skybox.h"

 // Iluminación y Materiales
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<MeshModel*> meshListModel;
std::vector<Shader*> shaderList;

Camera camera;

Texture pisoTexture;
Texture naveTexture;
Texture fuegoAzulTexture;

// Modelos
Model* Militia_Ship_M;
Model* Headlight_M;

Skybox* skybox;

// Materiales
Material Material_metalico;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Posición base de la nave
const glm::vec3 posNave = glm::vec3(0.0f, 5.0f, -5.0f);

const glm::vec3 offsetNavePoint = glm::vec3(0.0f, 1.5f, 0.0f);
const glm::vec3 offsetNaveSpot = glm::vec3(0.0f, -0.5f, -3.0f);

// COORDENADAS FIJAS CALIBRADAS A MANO LOL
const glm::vec3 offsetMotor1 = glm::vec3(-2.000f, 7.000f, -7.300f); // Motor 1 (Izquierdo)
const glm::vec3 offsetMotor2 = glm::vec3(2.000f, 7.000f, -7.300f); // Motor 2 (Derecho)
const float escalaFuego = 2.40f;

// Fuentes de luz
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Shaders
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";


void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
    unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indiceCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;
        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

        in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
        vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
        vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
        vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
    }

    for (size_t i = 0; i < verticeCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}


void CreateObjects()
{
    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -30.0f, 0.0f, -30.0f,    0.0f, 0.0f,        0.0f, 1.0f, 0.0f,
         30.0f, 0.0f, -30.0f,   10.0f, 0.0f,        0.0f, 1.0f, 0.0f,
        -30.0f, 0.0f,  30.0f,    0.0f, 10.0f,       0.0f, 1.0f, 0.0f,
         30.0f, 0.0f,  30.0f,   10.0f, 10.0f,       0.0f, 1.0f, 0.0f
    };

    // Plano del fuego
    unsigned int fuegoIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLfloat fuegoVertices[] = {
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,    0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,    1.0f, 1.0f,    0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,    0.0f, 1.0f,    0.0f, 0.0f, 1.0f
    };

    MeshModel* objPiso = new MeshModel();
    objPiso->CreateMeshModel(floorVertices, floorIndices, 32, 6);
    meshListModel.push_back(objPiso);

    MeshModel* objFuego = new MeshModel();
    objFuego->CreateMeshModel(fuegoVertices, fuegoIndices, 32, 6);
    meshListModel.push_back(objFuego);
}


void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(shader1);
}


int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.Initialise();
    glDisable(GL_CULL_FACE);

    CreateObjects();
    CreateShaders();
    camera = Camera(glm::vec3(0.0f, 5.0f, 12.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 0.3f, 0.5f);

    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();

    naveTexture = Texture("Textures/Militia_Ship.jpg");
    naveTexture.LoadTextureA();

    fuegoAzulTexture = Texture("Textures/fuego_azul.png");
    fuegoAzulTexture.LoadTextureA();

    Militia_Ship_M = new Model();
    Militia_Ship_M->LoadModel("Models/Militia_Ship.obj");

    Headlight_M = new Model();
    Headlight_M->LoadModel("Models/headlight.obj");

    std::vector<std::string> skyboxFaces;
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_rt.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_lf.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_dn.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_up.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_bk.tga");
    skyboxFaces.push_back("Textures/Skybox/cupertin-lake_ft.tga");

    skybox = new Skybox(skyboxFaces);

    Material_metalico = Material(2.0f, 128.0f);
    Material_opaco = Material(0.3f, 4.0f);

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
        0.2f, 0.5f,
        0.5f, -0.8f, 1.0f);

    // Inicialización fija de fuentes de luz
    pointLights[0] = PointLight(1.0f, 1.0f, 1.0f,
        0.3f, 2.0f,
        posNave.x + offsetNavePoint.x, posNave.y + offsetNavePoint.y, posNave.z + offsetNavePoint.z,
        1.0f, 0.09f, 0.032f);

    pointLights[1] = PointLight(0.0f, 0.4f, 1.0f,
        0.4f, 3.0f,
        posNave.x + offsetMotor1.x, posNave.y + offsetMotor1.y, posNave.z + offsetMotor1.z,
        1.0f, 0.09f, 0.032f);

    pointLights[2] = PointLight(0.0f, 0.4f, 1.0f,
        0.4f, 3.0f,
        posNave.x + offsetMotor2.x, posNave.y + offsetMotor2.y, posNave.z + offsetMotor2.z,
        1.0f, 0.09f, 0.032f);

    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
        0.2f, 4.0f,
        posNave.x + offsetNaveSpot.x, posNave.y + offsetNaveSpot.y, posNave.z + offsetNaveSpot.z,
        0.0f, -0.2f, -1.0f,
        1.0f, 0.05f, 0.01f,
        25.0f);

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0;
    GLuint uniformColor = 0;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

    // Bucle principal
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
        skybox->DrawSkybox(camera.calculateViewMatrix(), projection);

        shaderList[0]->UseShader();
        uniformModel = shaderList[0]->GetModelLocation();
        uniformProjection = shaderList[0]->GetProjectionLocation();
        uniformView = shaderList[0]->GetViewLocation();
        uniformEyePosition = shaderList[0]->GetEyePositionLocation();
        uniformColor = shaderList[0]->getColorLocation();

        uniformSpecularIntensity = shaderList[0]->GetSpecularIntensityLocation();
        uniformShininess = shaderList[0]->GetShininessLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        // Lectura de banderas ON/OFF de luces
        bool lucesNaveOn = mainWindow.getNaveLucesOn();
        bool motor1On = mainWindow.getMotor1On();
        bool motor2On = mainWindow.getMotor2On();

        // Filtrado dinámico de luces puntuales activas
        PointLight activePointLights[3];
        unsigned int numActivePointLights = 0;

        if (lucesNaveOn) { activePointLights[numActivePointLights++] = pointLights[0]; }
        if (motor1On) { activePointLights[numActivePointLights++] = pointLights[1]; }
        if (motor2On) { activePointLights[numActivePointLights++] = pointLights[2]; }

        shaderList[0]->SetDirectionalLight(&mainLight);
        shaderList[0]->SetPointLights(activePointLights, numActivePointLights);
        shaderList[0]->SetSpotLights(spotLights, lucesNaveOn ? 1 : 0);

        glm::mat4 model(1.0);
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

        // --- PISO ---
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshListModel[0]->RenderMeshModel();

        // --- NAVE ESPACIAL ---
        model = glm::mat4(1.0f);
        model = glm::translate(model, posNave);
        model = glm::rotate(model, 180.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

        naveTexture.UseTexture();
        Material_metalico.UseMaterial(uniformSpecularIntensity, uniformShininess);
        Militia_Ship_M->RenderModel();

        // --- DIBUJAR FUEGO DE LOS MOTORES ---
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        fuegoAzulTexture.UseTexture();

        // Motor 1 (Izquierdo)
        if (motor1On)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, posNave + offsetMotor1);
            model = glm::scale(model, glm::vec3(escalaFuego, escalaFuego, escalaFuego));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshListModel[1]->RenderMeshModel();
        }

        // Motor 2 (Derecho)
        if (motor2On)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, posNave + offsetMotor2);
            model = glm::scale(model, glm::vec3(escalaFuego, escalaFuego, escalaFuego));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshListModel[1]->RenderMeshModel();
        }

        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, 0); // Desvincular textura activa
        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    return 0;
}