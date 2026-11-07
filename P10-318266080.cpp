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
#include "ModelAnimation.h"

const float toRadians = 3.14159265f / 180.0f;
Window mainWindow;
std::vector<Shader> shaderList;
Camera camera;

Model Planeta_M;
Model Vehiculo_M;
Model BizarrePlane_M;
Model Holocron_M;

Model RocketFase1_M;
Model RocketFase2_M;
Model RocketCapsula_M;

HolocronAnimationController holocronAnim;
float tiempoAnimacionHolocron = 0.0f;
glm::vec3 posBaseHolocron = glm::vec3(0.0f, 60.0f, 0.0f);

const float ALTURA_SUELO = 0.0f;

struct KeyFrameCohete {
    glm::vec3 posBase;
    float rotZ;
    glm::vec3 offsetFase1;
    glm::vec3 offsetFase2;
    glm::vec3 offsetCapsula;
    bool estaOrbitando;
};

std::vector<KeyFrameCohete> keyframesCohete;
int keyframeActualIdx = 0;
float tiempoKeyframe = 0.0f;
float duracionPorKeyframe = 1.5f;
bool animacionCoheteActiva = false;
bool animacionConcluida = false;

glm::vec3 posCohete = glm::vec3(80.0f, 10.0f, -40.0f);
float anguloInclinacionCohete = 0.0f;
glm::vec3 offsetFase1Actual = glm::vec3(0.0f);
glm::vec3 offsetFase2Actual = glm::vec3(0.0f);
glm::vec3 offsetCapsulaActual = glm::vec3(0.0f);
bool modoOrbitaCapsula = false;

int subPiezaSeleccionada = 0;
float anguloOrbitaCapsula = 0.0f;

bool teclaKPresionada = false;
bool teclaLPresionada = false;
bool teclaCPresionada = false;
bool teclaEspacioPresionada = false;
bool teclaRPresionada = false;
bool teclaTabPresionada = false;
bool teclaBPresionada = false;

Model SateliteCuerpo_M;
Model SatelitePanelIzquierdo_M;
Model SatelitePanelDerecho_M;
Model SateliteAntena_M;

Skybox* skybox = nullptr;
Material Material_brillante;
DirectionalLight mainLight;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

std::vector<glm::vec3> waypoints;

int puntoActual = 0;
float progreso = 0.0f;
float velocidad = 25.0f;
float offsetLateral = -6.5f;
float anguloYawSuave = 0.0f;

float alturaYActual = 25.0f;
float alturaObjetivo = 25.0f;
float offsetAlturaVehiculo = 2.5f;
int contadorFrames = 0;

float movVelocidad = 100.0f;

float anguloPanelSolar = 0.0f;
float anguloAntena = 0.0f;
float anguloOrbita = 0.0f;
float velocidadOrbita = 15.0f;
float radioOrbita = 600.0f;
float escalaSatelite = 8.0f;
float inclinacionOrbita = 25.0f;
float giroSatelite = 0.0f;
glm::vec3 centroPlaneta = glm::vec3(0.0f, 0.0f, 0.0f);

float rotacionMapaX = 0.0f;
float rotacionMapaY = 0.0f;

struct ParticulaHumo {
    glm::vec3 posicion;
    float vidaActual;
    float vidaMaxima;
    float tamano;
};

std::vector<ParticulaHumo> humoParticulas;
float tiempoAcrobacia = 0.0f;
float tasaEmisionHumo = 0.0f;

static const char* vShader = "shaders/shader_light.vert";
static const char* fShader = "shaders/shader_light.frag";

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

void GuardarKeyframesEnTXT(const std::string& rutaArchivo)
{
    std::ofstream archivo(rutaArchivo, std::ios::trunc);
    if (!archivo.is_open()) return;

    archivo << "# PosBase(X Y Z) RotZ Off1(X Y Z) Off2(X Y Z) OffCaps(X Y Z) Orbitando\n";
    for (size_t i = 0; i < keyframesCohete.size(); ++i)
    {
        const auto& kf = keyframesCohete[i];
        archivo << kf.posBase.x << " " << kf.posBase.y << " " << kf.posBase.z << " "
            << kf.rotZ << " "
            << kf.offsetFase1.x << " " << kf.offsetFase1.y << " " << kf.offsetFase1.z << " "
            << kf.offsetFase2.x << " " << kf.offsetFase2.y << " " << kf.offsetFase2.z << " "
            << kf.offsetCapsula.x << " " << kf.offsetCapsula.y << " " << kf.offsetCapsula.z << " "
            << (kf.estaOrbitando ? 1 : 0) << "\n";
    }
    archivo.close();
    std::cout << "[INFO] Keyframes guardados en " << rutaArchivo << std::endl;
}

