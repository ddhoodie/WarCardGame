#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Renderer3D.h"
#include "Camera.h"

struct ChipRender {
    Mesh top{}, bottom{}, side{};
    unsigned int texFace = 0;
    unsigned int texSide = 0;

    glm::vec3 pos{ 0 }, rot{ 0 }, scale{ 1 };

    void init(int segments, float radius, float thickness,
        const char* facePngPath,
        const glm::u8vec4& sideColor);

    void draw(Renderer3D& r3d, const Camera& cam);
    void destroy();
};
