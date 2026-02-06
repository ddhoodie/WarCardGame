#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"
#include <GL/glew.h>

struct Renderer3D;
struct Camera;

struct NoteBoardRender {
    Mesh boardBox{};
    Mesh paperBox{};
    GLuint texBoard = 0;
    GLuint texPaper = 0;

    glm::vec3 pos{0};
    glm::vec3 rot{0};   // radians

    // world dimenzije
    float boardW = 0.75f;
    float boardH = 1.20f;
    float boardT = 0.04f;

    float paperScaleXY = 1.0f;
    float paperScaleT  = 0.0f;

    // Ako ti rules.png i dalje bude “na bok” zbog UV-a:
    bool paperRotateUV90 = false; // probaj false prvo

    void init(const char* rulesTexPath);
    void draw(Renderer3D& r3d, const Camera& cam);
    void destroy();

private:
    bool meshesBuilt = false;
    void rebuildMeshesIfNeeded();
};