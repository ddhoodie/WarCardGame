#pragma once
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"

struct LoadedModel {
    Mesh mesh;
    unsigned int tex = 0;
};

LoadedModel loadModelAssimp(const std::string& path,
    const std::string& diffuseTexturePath = "",
    const glm::u8vec4& fallbackColor = { 255,255,255,255 });