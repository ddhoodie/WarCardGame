#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Camera.h"

struct Renderer3D {
    unsigned int shader = 0;

    void init();
    void begin(const Camera& cam, int w, int h);
    void draw(const Mesh& mesh, unsigned int tex, const glm::mat4& M, const Camera& cam);
};

void setInt(GLuint prog, const char* name, int v);
