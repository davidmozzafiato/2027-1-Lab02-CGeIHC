#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#include <glew.h>
#include <glfw3.h>
// glm
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/random.hpp>
// Clases para dar orden y limpieza al código
#include "Mesh.h"
#include "Shader.h"
#include "Sphere.h"
#include "Window.h"
#include "Camera.h"

using std::vector;

const float toRadians = 3.14159265f / 180.0f;
const float PI = 3.14159265f;
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;
static double limitFPS = 1.0 / 60.0;
Camera camera;
Window mainWindow;
vector<Mesh*> meshList;
vector<Shader> shaderList;

static const char* vShader = "shaders/shader.vert";
static const char* fShader = "shaders/shader.frag";
static const char* vShaderColor = "shaders/shadercolor.vert";
Sphere sp = Sphere(1.0, 20, 20);

// 1. Cubo (Índice 0)
void CrearCubo()
{
    unsigned int cubo_indices[] = {
        0, 1, 2,  2, 3, 0, // front
        1, 5, 6,  6, 2, 1, // right
        7, 6, 5,  5, 4, 7, // back
        4, 0, 3,  3, 7, 4, // left
        4, 5, 1,  1, 0, 4, // bottom
        3, 2, 6,  6, 7, 3  // top
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

// 2. Pirámide triangular regular (Índice 1 - Usada para aletas del cohete)
void CrearPiramideTriangular()
{
    unsigned int indices_piramide_triangular[] = {
        0, 1, 2,
        1, 3, 2,
        3, 0, 2,
        1, 0, 3
    };
    GLfloat vertices_piramide_triangular[] = {
        -0.5f, -0.5f,  0.0f,
         0.5f, -0.5f,  0.0f,
         0.0f,  0.5f, -0.25f,
         0.0f, -0.5f, -0.5f
    };
    Mesh* piramidet = new Mesh();
    piramidet->CreateMesh(vertices_piramide_triangular, indices_piramide_triangular, 12, 12);
    meshList.push_back(piramidet);
}

// 3. Cilindro Sólido (Índice 2)
void CrearCilindro(int res, float R) {
    vector<GLfloat> vertices;
    vector<unsigned int> indices;
    GLfloat dt = 2 * PI / res;

    for (int n = 0; n <= res; n++) {
        GLfloat x = R * cos(n * dt);
        GLfloat z = R * sin(n * dt);
        vertices.push_back(x); vertices.push_back(-0.5f); vertices.push_back(z);
        vertices.push_back(x); vertices.push_back(0.5f); vertices.push_back(z);
    }

    for (int n = 0; n < res; n++) {
        int i0 = n * 2;
        int i1 = n * 2 + 1;
        int i2 = (n + 1) * 2;
        int i3 = (n + 1) * 2 + 1;

        indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
        indices.push_back(i1); indices.push_back(i2); indices.push_back(i3);
    }

    Mesh* cilindro = new Mesh();
    cilindro->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
    meshList.push_back(cilindro);
}

// 4. Cono Sólido y Completo (Índice 3)
void CrearCono(int res, float R) {
    vector<GLfloat> vertices;
    vector<unsigned int> indices;
    GLfloat dt = 2 * PI / res;

    vertices.push_back(0.0f);
    vertices.push_back(0.5f);
    vertices.push_back(0.0f);

    for (int n = 0; n <= res; n++) {
        GLfloat x = R * cos(n * dt);
        GLfloat z = R * sin(n * dt);
        vertices.push_back(x);
        vertices.push_back(-0.5f);
        vertices.push_back(z);
    }

    int centroBaseIdx = vertices.size() / 3;
    vertices.push_back(0.0f);
    vertices.push_back(-0.5f);
    vertices.push_back(0.0f);

    for (int i = 1; i <= res; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }

    for (int i = 1; i <= res; i++) {
        indices.push_back(centroBaseIdx);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    Mesh* cono = new Mesh();
    cono->CreateMeshGeometry(vertices, indices, vertices.size(), indices.size());
    meshList.push_back(cono);
}

// 5. Pirámide Cuadrangular (Índice 4)
void CrearPiramideCuadrangular()
{
    unsigned int piramidecuadrangular_indices[] = {
        0, 3, 4,
        3, 2, 4,
        2, 1, 4,
        1, 0, 4,
        0, 1, 2,
        0, 2, 3
    };
    GLfloat piramidecuadrangular_vertices[] = {
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.0f,  0.5f,  0.0f
    };
    Mesh* piramidec = new Mesh();
    piramidec->CreateMesh(piramidecuadrangular_vertices, piramidecuadrangular_indices, 15, 18);
    meshList.push_back(piramidec);
}

// 6. Triángulo Plano Individual (Índice 5 - Para armar las caras de colores)
void CrearTriangulo()
{
    unsigned int indices_triangulo[] = { 0, 1, 2 };
    GLfloat vertices_triangulo[] = {
         0.0f,  0.5f,  0.0f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f
    };
    Mesh* tri = new Mesh();
    tri->CreateMesh(vertices_triangulo, indices_triangulo, 9, 3);
    meshList.push_back(tri);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);

    Shader* shader2 = new Shader();
    shader2->CreateFromFiles(vShaderColor, fShader);
    shaderList.push_back(*shader2);
}

void DibujarCohete(GLuint uniformModel, GLuint uniformColor, glm::mat4 modelBase) {
    glm::mat4 model(1.0f);
    glm::mat4 modelCoheteElevado = glm::translate(modelBase, glm::vec3(0.0f, 0.4f, 0.0f));

    model = glm::translate(modelCoheteElevado, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 3.0f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.85f, 0.88f, 0.92f)));
    meshList[2]->RenderMesh();

    model = glm::translate(modelCoheteElevado, glm::vec3(0.0f, -1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 0.05f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.7f, 0.73f, 0.78f)));
    sp.render();

    model = glm::translate(modelCoheteElevado, glm::vec3(0.0f, 2.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.95f, 0.15f, 0.15f)));
    meshList[3]->RenderMesh();

    model = glm::translate(modelCoheteElevado, glm::vec3(0.0f, 0.5f, 1.02f));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.3f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.15f, 0.55f, 0.95f)));
    sp.render();

    model = glm::translate(modelCoheteElevado, glm::vec3(0.0f, -1.85f, 0.0f));
    model = glm::scale(model, glm::vec3(1.15f, 1.4f, 1.15f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.45f, 0.48f, 0.52f)));
    meshList[3]->RenderMesh();

    float angulosAletas[3] = { 0.0f, 120.0f, 240.0f };
    for (int i = 0; i < 3; i++) {
        model = modelCoheteElevado;
        model = glm::rotate(model, glm::radians(angulosAletas[i]), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.6f, 0.92f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.7f, 1.3f, 0.35f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(glm::vec3(0.95f, 0.15f, 0.15f)));
        meshList[1]->RenderMesh();
    }
}

