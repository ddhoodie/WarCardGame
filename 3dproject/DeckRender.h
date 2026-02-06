#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"

struct Renderer3D;
struct Camera;

class DeckRender {
public:
    glm::vec3 pos{ 0.0f };
    glm::vec3 rot{ 0.0f };
    glm::vec3 scale{ 1.0f };

    void init(float thickness, const char* backTexturePath);
    void draw(Renderer3D& r3d, const Camera& cam);
    void destroy();

private:
    Mesh top, bottom, side;
    unsigned int texBack = 0;
    unsigned int texSide = 0;
};