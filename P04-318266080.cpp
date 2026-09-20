/*Práctica 4: Modelado Jerárquico.
Se implementa el uso de matrices adicionales para almacenar información de transformaciones geométricas que se quiere
heredar entre diversas instancias para que estén unidas
Teclas de la F al K para rotaciones de articulaciones
Teclas T-Y, U-I para la suspensión
Teclas O-P para abrir/cerrar la garra
Teclas Z, X, C, V, B, N para hacer rodar cada una de las 6 llantas sobre su propio eje
Incluye Sonda Espacial jerárquica con detalles avanzados (antena, mástil de cámara, propulsores y paneles estructurados).
*/
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>
//glm
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <gtc\random.hpp>
//clases para dar orden y limpieza al código
#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

//Dimensiones de la ventana
const float toRadians = 3.14159265f / 180.0; //grados a radianes
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

// Variables para el control de las articulaciones del brazo
float rotBrazoBase = 0.0f;
float rotHombro = 0.0f;
float rotAntebrazo = 0.0f;

// Variable para la animación de apertura/cierre de la garra de 4 dedos (Teclas O-P)
float aperturaGarra = 0.0f;

// Variables para el control de la suspensión delantera en L (Teclas T-Y, U-I)
float rotSuspension1 = 30.0f;
float rotSuspension2 = -20.0f;

// Variables para el giro sobre su propio eje (rodadura) de cada una de las 6 llantas (Teclas Z, X, C, V, B, N)
float rodLlanta1 = 0.0f; // Delantera Izquierda (Tecla Z)
float rodLlanta2 = 0.0f; // Delantera Derecha  (Tecla X)
float rodLlanta3 = 0.0f; // Media Izquierda    (Tecla C)
float rodLlanta4 = 0.0f; // Media Derecha      (Tecla V)
float rodLlanta5 = 0.0f; // Trasera Izquierda  (Tecla B)
float rodLlanta6 = 0.0f; // Trasera Derecha    (Tecla N)

// Variables para la animación automática de la sonda espacial
float rotPanelesSolares = 0.0f;
float rotAntenaSonda = 0.0f;

//Vertex Shader
static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
Sphere sp = Sphere(1.0, 20, 20); //recibe radio, slices, stacks

