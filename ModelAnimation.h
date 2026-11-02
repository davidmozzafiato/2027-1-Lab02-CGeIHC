#ifndef MODEL_ANIMATION_H
#define MODEL_ANIMATION_H

// DEFINIR PRIMERO LA MACRO EXPERIMENTAL
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp> // Extensión experimental gtx
#include <gtx/norm.hpp>       // Extensión experimental gtx

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

inline glm::mat4 AiToGlm(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

struct HolocronAnimationController
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    float duration = 0.0f;
    float ticksPerSecond = 25.0f;

    bool LoadAnimation(const std::string& path);
    glm::mat4 GetNodeTransform(float currentTimeSec);
};

#endif