#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include "ModelAnimation.h"
#include <iostream>
#include <cmath>

bool HolocronAnimationController::LoadAnimation(const std::string& path)
{
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    if (!scene || !scene->HasAnimations())
    {
        std::cout << "Advertencia: No se encontraron animaciones en " << path << std::endl;
        return false;
    }

    aiAnimation* anim = scene->mAnimations[0];
    duration = (float)anim->mDuration;
    ticksPerSecond = anim->mTicksPerSecond != 0 ? (float)anim->mTicksPerSecond : 25.0f;
    std::cout << "Animación de " << path << " cargada correctamente. Duración: " << duration << " ticks." << std::endl;
    return true;
}

glm::mat4 HolocronAnimationController::GetNodeTransform(float currentTimeSec)
{
    if (!scene || !scene->HasAnimations())
        return glm::mat4(1.0f);

    aiAnimation* anim = scene->mAnimations[0];
    float timeInTicks = currentTimeSec * ticksPerSecond;
    float animTime = std::fmod(timeInTicks, duration);

    for (unsigned int i = 0; i < anim->mNumChannels; i++)
    {
        aiNodeAnim* channel = anim->mChannels[i];

        glm::vec3 pos(0.0f);
        if (channel->mNumPositionKeys > 0)
        {
            unsigned int pIdx = 0;
            for (unsigned int j = 0; j < channel->mNumPositionKeys - 1; j++)
            {
                if (animTime < (float)channel->mPositionKeys[j + 1].mTime)
                {
                    pIdx = j;
                    break;
                }
            }
            unsigned int nextPIdx = (pIdx + 1) % channel->mNumPositionKeys;
            float t1 = (float)channel->mPositionKeys[pIdx].mTime;
            float t2 = (float)channel->mPositionKeys[nextPIdx].mTime;
            float factor = (t2 - t1 > 0.0001f) ? (animTime - t1) / (t2 - t1) : 0.0f;

            aiVector3D p1 = channel->mPositionKeys[pIdx].mValue;
            aiVector3D p2 = channel->mPositionKeys[nextPIdx].mValue;
            pos = glm::mix(glm::vec3(p1.x, p1.y, p1.z), glm::vec3(p2.x, p2.y, p2.z), factor);
        }

        glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
        if (channel->mNumRotationKeys > 0)
        {
            unsigned int rIdx = 0;
            for (unsigned int j = 0; j < channel->mNumRotationKeys - 1; j++)
            {
                if (animTime < (float)channel->mRotationKeys[j + 1].mTime)
                {
                    rIdx = j;
                    break;
                }
            }
            unsigned int nextRIdx = (rIdx + 1) % channel->mNumRotationKeys;
            float t1 = (float)channel->mRotationKeys[rIdx].mTime;
            float t2 = (float)channel->mRotationKeys[nextRIdx].mTime;
            float factor = (t2 - t1 > 0.0001f) ? (animTime - t1) / (t2 - t1) : 0.0f;

            aiQuaternion q1 = channel->mRotationKeys[rIdx].mValue;
            aiQuaternion q2 = channel->mRotationKeys[nextRIdx].mValue;
            glm::quat gq1(q1.w, q1.x, q1.y, q1.z);
            glm::quat gq2(q2.w, q2.x, q2.y, q2.z);
            rot = glm::slerp(gq1, gq2, factor);
        }

        glm::mat4 matPos = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 matRot = glm::toMat4(rot);
        return matPos * matRot;
    }

    return glm::mat4(1.0f);
}