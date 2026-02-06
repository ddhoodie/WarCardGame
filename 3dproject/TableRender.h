#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"

struct Renderer3D;
struct Camera;


class TableRender {
public:
    glm::vec3 pos{ 0.0f };
    glm::vec3 rot{ 0.0f };
    glm::vec3 scale{ 1.0f };

    void init(int segments,
        float mainRadius, float mainThickness,
        const char* feltTexturePath,
        float baseRadius, float baseHeight);

    void draw(Renderer3D& r3d, const Camera& cam);
    void destroy();

private:
    // gornji sto 
    Mesh top, bottom, side;
    unsigned int texFelt = 0;
    unsigned int texSide = 0;   // crvena ivica
    float mainThickness_ = 0.0f;
    float baseHeight_ = 0.0f;

    // baza 
    Mesh baseTop, baseBottom, baseSide;
    unsigned int texBase = 0;   // tamno siva
};
