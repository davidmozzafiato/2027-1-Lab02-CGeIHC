/*
 Práctica 7: Iluminación 1
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
Texture avionTexture;

// Modelos
Model* Rover_M;
Model* BizarrePlane_M;
Model* Headlight_M;

Skybox* skybox;

// Materiales 
Material Material_brillante;
Material Material_opaco;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;

// Variables para animación de vehículos
float movRoverZ = 0.0f;
float movBizarrePlaneZ = -10.0f;
float movBizarrePlaneY = 1.0f;

// Velocidades de movimiento manuales para
// evitar que el avión se vaya de viaje con un clic
const float velocidadAvion = 0.8f;
const float velocidadRover = 1.0f;

// --- OFFSETS CALIBRADOS DE LUCES ---
const float offsetRoverX = 0.010f;
const float offsetRoverY = 1.603f;
const float offsetRoverZ = -1.335f;

const float offsetPlaneInfX = 0.000f;
const float offsetPlaneInfY = 0.823f;
const float offsetPlaneInfZ = 0.010f;

const float offsetPlaneFrontX = -0.009f;
const float offsetPlaneFrontY = 1.714f;
const float offsetPlaneFrontZ = -8.270f;

// Fuentes de luz 
DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

// Shaders 
static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";


// Cálculo de normales por promedio de vértices 
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
    unsigned int indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    GLfloat vertices[] = {
        //    x      y      z            u      v            nx      ny    nz 
            -1.0f, -1.0f, -0.6f,    0.0f, 0.0f,        0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 1.0f,        0.5f, 0.0f,        0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, -0.6f,        1.0f, 0.0f,        0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,        0.5f, 1.0f,        0.0f, 0.0f, 0.0f
    };

    unsigned int floorIndices[] = {
        0, 2, 1,
        1, 2, 3
    };

    GLfloat floorVertices[] = {
        -10.0f, 0.0f, -10.0f,    0.0f, 0.0f,        0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, -10.0f,    10.0f, 0.0f,    0.0f, -1.0f, 0.0f,
        -10.0f, 0.0f, 10.0f,    0.0f, 10.0f,    0.0f, -1.0f, 0.0f,
        10.0f, 0.0f, 10.0f,        10.0f, 10.0f,    0.0f, -1.0f, 0.0f
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

    calcAverageNormals(indices, 12, vertices, 32, 8, 5);
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
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.5f);

    pisoTexture = Texture("Textures/piso.tga");
    pisoTexture.LoadTextureA();

    avionTexture = Texture("Textures/BizarrePlaneTexture.png");
    avionTexture.LoadTextureA();

    // Carga de modelos 3D 
    Rover_M = new Model();
    Rover_M->LoadModel("Models/rovermodificado.obj");

    BizarrePlane_M = new Model();
    BizarrePlane_M->LoadModel("Models/BizarrePlane.obj");

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

    // Materiales 
    Material_brillante = Material(1.0f, 32.0f);
    Material_opaco = Material(0.3f, 4.0f);

    mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
        0.3f, 0.7f,
        0.5f, 0.8f, 1.0f);

    // Sin luces puntuales, solo las de los ejercicios
    unsigned int pointLightCount = 0;

    // Configuración de las 3 luces (SpotLights) 
    unsigned int spotLightCount = 0;

    // SpotLight 0: Faro frontal del Rover
    spotLights[0] = SpotLight(1.0f, 1.0f, 1.0f,
        0.2f, 3.0f,
        1.0f, 0.5f, 5.0f,
        0.0f, -0.2f, -1.0f,
        1.0f, 0.05f, 0.01f,
        15.0f);
    spotLightCount++;

    // SpotLight 1: Faro inferior del Bizarre Plane
    spotLights[1] = SpotLight(1.0f, 1.0f, 1.0f,
        0.2f, 3.0f,
        6.0f, 1.3f, -10.0f,
        0.0f, -1.0f, 0.0f,
        1.0f, 0.05f, 0.01f,
        20.0f);
    spotLightCount++;

    // SpotLight 2: Faro frontal del Bizarre Plane
    spotLights[2] = SpotLight(1.0f, 1.0f, 1.0f,
        0.2f, 3.0f,
        6.0f, 1.5f, -10.0f,
        0.0f, 0.0f, -1.0f,
        1.0f, 0.05f, 0.01f,
        15.0f);
    spotLightCount++;

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
        uniformSpecularIntensity = 0, uniformShininess = 0;
    GLuint uniformColor = 0;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 1000.0f);

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

        // Movimiento del Bizarre Plane (O, L, I, K)
        if (mainWindow.getsKeys()[GLFW_KEY_O])
        {
            movBizarrePlaneZ -= velocidadAvion * deltaTime;
        }
        if (mainWindow.getsKeys()[GLFW_KEY_L])
        {
            movBizarrePlaneZ += velocidadAvion * deltaTime;
        }
        if (mainWindow.getsKeys()[GLFW_KEY_I])
        {
            movBizarrePlaneY += velocidadAvion * deltaTime;
        }
        if (mainWindow.getsKeys()[GLFW_KEY_K])
        {
            movBizarrePlaneY -= velocidadAvion * deltaTime;
        }

        // Movimiento manual del Rover (U adelante, J atrás)
        if (mainWindow.getsKeys()[GLFW_KEY_U])
        {
            movRoverZ -= velocidadRover * deltaTime;
        }
        if (mainWindow.getsKeys()[GLFW_KEY_J])
        {
            movRoverZ += velocidadRover * deltaTime;
        }

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

        // Posiciones 3D de los focos de luz
        glm::vec3 posRoverActual = glm::vec3(1.0f, -1.0f, 6.0f + movRoverZ);
        glm::vec3 posBizarreActual = glm::vec3(6.0f, movBizarrePlaneY, movBizarrePlaneZ);

        glm::vec3 posFocoRover = posRoverActual + glm::vec3(offsetRoverX, offsetRoverY, offsetRoverZ);
        glm::vec3 posFocoAvionInf = posBizarreActual + glm::vec3(offsetPlaneInfX, offsetPlaneInfY, offsetPlaneInfZ);
        glm::vec3 posFocoAvionFront = posBizarreActual + glm::vec3(offsetPlaneFrontX, offsetPlaneFrontY, offsetPlaneFrontZ);

        // Actualización de luces SpotLights
        spotLights[0].SetFlash(posFocoRover, glm::vec3(0.0f, -0.2f, -1.0f));
        spotLights[1].SetFlash(posFocoAvionInf, glm::vec3(0.0f, -1.0f, 0.0f));  // Luz inferior
        spotLights[2].SetFlash(posFocoAvionFront, glm::vec3(0.0f, 0.0f, -1.0f));  // Luz frontal

        // Envío de información de luces al shader
        shaderList[0]->SetDirectionalLight(&mainLight);
        shaderList[0]->SetPointLights(pointLights, pointLightCount);
        shaderList[0]->SetSpotLights(spotLights, spotLightCount);

        glm::mat4 model(1.0);
        glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

        // Piso 
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(30.0f, 1.0f, 30.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        pisoTexture.UseTexture();
        Material_opaco.UseMaterial(uniformSpecularIntensity, uniformShininess);
        meshListModel[2]->RenderMeshModel();

        // Rover 
        model = glm::mat4(1.0);
        model = glm::translate(model, posRoverActual);
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Rover_M->RenderModel();

        // Bizarre Plane 
        model = glm::mat4(1.0);
        model = glm::translate(model, posBizarreActual);
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        avionTexture.UseTexture();
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        BizarrePlane_M->RenderModel();

        // =========================================================================
        // MODELOS HEADLIGHT
        // =========================================================================
        Material_brillante.UseMaterial(uniformSpecularIntensity, uniformShininess);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));

        // Headlight 0: Faro Rover
        model = glm::mat4(1.0f);
        model = glm::translate(model, posFocoRover);
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Headlight_M->RenderModel();

        // Headlight 1: Faro Avión Inferior
        model = glm::mat4(1.0f);
        model = glm::translate(model, posFocoAvionInf);
        model = glm::rotate(model, 90.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Headlight_M->RenderModel();

        // Headlight 2: Faro Avión Frontal
        model = glm::mat4(1.0f);
        model = glm::translate(model, posFocoAvionFront);
        model = glm::rotate(model, -90.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        Headlight_M->RenderModel();

        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    return 0;
}