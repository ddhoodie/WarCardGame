#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "Renderer3D.h"
#include "Camera.h"

struct CardRender {
    Mesh top, bottom, side;
    unsigned int texFront = 0;
    unsigned int texBack = 0;
    unsigned int texSide = 0;

    glm::vec3 pos = { 0.0f, 0.05f, 0.0f };
    glm::vec3 rot = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

    void init(int sideSegments,
        const char* frontPng,
        const char* backPng);

    void draw(Renderer3D& r3d, const Camera& cam);

    void destroy();
};