void CargarKeyframesDesdeTXT(const std::string& rutaArchivo)
{
    keyframesCohete.clear();
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) return;

    std::string linea;
    while (std::getline(archivo, linea))
    {
        if (linea.empty() || linea[0] == '#' || linea[0] == '/') continue;
        std::stringstream ss(linea);
        KeyFrameCohete kf;
        int orb = 0;
        if (ss >> kf.posBase.x >> kf.posBase.y >> kf.posBase.z
            >> kf.rotZ
            >> kf.offsetFase1.x >> kf.offsetFase1.y >> kf.offsetFase1.z
            >> kf.offsetFase2.x >> kf.offsetFase2.y >> kf.offsetFase2.z
            >> kf.offsetCapsula.x >> kf.offsetCapsula.y >> kf.offsetCapsula.z
            >> orb)
        {
            kf.estaOrbitando = (orb == 1);
            keyframesCohete.push_back(kf);
        }
    }
    archivo.close();
    animacionConcluida = false;
    std::cout << "[INFO] " << keyframesCohete.size() << " Keyframes cargados." << std::endl;
}

void AgregarKeyframeActual()
{
    keyframesCohete.push_back({ posCohete, anguloInclinacionCohete, offsetFase1Actual, offsetFase2Actual, offsetCapsulaActual, modoOrbitaCapsula });
    std::cout << "[KEYFRAME #" << keyframesCohete.size() << " GUARDADO]" << std::endl;
}

void ActualizarAnimacionCohete(float dt)
{
    if (!animacionCoheteActiva || keyframesCohete.empty() || animacionConcluida) return;

    if (keyframesCohete.size() == 1)
    {
        posCohete = keyframesCohete[0].posBase;
        anguloInclinacionCohete = keyframesCohete[0].rotZ;
        offsetFase1Actual = keyframesCohete[0].offsetFase1;
        offsetFase2Actual = keyframesCohete[0].offsetFase2;
        offsetCapsulaActual = keyframesCohete[0].offsetCapsula;
        modoOrbitaCapsula = keyframesCohete[0].estaOrbitando;
        return;
    }

    tiempoKeyframe += dt;
    float t = tiempoKeyframe / duracionPorKeyframe;

    if (t >= 1.0f)
    {
        tiempoKeyframe = 0.0f;

        if (keyframeActualIdx < static_cast<int>(keyframesCohete.size()) - 2)
        {
            keyframeActualIdx++;
            t = 0.0f;
        }
        else
        {
            const KeyFrameCohete& kfFinal = keyframesCohete.back();
            posCohete = kfFinal.posBase;
            anguloInclinacionCohete = kfFinal.rotZ;
            offsetFase1Actual = kfFinal.offsetFase1;
            offsetFase2Actual = kfFinal.offsetFase2;
            offsetCapsulaActual = kfFinal.offsetCapsula;
            modoOrbitaCapsula = kfFinal.estaOrbitando;

            animacionConcluida = true;
            return;
        }
    }

    int idxSiguiente = keyframeActualIdx + 1;
    const KeyFrameCohete& kfOrigen = keyframesCohete[keyframeActualIdx];
    const KeyFrameCohete& kfDestino = keyframesCohete[idxSiguiente];

    posCohete = glm::mix(kfOrigen.posBase, kfDestino.posBase, t);
    anguloInclinacionCohete = glm::mix(kfOrigen.rotZ, kfDestino.rotZ, t);
    offsetFase1Actual = glm::mix(kfOrigen.offsetFase1, kfDestino.offsetFase1, t);
    offsetFase2Actual = glm::mix(kfOrigen.offsetFase2, kfDestino.offsetFase2, t);
    offsetCapsulaActual = glm::mix(kfOrigen.offsetCapsula, kfDestino.offsetCapsula, t);
    modoOrbitaCapsula = kfDestino.estaOrbitando;
}

