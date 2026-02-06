#include "CardRender.h"
#include "Util.h"
#include "Helper2D.h"

void CardRender::init(int sideSegments, const char* frontPng, const char* backPng) {
    top = MakeCardTopMesh();
    bottom = MakeCardBottomMesh();
    side = MakeCardSideMesh(sideSegments);

    texFront = loadImageToTexture(frontPng);
    glBindTexture(GL_TEXTURE_2D, texFront);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    texBack = loadImageToTexture(backPng);
    glBindTexture(GL_TEXTURE_2D, texBack);
    setTextureParamsClamp();
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    texSide = makeSolidTexture(0, 0, 0, 255);
}

void CardRender::draw(Renderer3D& r3d, const Camera& cam) {
    glm::mat4 M(1.0f);
    M = glm::translate(M, pos);
    M = glm::rotate(M, rot.y, glm::vec3(0, 1, 0));
    M = glm::rotate(M, rot.x, glm::vec3(1, 0, 0));
    M = glm::rotate(M, rot.z, glm::vec3(0, 0, 1));
    M = glm::scale(M, scale);

    glDisable(GL_CULL_FACE);

    glUseProgram(r3d.shader);
    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 1);
    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);
    r3d.draw(top, texFront, M, cam);

    glUseProgram(r3d.shader);
    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 1);
    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);
    r3d.draw(bottom, texBack, M, cam);

    glUseProgram(r3d.shader);
    glUniform1i(glGetUniformLocation(r3d.shader, "uIsCard"), 1);
    glUniform1i(glGetUniformLocation(r3d.shader, "uUseAlpha"), 0);
    r3d.draw(side, texSide, M, cam);
}

void CardRender::destroy() {
    top.destroy();
    bottom.destroy();
    side.destroy();
}
