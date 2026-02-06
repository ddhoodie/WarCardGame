#pragma once
#include <GL/glew.h>

struct Mesh {
    GLuint vao = 0, vbo = 0;
    int vertexCount = 0;

    void destroy();
};

Mesh MakeTableMesh();  
Mesh MakeCardTopMesh();
Mesh MakeCardBottomMesh();
Mesh MakeCardSideMesh(int cornerSeg);

Mesh MakeChipTopMesh(int segments, float radius, float halfT);
Mesh MakeChipBottomMesh(int segments, float radius, float halfT);
Mesh MakeChipSideMesh(int segments, float radius, float halfT);
Mesh MakeBoxMesh(float hx, float hy, float hz); 