std::vector<glm::vec3> CargarWaypointsDesdeTXT(const std::string& rutaArchivo)
{
    std::vector<glm::vec3> pts;
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) return pts;

    std::string linea;
    while (std::getline(archivo, linea))
    {
        if (linea.empty() || linea[0] == '/' || linea[0] == '#') continue;
        for (char& c : linea) { if (c == ',') c = ' '; }

        std::stringstream ss(linea);
        float x, y, z;
        if (ss >> x >> y >> z) pts.push_back(glm::vec3(x, y, z));
    }
    archivo.close();
    return pts;
}

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
        if (distXZ >= distMinima) resultado.push_back(pNuevo);
    }
    return resultado;
}

glm::vec3 CalcularPosicionAvion(float t)
{
    float escala = 180.0f;
    float x = escala * std::sin(t * 0.5f);
    float y = 120.0f + 40.0f * std::sin(t * 1.0f) + 30.0f * std::cos(t * 0.5f);
    float z = escala * std::sin(t * 0.5f) * std::cos(t * 0.5f);
    return glm::vec3(x, y, z);
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
    BizarrePlane_M.LoadModel("Models/BizarrePlane.glb");

    RocketFase1_M.LoadModel("Models/RocketFase1.glb");
    RocketFase2_M.LoadModel("Models/RocketFase2.glb");
    RocketCapsula_M.LoadModel("Models/RocketCapsula.glb");

    Holocron_M.LoadModel("Models/Holocron.glb");
    holocronAnim.LoadAnimation("Models/Holocron.glb");

    SateliteCuerpo_M.LoadModel("Models/SateliteCuerpo.glb");
    SatelitePanelIzquierdo_M.LoadModel("Models/SatelitePanelIzquierdo.glb");
    SatelitePanelDerecho_M.LoadModel("Models/SatelitePanelDerecho.glb");
    SateliteAntena_M.LoadModel("Models/SateliteAntena.glb");

    std::vector<glm::vec3> waypointsCrudos = CargarWaypointsDesdeTXT("vertices_pista_opengl.txt");
    waypoints = waypointsCrudos.empty() ? std::vector<glm::vec3>{ glm::vec3(10.60f, 25.0f, 60.0f) } : UnirYFiltrarPista(waypointsCrudos, 1.2f);

    if (!waypoints.empty()) { alturaYActual = waypoints[0].y; alturaObjetivo = waypoints[0].y; }

    std::vector<std::string> skyboxFaces = {
        "Textures/Skybox/space_rt.png", "Textures/Skybox/space_lf.png",
        "Textures/Skybox/space_dn.png", "Textures/Skybox/space_up.png",
        "Textures/Skybox/space_bk.png", "Textures/Skybox/space_ft.png"
    };
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

        bool* keys = mainWindow.getsKeys();

        if (keys[GLFW_KEY_TAB] && !teclaTabPresionada)
        {
            subPiezaSeleccionada = (subPiezaSeleccionada + 1) % 4;
            teclaTabPresionada = true;
        }
        else if (!keys[GLFW_KEY_TAB]) teclaTabPresionada = false;

        if (!animacionCoheteActiva)
        {
            float velTeclado = 25.0f * deltaTime;
            glm::vec3 des = glm::vec3(0.0f);
            if (keys[GLFW_KEY_UP])    des.y += velTeclado;
            if (keys[GLFW_KEY_DOWN])  des.y -= velTeclado;
            if (keys[GLFW_KEY_RIGHT]) des.x += velTeclado;
            if (keys[GLFW_KEY_LEFT])  des.x -= velTeclado;
            if (keys[GLFW_KEY_O])     des.z += velTeclado;
            if (keys[GLFW_KEY_P])     des.z -= velTeclado;

            if (subPiezaSeleccionada == 0)      posCohete += des;
            else if (subPiezaSeleccionada == 1) offsetFase1Actual += des;
            else if (subPiezaSeleccionada == 2) offsetFase2Actual += des;
            else if (subPiezaSeleccionada == 3) offsetCapsulaActual += des;

            if (keys[GLFW_KEY_U]) anguloInclinacionCohete += 45.0f * deltaTime;
            if (keys[GLFW_KEY_I]) anguloInclinacionCohete -= 45.0f * deltaTime;

            if (keys[GLFW_KEY_B] && !teclaBPresionada)
            {
                modoOrbitaCapsula = !modoOrbitaCapsula;
                teclaBPresionada = true;
            }
            else if (!keys[GLFW_KEY_B]) teclaBPresionada = false;
        }

        if (keys[GLFW_KEY_K] && !teclaKPresionada) { AgregarKeyframeActual(); teclaKPresionada = true; }
        else if (!keys[GLFW_KEY_K]) teclaKPresionada = false;

        if (keys[GLFW_KEY_L] && !teclaLPresionada) { GuardarKeyframesEnTXT("keyframescohete.txt"); teclaLPresionada = true; }
        else if (!keys[GLFW_KEY_L]) teclaLPresionada = false;

        if (keys[GLFW_KEY_C] && !teclaCPresionada) { CargarKeyframesDesdeTXT("keyframescohete.txt"); teclaCPresionada = true; }
        else if (!keys[GLFW_KEY_C]) teclaCPresionada = false;

        if (keys[GLFW_KEY_SPACE] && !teclaEspacioPresionada)
        {
            animacionCoheteActiva = !animacionCoheteActiva;
            tiempoKeyframe = 0.0f;
            keyframeActualIdx = 0;
            animacionConcluida = false;
            teclaEspacioPresionada = true;
        }
        else if (!keys[GLFW_KEY_SPACE]) teclaEspacioPresionada = false;

        if (keys[GLFW_KEY_R] && !teclaRPresionada)
        {
            keyframesCohete.clear();
            animacionCoheteActiva = false;
            animacionConcluida = false;
            teclaRPresionada = true;
        }
        else if (!keys[GLFW_KEY_R]) teclaRPresionada = false;

        ActualizarAnimacionCohete(deltaTime);

        if (modoOrbitaCapsula)
        {
            anguloOrbitaCapsula += 25.0f * deltaTime;
        }

        progreso += velocidad * deltaTime;
        if (progreso >= 1.0f)
        {
            progreso -= 1.0f;
            puntoActual = (puntoActual + 1) % waypoints.size();
        }

        int numPuntos = static_cast<int>(waypoints.size());
        int p0 = (puntoActual - 1 + numPuntos) % numPuntos;
        int p1 = puntoActual;
        int p2 = (puntoActual + 1) % numPuntos;
        int p3 = (puntoActual + 2) % numPuntos;

        glm::vec3 posInterpolada = glm::catmullRom(waypoints[p0], waypoints[p1], waypoints[p2], waypoints[p3], static_cast<float>(progreso));

        float progresoDelta = progreso + 0.03f;
        glm::vec3 posSiguiente;

        if (progresoDelta < 1.0f)
        {
            posSiguiente = glm::catmullRom(waypoints[p0], waypoints[p1], waypoints[p2], waypoints[p3], static_cast<float>(progresoDelta));
        }
        else
        {
            int p0_n = p1, p1_n = p2, p2_n = p3, p3_n = (puntoActual + 3) % numPuntos;
            float tExceso = static_cast<float>(progresoDelta - 1.0f);
            posSiguiente = glm::catmullRom(waypoints[p0_n], waypoints[p1_n], waypoints[p2_n], waypoints[p3_n], tExceso);
        }

        glm::vec3 dir = posSiguiente - posInterpolada;
        dir.y = 0.0f;
        glm::vec3 posOffset = posInterpolada;

        if (glm::length(dir) > 0.0001f)
        {
            dir = glm::normalize(dir);
            glm::vec3 derecha = glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
            posOffset += derecha * offsetLateral;

            float anguloObjetivo = glm::degrees(atan2(dir.x, dir.z));
            float diff = anguloObjetivo - anguloYawSuave;
            while (diff < -180.0f) diff += 360.0f;
            while (diff > 180.0f)  diff -= 360.0f;
            anguloYawSuave += diff * 10.0f * deltaTime;
        }

        contadorFrames++;
        if (contadorFrames >= 2)
        {
            float nuevaAlturaRaycast = 0.0f;
            if (Planeta_M.GetRayIntersectionY(posOffset.x, posOffset.z, nuevaAlturaRaycast))
            {
                alturaObjetivo = (std::abs(nuevaAlturaRaycast - posOffset.y) < 20.0f) ? nuevaAlturaRaycast : posOffset.y;
            }
            else { alturaObjetivo = posOffset.y; }
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

        tiempoAcrobacia += deltaTime * 0.8f;
        glm::vec3 posAvion = CalcularPosicionAvion(tiempoAcrobacia);
        glm::vec3 posAvionFutura = CalcularPosicionAvion(tiempoAcrobacia + 0.02f);
        glm::vec3 dirAvion = glm::normalize(posAvionFutura - posAvion);

        glm::mat4 matOrientacionAvion(1.0f);
        if (glm::length(dirAvion) > 0.001f)
        {
            glm::vec3 right = glm::normalize(glm::cross(dirAvion, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 up = glm::cross(right, dirAvion);
            matOrientacionAvion = glm::mat4(
                glm::vec4(right, 0.0f), glm::vec4(up, 0.0f),
                glm::vec4(-dirAvion, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
            );
        }
        glm::mat4 rotacionBaseAvion = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotacionBaseAvion = glm::rotate(rotacionBaseAvion, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 modelAvion = glm::translate(glm::mat4(1.0f), posAvion) * matOrientacionAvion * rotacionBaseAvion;

        tasaEmisionHumo += deltaTime;
        if (tasaEmisionHumo >= 0.02f)
        {
            tasaEmisionHumo = 0.0f;
            humoParticulas.push_back({ glm::vec3(modelAvion * glm::vec4(-1.2f, 0.0f, -2.5f, 1.0f)), 0.0f, 2.0f, 0.8f });
            humoParticulas.push_back({ glm::vec3(modelAvion * glm::vec4(1.2f, 0.0f, -2.5f, 1.0f)), 0.0f, 2.0f, 0.8f });
        }

        for (auto it = humoParticulas.begin(); it != humoParticulas.end(); )
        {
            it->vidaActual += deltaTime;
            it->tamano += deltaTime * 0.5f;
            if (it->vidaActual >= it->vidaMaxima) it = humoParticulas.erase(it);
            else ++it;
        }

        glfwPollEvents();
        camera.keyControl(mainWindow.getsKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (skybox) skybox->DrawSkybox(camera.calculateViewMatrix(), projection);

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

        glm::mat4 modelMapa(1.0f);
        modelMapa = glm::translate(modelMapa, centroPlaneta);
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelMapa));
        Planeta_M.RenderModel();

        glm::mat4 modelVehiculo(1.0f);
        modelVehiculo = glm::translate(modelVehiculo, posVehiculo);
        modelVehiculo = glm::rotate(modelVehiculo, glm::radians(anguloYawSuave), glm::vec3(0.0f, 1.0f, 0.0f));
        modelVehiculo = glm::rotate(modelVehiculo, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelVehiculo = glm::rotate(modelVehiculo, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        modelVehiculo = glm::scale(modelVehiculo, glm::vec3(2.0f, 2.0f, 2.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelVehiculo));
        Vehiculo_M.RenderModel();

        float radOrb = glm::radians(anguloOrbita);
        glm::vec3 posLocalOrbita(radioOrbita * cos(radOrb), 0.0f, radioOrbita * sin(radOrb));
        glm::mat4 matInclinacion = glm::rotate(glm::mat4(1.0f), glm::radians(inclinacionOrbita), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 posSatelite = centroPlaneta + glm::vec3(matInclinacion * glm::vec4(posLocalOrbita, 1.0f));

        glm::mat4 modelSatelitePadre(1.0f);
        modelSatelitePadre = glm::translate(modelSatelitePadre, posSatelite) * matInclinacion;
        modelSatelitePadre = glm::rotate(modelSatelitePadre, -radOrb + glm::radians(giroSatelite), glm::vec3(0.0f, 1.0f, 0.0f));
        modelSatelitePadre = glm::rotate(modelSatelitePadre, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelSatelitePadre = glm::scale(modelSatelitePadre, glm::vec3(escalaSatelite));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelSatelitePadre));
        SateliteCuerpo_M.RenderModel();

        glm::mat4 modelPanelIzq = glm::rotate(modelSatelitePadre, glm::radians(anguloPanelSolar), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPanelIzq = glm::rotate(modelPanelIzq, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelPanelIzq));
        SatelitePanelIzquierdo_M.RenderModel();

        glm::mat4 modelPanelDer = glm::rotate(modelSatelitePadre, glm::radians(-anguloPanelSolar), glm::vec3(1.0f, 0.0f, 0.0f));
        modelPanelDer = glm::rotate(modelPanelDer, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelPanelDer));
        SatelitePanelDerecho_M.RenderModel();

        glm::mat4 modelAntena = glm::rotate(modelSatelitePadre, glm::radians(anguloAntena), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelAntena));
        SateliteAntena_M.RenderModel();

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelAvion));
        BizarrePlane_M.RenderModel();

        tiempoAnimacionHolocron += deltaTime;
        glm::mat4 matAnimadaHolocron = holocronAnim.GetNodeTransform(tiempoAnimacionHolocron);
        glm::mat4 modelHolocron = glm::translate(glm::mat4(1.0f), posBaseHolocron) * matAnimadaHolocron;
        modelHolocron = glm::scale(modelHolocron, glm::vec3(4.0f));
        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelHolocron));
        Holocron_M.RenderModel();

        glm::mat4 modelCoheteBase(1.0f);
        modelCoheteBase = glm::translate(modelCoheteBase, posCohete);
        modelCoheteBase = glm::rotate(modelCoheteBase, glm::radians(anguloInclinacionCohete), glm::vec3(0.0f, 0.0f, 1.0f));
        modelCoheteBase = glm::rotate(modelCoheteBase, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelCoheteBase = glm::scale(modelCoheteBase, glm::vec3(3.0f, 3.0f, 3.0f));

        glm::vec3 posGlobalFase1 = posCohete + offsetFase1Actual;
        if (posGlobalFase1.y <= ALTURA_SUELO) posGlobalFase1.y = ALTURA_SUELO;

        glm::mat4 modelFase1(1.0f);
        modelFase1 = glm::translate(modelFase1, posGlobalFase1);
        modelFase1 = glm::rotate(modelFase1, glm::radians(anguloInclinacionCohete), glm::vec3(0.0f, 0.0f, 1.0f));
        modelFase1 = glm::rotate(modelFase1, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelFase1 = glm::scale(modelFase1, glm::vec3(3.0f, 3.0f, 3.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelFase1));
        RocketFase1_M.RenderModel();

        glm::vec3 posGlobalFase2 = posCohete + offsetFase2Actual;
        if (posGlobalFase2.y <= ALTURA_SUELO) posGlobalFase2.y = ALTURA_SUELO;

        glm::mat4 modelFase2(1.0f);
        modelFase2 = glm::translate(modelFase2, posGlobalFase2);
        modelFase2 = glm::rotate(modelFase2, glm::radians(anguloInclinacionCohete), glm::vec3(0.0f, 0.0f, 1.0f));
        modelFase2 = glm::rotate(modelFase2, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelFase2 = glm::scale(modelFase2, glm::vec3(3.0f, 3.0f, 3.0f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelFase2));
        RocketFase2_M.RenderModel();

        glm::mat4 modelCapsula(1.0f);
        if (modoOrbitaCapsula)
        {
            modelCapsula = glm::translate(modelCapsula, centroPlaneta);
            modelCapsula = glm::rotate(modelCapsula, glm::radians(anguloOrbitaCapsula), glm::vec3(0.0f, 1.0f, 0.0f));
            modelCapsula = glm::translate(modelCapsula, glm::vec3(220.0f, 90.0f, 0.0f));
            modelCapsula = glm::rotate(modelCapsula, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            modelCapsula = glm::scale(modelCapsula, glm::vec3(3.0f, 3.0f, 3.0f));
        }
        else
        {
            modelCapsula = glm::translate(modelCoheteBase, offsetCapsulaActual);
        }

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelCapsula));
        RocketCapsula_M.RenderModel();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        for (const auto& p : humoParticulas)
        {
            float alpha = 1.0f - (p.vidaActual / p.vidaMaxima);
            glm::vec3 colorHumo = glm::vec3(0.85f, 0.85f, 0.90f) * alpha;
            glUniform3fv(uniformColor, 1, glm::value_ptr(colorHumo));

            glm::mat4 modelHumo = glm::translate(glm::mat4(1.0f), p.posicion);
            modelHumo = glm::scale(modelHumo, glm::vec3(p.tamano));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(modelHumo));
            Vehiculo_M.RenderModel();
        }
        glDisable(GL_BLEND);

        glUseProgram(0);
        mainWindow.swapBuffers();
    }

    if (skybox) delete skybox;
    return 0;
}