void CrearCubo()
{
    unsigned int cubo_indices[] = {
        0, 1, 2,  2, 3, 0,
        1, 5, 6,  6, 2, 1,
        7, 6, 5,  5, 4, 7,
        4, 0, 3,  3, 7, 4,
        4, 5, 1,  1, 0, 4,
        3, 2, 6,  6, 7, 3
    };

    GLfloat cubo_vertices[] = {
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
    cubo->CreateMesh(cubo_vertices, cubo_indices, 24, 36);
    meshList.push_back(cubo);
}

void CrearPiramideTriangular()
{
    unsigned int indices_piramide_triangular[] = { 0,1,2, 1,3,2, 3,0,2, 1,0,3 };
    GLfloat vertices_piramide_triangular[] = {
        -0.5f, -0.5f,0.0f,
         0.5f,-0.5f,0.0f,
         0.0f,0.5f, -0.25f,
         0.0f,-0.5f,-0.5f,
    };
    Mesh* obj1 = new Mesh();
    obj1->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
    meshList.push_back(obj1);
}

void CrearCilindro(int res, float R) {
    int n, i;
    GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
    vector<GLfloat> vertices;
    vector<unsigned int> indices;

    for (n = 0; n <= (res); n++) {
        x = (n != res) ? R * cos(n * dt) : R * cos(0);
        z = (n != res) ? R * sin(n * dt) : R * sin(0);
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        vertices.push_back(x); vertices.push_back(0.5); vertices.push_back(z);
    }
    for (n = 0; n <= (res); n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
    }
    for (n = 0; n <= (res); n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(0.5); vertices.push_back(z);
    }
    for (i = 0; i < vertices.size(); i++) indices.push_back(i);

    Mesh* cilindro = new Mesh();
    cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
    meshList.push_back(cilindro);
}

void CrearCono(int res, float R) {
    int n, i;
    GLfloat dt = 2 * PI / res, x, z, y = -0.5f;
    vector<GLfloat> vertices;
    vector<unsigned int> indices;
    vertices.push_back(0.0); vertices.push_back(0.5); vertices.push_back(0.0);
    for (n = 0; n <= (res); n++) {
        x = R * cos(n * dt); z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
    }
    vertices.push_back(R * cos(0) * dt); vertices.push_back(-0.5); vertices.push_back(R * sin(0) * dt);
    for (i = 0; i < res + 2; i++) indices.push_back(i);
    Mesh* cono = new Mesh();
    cono->CreateMeshGeometry(vertices, indices, vertices.size(), res + 2);
    meshList.push_back(cono);
}

void CrearPiramideCuadrangular()
{
    vector<unsigned int> piramidecuadrangular_indices = { 0,3,4, 3,2,4, 2,1,4, 1,0,4, 0,1,2, 0,2,4 };
    vector<GLfloat> piramidecuadrangular_vertices = {
        0.5f,-0.5f,0.5f, 0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f, -0.5f,-0.5f,0.5f, 0.0f,0.5f,0.0f,
    };
    Mesh* piramide = new Mesh();
    piramide->CreateMeshGeometry(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
    meshList.push_back(piramide);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

int main()
{
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    CrearCubo();
    CrearPiramideTriangular();
    CrearCilindro(20, 1.0f);
    CrearCono(25, 2.0f);
    CrearPiramideCuadrangular();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 0.3f, 0.3f);

    GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformColor = 0;
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    sp.init();
    sp.load();

    glm::mat4 model(1.0), modelaux(1.0), modelaux2(1.0);
    glm::vec3 color(0.0f);

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        // Controles de articulaciones del brazo principal
        if (mainWindow.getsKeys()[GLFW_KEY_F]) rotBrazoBase += 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_G]) rotBrazoBase -= 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_H]) rotHombro += 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_J]) rotHombro -= 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_K]) rotAntebrazo += 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_L]) rotAntebrazo -= 0.8f;

        // Controles de la garra de 4 dedos (Teclas O-P para abrir/cerrar)
        if (mainWindow.getsKeys()[GLFW_KEY_O]) aperturaGarra += 0.015f;
        if (mainWindow.getsKeys()[GLFW_KEY_P]) aperturaGarra -= 0.015f;
        if (aperturaGarra < 0.0f) aperturaGarra = 0.0f;
        if (aperturaGarra > 0.45f) aperturaGarra = 0.45f; // Límite de apertura

        // Controles de la suspensión en L (Teclas T-Y, U-I)
        if (mainWindow.getsKeys()[GLFW_KEY_T]) rotSuspension1 += 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_Y]) rotSuspension1 -= 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_U]) rotSuspension2 += 0.8f;
        if (mainWindow.getsKeys()[GLFW_KEY_I]) rotSuspension2 -= 0.8f;

        // Controles de giro sobre su propio eje (rodadura) para las 6 llantas con teclas Z, X, C, V, B, N
        if (mainWindow.getsKeys()[GLFW_KEY_Z]) rodLlanta1 += 2.0f;
        if (mainWindow.getsKeys()[GLFW_KEY_X]) rodLlanta2 += 2.0f;
        if (mainWindow.getsKeys()[GLFW_KEY_C]) rodLlanta3 += 2.0f;
        if (mainWindow.getsKeys()[GLFW_KEY_V]) rodLlanta4 += 2.0f;
        if (mainWindow.getsKeys()[GLFW_KEY_B]) rodLlanta5 += 2.0f;
        if (mainWindow.getsKeys()[GLFW_KEY_N]) rodLlanta6 += 2.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        // 1. CABINA DEL ROVER
        model = glm::mat4(1.0);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, -4.0f));
        modelaux = model;
        model = glm::translate(model, glm::vec3(1.0f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(8.0f, 4.0f, 6.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));
        color = glm::vec3(1.0f, 0.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // 2. BASE INFERIOR DEL ROVER
        model = modelaux;
        model = glm::translate(model, glm::vec3(0.0f, -0.75f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(10.0f, 1.5f, 8.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelaux;
        modelaux2 = model; // Base principal guardada

        // ==========================================
        // SUSPENSIÓN Y LLANTAS DELANTERAS
        // ==========================================

        // --- DELANTERA IZQUIERDA (Llanta 1 - Tecla Z) ---
        model = modelaux2;
        model = glm::translate(model, glm::vec3(-4.0f, -0.2f, 4.2f));
        modelaux = model;

        model = modelaux;
        glm::mat4 modelEsferaRotor = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelEsferaRotor));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        model = modelaux;
        model = glm::rotate(model, glm::radians(rotSuspension1), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelCodoL = model;
        modelCodoL = glm::translate(modelCodoL, glm::vec3(-3.2f, 0.0f, 0.0f));

        model = model;
        model = glm::translate(model, glm::vec3(-1.6f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.2f, 0.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelCodoL;
        glm::mat4 modelEsferaCodoL = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelEsferaCodoL));
        color = glm::vec3(1.0f, 0.5f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        model = modelCodoL;
        model = glm::rotate(model, glm::radians(rotSuspension2), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        glm::mat4 modelExtremoL = model;

        model = model;
        model = glm::scale(model, glm::vec3(0.3f, 1.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Render Llanta 1 con giro sobre su propio eje (Tecla Z)
        model = modelExtremoL;
        model = glm::translate(model, glm::vec3(0.0f, -0.65f, 0.65f));
        model = glm::rotate(model, glm::radians(rodLlanta1), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.2f, 0.2f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();


        // --- DELANTERA DERECHA (Llanta 2 - Tecla X) ---
        model = modelaux2;
        model = glm::translate(model, glm::vec3(-4.0f, -0.2f, -4.2f));
        modelaux = model;

        model = modelaux;
        glm::mat4 modelEsferaRotorD = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelEsferaRotorD));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        model = modelaux;
        model = glm::rotate(model, glm::radians(rotSuspension1), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelCodoLD = model;
        modelCodoLD = glm::translate(modelCodoLD, glm::vec3(-3.2f, 0.0f, 0.0f));

        model = model;
        model = glm::translate(model, glm::vec3(-1.6f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(3.2f, 0.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelCodoLD;
        glm::mat4 modelEsferaCodoLD = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelEsferaCodoLD));
        color = glm::vec3(1.0f, 0.5f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        model = modelCodoLD;
        model = glm::rotate(model, glm::radians(rotSuspension2), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        glm::mat4 modelExtremoLD = model;

        model = model;
        model = glm::scale(model, glm::vec3(0.3f, 1.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Render Llanta 2 con giro sobre su propio eje (Tecla X)
        model = modelExtremoLD;
        model = glm::translate(model, glm::vec3(0.0f, -0.65f, -0.65f));
        model = glm::rotate(model, glm::radians(rodLlanta2), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.2f, 0.2f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();


        // ==========================================
        // SUSPENSIÓN Y LLANTAS DE EN MEDIO
        // ==========================================

        // --- IZQUIERDA MEDIA (Llanta 3 - Tecla C) ---
        model = modelaux2;
        model = glm::translate(model, glm::vec3(-1.5f, -0.2f, 4.2f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelCodoMedI = model;

        model = modelCodoMedI;
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 0.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelCodoMedI;
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        glm::mat4 modelExtremoMedI = model;

        model = model;
        model = glm::scale(model, glm::vec3(0.3f, 1.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Render Llanta 3 con giro sobre su propio eje (Tecla C)
        model = modelExtremoMedI;
        model = glm::translate(model, glm::vec3(0.0f, -0.65f, 0.65f));
        model = glm::rotate(model, glm::radians(rodLlanta3), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.2f, 0.2f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();


        // --- DERECHA MEDIA (Llanta 4 - Tecla V) ---
        model = modelaux2;
        model = glm::translate(model, glm::vec3(-1.5f, -0.2f, -4.2f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelCodoMedD = model;

        model = modelCodoMedD;
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 0.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelCodoMedD;
        model = glm::translate(model, glm::vec3(-2.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        glm::mat4 modelExtremoMedD = model;

        model = model;
        model = glm::scale(model, glm::vec3(0.3f, 1.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Render Llanta 4 con giro sobre su propio eje (Tecla V)
        model = modelExtremoMedD;
        model = glm::translate(model, glm::vec3(0.0f, -0.65f, -0.65f));
        model = glm::rotate(model, glm::radians(rodLlanta4), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.2f, 0.2f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();


        // ==========================================
        // SUSPENSIÓN Y LLANTAS TRASERAS
        // ==========================================

        // --- IZQUIERDA ATRÁS (Llanta 5 - Tecla B) ---
        model = modelaux2;
        model = glm::translate(model, glm::vec3(2.5f, -0.2f, 4.2f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelCodoAtrI = model;

        model = modelCodoAtrI;
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 0.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelCodoAtrI;
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        glm::mat4 modelExtremoAtrI = model;

        model = model;
        model = glm::scale(model, glm::vec3(0.3f, 1.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Render Llanta 5 con giro sobre su propio eje (Tecla B)
        model = modelExtremoAtrI;
        model = glm::translate(model, glm::vec3(0.0f, -0.65f, 0.65f));
        model = glm::rotate(model, glm::radians(rodLlanta5), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.2f, 0.2f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();


        // --- DERECHA ATRÁS (Llanta 6 - Tecla N) ---
        model = modelaux2;
        model = glm::translate(model, glm::vec3(2.5f, -0.2f, -4.2f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelCodoAtrD = model;

        model = modelCodoAtrD;
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f, 0.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.6f, 0.6f, 0.6f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        model = modelCodoAtrD;
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.0f));
        glm::mat4 modelExtremoAtrD = model;

        model = model;
        model = glm::scale(model, glm::vec3(0.3f, 1.3f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.8f, 0.8f, 0.8f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Render Llanta 6 con giro sobre su propio eje (Tecla N)
        model = modelExtremoAtrD;
        model = glm::translate(model, glm::vec3(0.0f, -0.65f, -0.65f));
        model = glm::rotate(model, glm::radians(rodLlanta6), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 0.6f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.2f, 0.2f, 0.2f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();


        // ==========================================
        // BRAZO ARTICULADO PRINCIPAL DEL ROVER
        // ==========================================
        model = modelaux2;
        model = glm::translate(model, glm::vec3(-3.75f, 1.25f, 3.0f));
        modelaux = model;
        color = glm::vec3(0.0f, 0.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        meshList[0]->RenderMesh();
        model = modelaux;

        // 1. Eje Base (Teclas F / G)
        model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f));
        model = glm::rotate(model, glm::radians(rotBrazoBase), glm::vec3(0.0f, 1.0f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();
        model = modelaux;

        // 2. Segmento inferior
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-0.25f, 1.5f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();

        // 3. Hombro (Teclas H / J)
        model = modelaux;
        model = glm::translate(model, glm::vec3(0.0, 1.5f, 0.0f));
        model = glm::rotate(model, glm::radians(rotHombro), glm::vec3(1.0f, 0.0f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 1.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();
        model = modelaux;

        // 4. Segmento medio
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(0.5f, 3.0f, 0.5f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();
        model = modelaux;

        // 5. Articulación superior (Teclas K / L)
        model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
        model = glm::rotate(model, glm::radians(rotAntebrazo), glm::vec3(1.0f, 0.0f, 0.0f));
        modelaux = model;
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();
        model = modelaux;

        // 6. Segmento superior corto (Punta del brazo para la garra)
        model = glm::translate(model, glm::vec3(0.0f, 0.75f, 0.0f));
        glm::mat4 modelPuntaBrazo = model;

        model = modelPuntaBrazo;
        model = glm::scale(model, glm::vec3(0.4f, 1.5f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 1.0f, 1.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[2]->RenderMeshGeometry();

        // ==========================================
        // GARRA DE 4 DEDOS
        // ==========================================

        // Dedo 1 (Lado +X)
        model = modelPuntaBrazo;
        model = glm::translate(model, glm::vec3(0.25f, 0.79f, 0.0f));
        model = glm::rotate(model, glm::radians(aperturaGarra * 45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
        model = glm::scale(model, glm::vec3(0.32f, 0.78f, 0.32f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.5f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[4]->RenderMeshGeometry();

        // Dedo 2 (Lado -X)
        model = modelPuntaBrazo;
        model = glm::translate(model, glm::vec3(-0.25f, 0.79f, 0.0f));
        model = glm::rotate(model, glm::radians(-aperturaGarra * 45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
        model = glm::scale(model, glm::vec3(0.32f, 0.78f, 0.32f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.5f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[4]->RenderMeshGeometry();

        // Dedo 3 (Lado +Z)
        model = modelPuntaBrazo;
        model = glm::translate(model, glm::vec3(0.0f, 0.79f, 0.25f));
        model = glm::rotate(model, glm::radians(-aperturaGarra * 45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
        model = glm::scale(model, glm::vec3(0.32f, 0.78f, 0.32f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.5f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[4]->RenderMeshGeometry();

        // Dedo 4 (Lado -Z)
        model = modelPuntaBrazo;
        model = glm::translate(model, glm::vec3(0.0f, 0.79f, -0.25f));
        model = glm::rotate(model, glm::radians(aperturaGarra * 45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.35f, 0.0f));
        model = glm::scale(model, glm::vec3(0.32f, 0.78f, 0.32f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.5f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[4]->RenderMeshGeometry();


        // ==========================================
        // SONDA ESPACIAL JERÁRQUICA (con somblelo chino jaja)
        // ==========================================

        rotPanelesSolares = sin(glfwGetTime() * 1.5f) * 20.0f; // Oscilación autónoma y suave de los paneles
        rotAntenaSonda += 0.6f;                                // Giro continuo del plato de la antena

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 4.0f, 12.0f)); // Posicionada al lado del rover
        glm::mat4 modelSondaBase = model;

        // 1. Cuerpo Central
        model = modelSondaBase;
        model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.75f, 0.78f, 0.82f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // 2. Tobera Inferior
        model = modelSondaBase;
        model = glm::translate(model, glm::vec3(0.0f, -1.8f, 0.0f));
        model = glm::scale(model, glm::vec3(1.2f, 0.8f, 1.2f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.3f, 0.3f, 0.3f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[3]->RenderMeshGeometry();

        // 3. Panel Solar Izquierdo
        model = modelSondaBase;
        model = glm::translate(model, glm::vec3(-1.8f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotPanelesSolares), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 0.0f));
        glm::mat4 modelPanelIzq = model;

        // Render del panel izquierdo principal
        model = modelPanelIzq;
        model = glm::scale(model, glm::vec3(3.0f, 0.2f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.0f, 0.15f, 0.5f); // Azul brillante solar
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Detalle estructural: Costilla/Viga trasera del panel izquierdo
        model = modelPanelIzq;
        model = glm::translate(model, glm::vec3(0.0f, -0.15f, 0.0f));
        model = glm::scale(model, glm::vec3(2.8f, 0.1f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.5f, 0.5f, 0.5f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // 4. Panel Solar Derecho (Simétrico y jerárquico)
        model = modelSondaBase;
        model = glm::translate(model, glm::vec3(1.8f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-rotPanelesSolares), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f));
        glm::mat4 modelPanelDer = model;

        // Render del panel derecho principal
        model = modelPanelDer;
        model = glm::scale(model, glm::vec3(3.0f, 0.2f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.0f, 0.15f, 0.5f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // Detalle estructural: Viga trasera del panel derecho
        model = modelPanelDer;
        model = glm::translate(model, glm::vec3(0.0f, -0.15f, 0.0f));
        model = glm::scale(model, glm::vec3(2.8f, 0.1f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.5f, 0.5f, 0.5f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        // 5. Mástil de Instrumentos Científicos
        model = modelSondaBase;
        model = glm::translate(model, glm::vec3(-0.8f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 1.2f, 0.3f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.4f, 0.4f, 0.4f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();
        model = modelSondaBase;
        model = glm::translate(model, glm::vec3(-0.8f, 2.7f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        sp.render();

        // 6. Antena Parabólica Principal (Montada arriba, con giro independiente autónomo)
        model = modelSondaBase;
        model = glm::translate(model, glm::vec3(0.8f, 1.8f, 0.0f)); // Base de la antena desplazada
        model = glm::rotate(model, glm::radians(rotAntenaSonda), glm::vec3(0.0f, 1.0f, 0.0f)); // Giro orbital/radar
        model = glm::scale(model, glm::vec3(1.6f, 1.0f, 1.6f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.95f, 0.95f, 0.95f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[3]->RenderMeshGeometry();

        glUseProgram(0);
        mainWindow.swapBuffers();
    }
    return 0;
}