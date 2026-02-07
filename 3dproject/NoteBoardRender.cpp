#include "NoteBoardRender.h"
#include "Renderer3D.h"
#include "Camera.h"
#include "Util.h"
#include "Helper2D.h"
#include <glm/gtc/matrix_transform.hpp>

static glm::mat4 makeModel(const glm::vec3& pos, const glm::vec3& rot) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, rot.y, glm::vec3(0, 1, 0));
    M = glm::rotate(M, rot.x, glm::vec3(1, 0, 0));
    M = glm::rotate(M, rot.z, glm::vec3(0, 0, 1));
    return M;
}

void NoteBoardRender::init(const char* rulesTexPath)
{
    texBoard = makeSolidTexture(210, 170, 40, 255);

    texPaper = loadImageToTexture(rulesTexPath);
    glBindTexture(GL_TEXTURE_2D, texPaper);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void NoteBoardRender::rebuildMeshesIfNeeded()
{
    if (!boardBox.vao) {
        boardBox = MakeBoxMesh(boardW * 0.5f, boardH * 0.5f, boardT * 0.5f);
    }
    if (!paperBox.vao) {
        float pw = boardW * paperScaleXY;
        float ph = boardH * paperScaleXY;
        float pt = boardT * paperScaleT;
        paperBox = MakeBoxMesh(pw * 0.5f, ph * 0.5f, pt * 0.5f);
    }
}

void NoteBoardRender::draw(Renderer3D& r3d, const Camera& cam)
{
    rebuildMeshesIfNeeded();

    glm::mat4 M = makeModel(pos, rot);

    glUseProgram(r3d.shader);
    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 0);
    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);
    glUniform1i(glGetUniformLocation(r3d.shader, "uIsChip"), 0);

    r3d.draw(boardBox, texBoard, M, cam);

    float lift = (boardT * 0.5f) + (boardT * 0.02f); 
    glm::mat4 Mpaper = glm::translate(M, glm::vec3(0.0f, 0.0f, lift));

    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 1); 
    r3d.draw(paperBox, texPaper, Mpaper, cam);

    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);
}

void NoteBoardRender::destroy()
{
    boardBox.destroy();
    paperBox.destroy();

    if (texBoard) glDeleteTextures(1, &texBoard);
    if (texPaper) glDeleteTextures(1, &texPaper);
    texBoard = texPaper = 0;
}