void DibujarEstructuraGeometrica(GLuint uniformModel, GLuint uniformColor, glm::mat4 modelBase) {
    glm::mat4 model(1.0f);

    float angulos[4] = { 0.0f, 90.0f, 180.0f, 270.0f };
    glm::vec3 colores[4] = {
        glm::vec3(0.95f, 0.15f, 0.15f),
        glm::vec3(0.55f, 0.15f, 0.95f),
        glm::vec3(0.15f, 0.75f, 0.15f),
        glm::vec3(0.95f, 0.80f, 0.15f)
    };

    // Modificación: Renderizado de las 4 pirámides superiores usando la malla base
    for (int i = 0; i < 4; i++) {
        model = modelBase;
        model = glm::rotate(model, glm::radians(angulos[i]), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.4f, 0.3f));
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(colores[i]));
        meshList[4]->RenderMesh();
    }

    // Modificación: Renderizado de las 4 pirámides inferiores invertidas y simétricas
    for (int i = 0; i < 4; i++) {
        model = modelBase;
        model = glm::rotate(model, glm::radians(angulos[i]), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.3f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f, 0.8f, 0.8f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(uniformColor, 1, glm::value_ptr(colores[(i + 2) % 4]));
        meshList[4]->RenderMesh();
    }
}

int main()
{
    mainWindow = Window(800, 600);
    mainWindow.Initialise();

    CrearCubo();
    CrearPiramideTriangular();
    CrearCilindro(20, 1.0f);
    CrearCono(25, 1.0f);
    CrearPiramideCuadrangular();
    // Modificación: Adición de la malla de triángulo plano individual
    CrearTriangulo();
    CreateShaders();

    camera = Camera(glm::vec3(0.0f, 2.5f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f, 0.3f, 0.3f);

    GLuint uniformProjection = 0;
    GLuint uniformModel = 0;
    GLuint uniformView = 0;
    GLuint uniformColor = 0;
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    sp.init();
    sp.load();

    glm::mat4 model(1.0f);
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        deltaTime += (now - lastTime) / limitFPS;
        lastTime = now;

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.2f, 0.2f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        shaderList[0].useShader();
        uniformModel = shaderList[0].getModelLocation();
        uniformProjection = shaderList[0].getProjectLocation();
        uniformView = shaderList[0].getViewLocation();
        uniformColor = shaderList[0].getColorLocation();

        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -2.5f, 0.0f));
        model = glm::scale(model, glm::vec3(25.0f, 0.05f, 25.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        color = glm::vec3(0.02f, 0.02f, 0.02f);
        glUniform3fv(uniformColor, 1, glm::value_ptr(color));
        meshList[0]->RenderMesh();

        glm::mat4 modelCohete = glm::mat4(1.0f);
        modelCohete = glm::translate(modelCohete, glm::vec3(-2.0f, 0.0f, 0.0f));
        modelCohete = glm::rotate(modelCohete, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
        modelCohete = glm::rotate(modelCohete, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));
        DibujarCohete(uniformModel, uniformColor, modelCohete);

        // Modificación: Integración de la nueva función de la estructura geométrica en la escena
        glm::mat4 modelEstructura = glm::mat4(1.0f);
        modelEstructura = glm::translate(modelEstructura, glm::vec3(2.5f, 0.0f, 0.0f));
        modelEstructura = glm::rotate(modelEstructura, glm::radians(mainWindow.getrotax()), glm::vec3(1.0f, 0.0f, 0.0f));
        modelEstructura = glm::rotate(modelEstructura, glm::radians(mainWindow.getrotay()), glm::vec3(0.0f, 1.0f, 0.0f));
        DibujarEstructuraGeometrica(uniformModel, uniformColor, modelEstructura);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    return 0;
}